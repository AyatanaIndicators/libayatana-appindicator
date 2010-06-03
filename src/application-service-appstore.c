/*
An object that stores the registration of all the application
indicators.  It also communicates this to the indicator visualization.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dbus/dbus-glib.h>
#include "app-indicator.h"
#include "app-indicator-enum-types.h"
#include "application-service-appstore.h"
#include "application-service-marshal.h"
#include "dbus-properties-client.h"
#include "dbus-shared.h"

/* DBus Prototypes */
static gboolean _application_service_server_get_applications (ApplicationServiceAppstore * appstore, GPtrArray ** apps, GError ** error);

#include "application-service-server.h"

#define NOTIFICATION_ITEM_PROP_ID         "Id"
#define NOTIFICATION_ITEM_PROP_CATEGORY   "Category"
#define NOTIFICATION_ITEM_PROP_STATUS     "Status"
#define NOTIFICATION_ITEM_PROP_ICON_NAME  "IconName"
#define NOTIFICATION_ITEM_PROP_AICON_NAME "AttentionIconName"
#define NOTIFICATION_ITEM_PROP_ICON_PATH  "IconThemePath"
#define NOTIFICATION_ITEM_PROP_MENU       "Menu"

#define NOTIFICATION_ITEM_SIG_NEW_ICON    "NewIcon"
#define NOTIFICATION_ITEM_SIG_NEW_AICON   "NewAttentionIcon"
#define NOTIFICATION_ITEM_SIG_NEW_STATUS  "NewStatus"

/* Private Stuff */
typedef struct _ApplicationServiceAppstorePrivate ApplicationServiceAppstorePrivate;
struct _ApplicationServiceAppstorePrivate {
	DBusGConnection * bus;
	GList * applications;
	AppLruFile * lrufile;
};

typedef struct _Application Application;
struct _Application {
	gchar * id;
	gchar * category;
	gchar * dbus_name;
	gchar * dbus_object;
	ApplicationServiceAppstore * appstore; /* not ref'd */
	DBusGProxy * dbus_proxy;
	DBusGProxy * prop_proxy;
	gboolean validated; /* Whether we've gotten all the parameters and they look good. */
	AppIndicatorStatus status;
	gchar * icon;
	gchar * aicon;
	gchar * menu;
	gchar * icon_path;
	gboolean currently_free;
};

#define APPLICATION_SERVICE_APPSTORE_GET_PRIVATE(o) \
			(G_TYPE_INSTANCE_GET_PRIVATE ((o), APPLICATION_SERVICE_APPSTORE_TYPE, ApplicationServiceAppstorePrivate))

/* Signals Stuff */
enum {
	APPLICATION_ADDED,
	APPLICATION_REMOVED,
	APPLICATION_ICON_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* GObject stuff */
static void application_service_appstore_class_init (ApplicationServiceAppstoreClass *klass);
static void application_service_appstore_init       (ApplicationServiceAppstore *self);
static void application_service_appstore_dispose    (GObject *object);
static void application_service_appstore_finalize   (GObject *object);
static AppIndicatorStatus string_to_status(const gchar * status_string);
static void apply_status (Application * app, AppIndicatorStatus status);

G_DEFINE_TYPE (ApplicationServiceAppstore, application_service_appstore, G_TYPE_OBJECT);

static void
application_service_appstore_class_init (ApplicationServiceAppstoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (ApplicationServiceAppstorePrivate));

	object_class->dispose = application_service_appstore_dispose;
	object_class->finalize = application_service_appstore_finalize;

	signals[APPLICATION_ADDED] = g_signal_new ("application-added",
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET (ApplicationServiceAppstoreClass, application_added),
	                                           NULL, NULL,
	                                           _application_service_marshal_VOID__STRING_INT_STRING_STRING_STRING,
	                                           G_TYPE_NONE, 5, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_NONE);
	signals[APPLICATION_REMOVED] = g_signal_new ("application-removed",
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET (ApplicationServiceAppstoreClass, application_removed),
	                                           NULL, NULL,
	                                           g_cclosure_marshal_VOID__INT,
	                                           G_TYPE_NONE, 1, G_TYPE_INT, G_TYPE_NONE);
	signals[APPLICATION_ICON_CHANGED] = g_signal_new ("application-icon-changed",
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET (ApplicationServiceAppstoreClass, application_icon_changed),
	                                           NULL, NULL,
	                                           _application_service_marshal_VOID__INT_STRING,
	                                           G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_STRING, G_TYPE_NONE);

	dbus_g_object_type_install_info(APPLICATION_SERVICE_APPSTORE_TYPE,
	                                &dbus_glib__application_service_server_object_info);

	return;
}

static void
application_service_appstore_init (ApplicationServiceAppstore *self)
{
	ApplicationServiceAppstorePrivate * priv = APPLICATION_SERVICE_APPSTORE_GET_PRIVATE(self);

	priv->applications = NULL;
	priv->lrufile = NULL;
	
	GError * error = NULL;
	priv->bus = dbus_g_bus_get(DBUS_BUS_STARTER, &error);
	if (error != NULL) {
		g_error("Unable to get session bus: %s", error->message);
		g_error_free(error);
		return;
	}

	dbus_g_connection_register_g_object(priv->bus,
	                                    INDICATOR_APPLICATION_DBUS_OBJ,
	                                    G_OBJECT(self));

	return;
}

static void
application_service_appstore_dispose (GObject *object)
{
	ApplicationServiceAppstorePrivate * priv = APPLICATION_SERVICE_APPSTORE_GET_PRIVATE(object);

	while (priv->applications != NULL) {
		application_service_appstore_application_remove(APPLICATION_SERVICE_APPSTORE(object),
		                                           ((Application *)priv->applications->data)->dbus_name,
		                                           ((Application *)priv->applications->data)->dbus_object);
	}

	G_OBJECT_CLASS (application_service_appstore_parent_class)->dispose (object);
	return;
}

static void
application_service_appstore_finalize (GObject *object)
{

	G_OBJECT_CLASS (application_service_appstore_parent_class)->finalize (object);
	return;
}

/* Return from getting the properties from the item.  We're looking at those
   and making sure we have everythign that we need.  If we do, then we'll
   move on up to sending this onto the indicator. */
static void
get_all_properties_cb (DBusGProxy * proxy, GHashTable * properties, GError * error, gpointer data)
{
	if (error != NULL) {
		g_warning("Unable to get properties: %s", error->message);
		return;
	}

	Application * app = (Application *)data;

	if (g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_MENU) == NULL ||
			g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_ID) == NULL ||
			g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_CATEGORY) == NULL ||
			g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_STATUS) == NULL ||
			g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_ICON_NAME) == NULL) {
		g_warning("Notification Item on object %s of %s doesn't have enough properties.", app->dbus_object, app->dbus_name);
		g_free(app); // Need to do more than this, but it gives the idea of the flow we're going for.
		return;
	}

	app->validated = TRUE;

	app->id = g_value_dup_string(g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_ID));
	app->category = g_value_dup_string(g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_CATEGORY));
	ApplicationServiceAppstorePrivate * priv = APPLICATION_SERVICE_APPSTORE_GET_PRIVATE(app->appstore);
	app_lru_file_touch(priv->lrufile, app->id, app->category);

	app->icon = g_value_dup_string(g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_ICON_NAME));

	GValue * menuval = (GValue *)g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_MENU);
	if (G_VALUE_TYPE(menuval) == G_TYPE_STRING) {
		/* This is here to support an older version where we
		   were using strings instea of object paths. */
		app->menu = g_value_dup_string(menuval);
	} else {
		app->menu = g_strdup((gchar *)g_value_get_boxed(menuval));
	}

	if (g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_AICON_NAME) != NULL) {
		app->aicon = g_value_dup_string(g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_AICON_NAME));
	}

	gpointer icon_path_data = g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_ICON_PATH);
	if (icon_path_data != NULL) {
		app->icon_path = g_value_dup_string((GValue *)icon_path_data);
	} else {
		app->icon_path = g_strdup("");
	}

	apply_status(app, string_to_status(g_value_get_string(g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_STATUS))));

	return;
}

/* Simple translation function -- could be optimized */
static AppIndicatorStatus
string_to_status(const gchar * status_string)
{
	return (AppIndicatorStatus) g_enum_get_value_by_nick((GEnumClass *)g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_STATUS), status_string);
}

/* A small helper function to get the position of an application
   in the app list. */
static gint 
get_position (Application * app) {
	ApplicationServiceAppstore * appstore = app->appstore;
	ApplicationServiceAppstorePrivate * priv = APPLICATION_SERVICE_APPSTORE_GET_PRIVATE(appstore);

	GList * applistitem = g_list_find(priv->applications, app);
	if (applistitem == NULL) {
		return -1;
	}

	return g_list_position(priv->applications, applistitem);
}

/* A simple global function for dealing with freeing the information
   in an Application structure */
static void
application_free (Application * app)
{
	if (app == NULL) return;
	
	/* Handle the case where this could be called by unref'ing one of
	   the proxy objects. */
	if (app->currently_free) return;
	app->currently_free = TRUE;
	
	if (app->dbus_proxy) {
		g_object_unref(app->dbus_proxy);
	}
	if (app->prop_proxy) {
		g_object_unref(app->prop_proxy);
	}

	if (app->id != NULL) {
		g_free(app->id);
	}
	if (app->category != NULL) {
		g_free(app->category);
	}
	if (app->dbus_name != NULL) {
		g_free(app->dbus_name);
	}
	if (app->dbus_object != NULL) {
		g_free(app->dbus_object);
	}
	if (app->icon != NULL) {
		g_free(app->icon);
	}
	if (app->aicon != NULL) {
		g_free(app->aicon);
	}
	if (app->menu != NULL) {
		g_free(app->menu);
	}
	if (app->icon_path != NULL) {
		g_free(app->icon_path);
	}

	g_free(app);
	return;
}

/* Gets called when the proxy is destroyed, which is usually when it
   drops off of the bus. */
static void
application_removed_cb (DBusGProxy * proxy, gpointer userdata)
{
	Application * app = (Application *)userdata;

	/* Remove from the panel */
	apply_status(app, APP_INDICATOR_STATUS_PASSIVE);

	/* Destroy the data */
	application_free(app);
	return;
}

static gboolean
can_add_application (GList *applications, Application *app)
{
  if (applications)
    {
      GList *l = NULL;

      for (l = applications; l != NULL; l = g_list_next (l))
        {
          Application *tmp_app = (Application *)l->data;

          if (g_strcmp0 (tmp_app->dbus_name, app->dbus_name) == 0 &&
              g_strcmp0 (tmp_app->dbus_object, app->dbus_object) == 0)
            {
              return FALSE;
            }
        }
    }

  return TRUE;
}

/* This function takes two Application structure
   pointers and uses the lrufile to compare them. */
static gint
app_sort_func (gconstpointer a, gconstpointer b, gpointer userdata)
{
	Application * appa = (Application *)a;
	Application * appb = (Application *)b;
	AppLruFile * lrufile = (AppLruFile *)userdata;

	return app_lru_file_sort(lrufile, appa->id, appb->id);
}

/* Change the status of the application.  If we're going passive
   it removes it from the panel.  If we're coming online, then
   it add it to the panel.  Otherwise it changes the icon. */
static void
apply_status (Application * app, AppIndicatorStatus status)
{
	if (app->status == status) {
		return;
	}
	g_debug("Changing app status to: %d", status);

	ApplicationServiceAppstore * appstore = app->appstore;
	ApplicationServiceAppstorePrivate * priv = APPLICATION_SERVICE_APPSTORE_GET_PRIVATE(appstore);

	/* This means we're going off line */
	if (status == APP_INDICATOR_STATUS_PASSIVE) {
		gint position = get_position(app);
		if (position == -1) return;

		g_signal_emit(G_OBJECT(appstore),
					  signals[APPLICATION_REMOVED], 0, 
					  position, TRUE);
                priv->applications = g_list_remove(priv->applications, app);
	} else {
		/* Figure out which icon we should be using */
		gchar * newicon = app->icon;
		if (status == APP_INDICATOR_STATUS_ATTENTION && app->aicon != NULL && app->aicon[0] != '\0') {
			newicon = app->aicon;
		}

		/* Determine whether we're already shown or not */
		if (app->status == APP_INDICATOR_STATUS_PASSIVE) {
                        if (can_add_application (priv->applications, app)) {
                                /* Put on panel */
                                priv->applications = g_list_insert_sorted_with_data (priv->applications, app, app_sort_func, priv->lrufile);

                                g_signal_emit(G_OBJECT(app->appstore),
                                              signals[APPLICATION_ADDED], 0,
                                              newicon,
                                              g_list_index(priv->applications, app), /* Position */
                                              app->dbus_name,
                                              app->menu,
                                              app->icon_path,
                                              TRUE);
                        }
		} else {
			/* Icon update */
			gint position = get_position(app);
			if (position == -1) return;

			g_signal_emit(G_OBJECT(appstore),
			              signals[APPLICATION_ICON_CHANGED], 0, 
			              position, newicon, TRUE);
		}
	}

	app->status = status;

	return;
}

/* Gets the data back on an updated icon signal.  Hopefully
   a new fun icon. */
static void
new_icon_cb (DBusGProxy * proxy, GValue value, GError * error, gpointer userdata)
{
	/* Check for errors */
	if (error != NULL) {
		g_warning("Unable to get updated icon name: %s", error->message);
		return;
	}

	/* Grab the icon and make sure we have one */
	const gchar * newicon = g_value_get_string(&value);
	if (newicon == NULL) {
		g_warning("Bad new icon :(");
		return;
	}

	Application * app = (Application *) userdata;

	if (g_strcmp0(newicon, app->icon)) {
		/* If the new icon is actually a new icon */
		if (app->icon != NULL) g_free(app->icon);
		app->icon = g_strdup(newicon);

		if (app->status == APP_INDICATOR_STATUS_ACTIVE) {
			gint position = get_position(app);
			if (position == -1) return;

			g_signal_emit(G_OBJECT(app->appstore),
			              signals[APPLICATION_ICON_CHANGED], 0, 
			              position, newicon, TRUE);
		}
	}

	return;
}

/* Gets the data back on an updated aicon signal.  Hopefully
   a new fun icon. */
static void
new_aicon_cb (DBusGProxy * proxy, GValue value, GError * error, gpointer userdata)
{
	/* Check for errors */
	if (error != NULL) {
		g_warning("Unable to get updated icon name: %s", error->message);
		return;
	}

	/* Grab the icon and make sure we have one */
	const gchar * newicon = g_value_get_string(&value);
	if (newicon == NULL) {
		g_warning("Bad new icon :(");
		return;
	}

	Application * app = (Application *) userdata;

	if (g_strcmp0(newicon, app->aicon)) {
		/* If the new icon is actually a new icon */
		if (app->aicon != NULL) g_free(app->aicon);
		app->aicon = g_strdup(newicon);

		if (app->status == APP_INDICATOR_STATUS_ATTENTION) {
			gint position = get_position(app);
			if (position == -1) return;

			g_signal_emit(G_OBJECT(app->appstore),
			              signals[APPLICATION_ICON_CHANGED], 0, 
			              position, newicon, TRUE);
		}
	}

	return;
}

/* Called when the Notification Item signals that it
   has a new icon. */
static void
new_icon (DBusGProxy * proxy, gpointer data)
{
	Application * app = (Application *)data;
	if (!app->validated) return;

	org_freedesktop_DBus_Properties_get_async(app->prop_proxy,
	                                          NOTIFICATION_ITEM_DBUS_IFACE,
	                                          NOTIFICATION_ITEM_PROP_ICON_NAME,
	                                          new_icon_cb,
	                                          app);
	return;
}

/* Called when the Notification Item signals that it
   has a new attention icon. */
static void
new_aicon (DBusGProxy * proxy, gpointer data)
{
	Application * app = (Application *)data;
	if (!app->validated) return;

	org_freedesktop_DBus_Properties_get_async(app->prop_proxy,
	                                          NOTIFICATION_ITEM_DBUS_IFACE,
	                                          NOTIFICATION_ITEM_PROP_AICON_NAME,
	                                          new_aicon_cb,
	                                          app);

	return;
}

/* Called when the Notification Item signals that it
   has a new status. */
static void
new_status (DBusGProxy * proxy, const gchar * status, gpointer data)
{
	Application * app = (Application *)data;
	if (!app->validated) return;

	apply_status(app, string_to_status(status));

	return;
}

/* Adding a new NotificationItem object from DBus in to the
   appstore.  First, we need to get the information on it
   though. */
void
application_service_appstore_application_add (ApplicationServiceAppstore * appstore, const gchar * dbus_name, const gchar * dbus_object)
{
	g_debug("Adding new application: %s:%s", dbus_name, dbus_object);

	/* Make sure we got a sensible request */
	g_return_if_fail(IS_APPLICATION_SERVICE_APPSTORE(appstore));
	g_return_if_fail(dbus_name != NULL && dbus_name[0] != '\0');
	g_return_if_fail(dbus_object != NULL && dbus_object[0] != '\0');
	ApplicationServiceAppstorePrivate * priv = APPLICATION_SERVICE_APPSTORE_GET_PRIVATE(appstore);

	/* Build the application entry.  This will be carried
	   along until we're sure we've got everything. */
	Application * app = g_new0(Application, 1);

	app->validated = FALSE;
	app->dbus_name = g_strdup(dbus_name);
	app->dbus_object = g_strdup(dbus_object);
	app->appstore = appstore;
	app->status = APP_INDICATOR_STATUS_PASSIVE;
	app->icon = NULL;
	app->aicon = NULL;
	app->menu = NULL;
	app->icon_path = NULL;
	app->currently_free = FALSE;

	/* Get the DBus proxy for the NotificationItem interface */
	GError * error = NULL;
	app->dbus_proxy = dbus_g_proxy_new_for_name_owner(priv->bus,
	                                                  app->dbus_name,
	                                                  app->dbus_object,
	                                                  NOTIFICATION_ITEM_DBUS_IFACE,
	                                                  &error);

	if (error != NULL) {
		g_warning("Unable to get notification item proxy for object '%s' on host '%s': %s", dbus_object, dbus_name, error->message);
		g_error_free(error);
		g_free(app);
		return;
	}
	
	/* We've got it, let's watch it for destruction */
	g_signal_connect(G_OBJECT(app->dbus_proxy), "destroy", G_CALLBACK(application_removed_cb), app);

	/* Grab the property proxy interface */
	app->prop_proxy = dbus_g_proxy_new_for_name_owner(priv->bus,
	                                                  app->dbus_name,
	                                                  app->dbus_object,
	                                                  DBUS_INTERFACE_PROPERTIES,
	                                                  &error);

	if (error != NULL) {
		g_warning("Unable to get property proxy for object '%s' on host '%s': %s", dbus_object, dbus_name, error->message);
		g_error_free(error);
		g_object_unref(app->dbus_proxy);
		g_free(app);
		return;
	}

	/* Connect to signals */
	dbus_g_proxy_add_signal(app->dbus_proxy,
	                        NOTIFICATION_ITEM_SIG_NEW_ICON,
	                        G_TYPE_INVALID);
	dbus_g_proxy_add_signal(app->dbus_proxy,
	                        NOTIFICATION_ITEM_SIG_NEW_AICON,
	                        G_TYPE_INVALID);
	dbus_g_proxy_add_signal(app->dbus_proxy,
	                        NOTIFICATION_ITEM_SIG_NEW_STATUS,
	                        G_TYPE_STRING,
	                        G_TYPE_INVALID);

	dbus_g_proxy_connect_signal(app->dbus_proxy,
	                            NOTIFICATION_ITEM_SIG_NEW_ICON,
	                            G_CALLBACK(new_icon),
	                            app,
	                            NULL);
	dbus_g_proxy_connect_signal(app->dbus_proxy,
	                            NOTIFICATION_ITEM_SIG_NEW_AICON,
	                            G_CALLBACK(new_aicon),
	                            app,
	                            NULL);
	dbus_g_proxy_connect_signal(app->dbus_proxy,
	                            NOTIFICATION_ITEM_SIG_NEW_STATUS,
	                            G_CALLBACK(new_status),
	                            app,
	                            NULL);

	/* Get all the propertiees */
	org_freedesktop_DBus_Properties_get_all_async(app->prop_proxy,
	                                              NOTIFICATION_ITEM_DBUS_IFACE,
	                                              get_all_properties_cb,
	                                              app);

	/* We're returning, nothing is yet added until the properties
	   come back and give us more info. */
	return;
}

/* Removes an application.  Currently only works for the apps
   that are shown.  /TODO Need to fix that. */
void
application_service_appstore_application_remove (ApplicationServiceAppstore * appstore, const gchar * dbus_name, const gchar * dbus_object)
{
	g_return_if_fail(IS_APPLICATION_SERVICE_APPSTORE(appstore));
	g_return_if_fail(dbus_name != NULL && dbus_name[0] != '\0');
	g_return_if_fail(dbus_object != NULL && dbus_object[0] != '\0');

	ApplicationServiceAppstorePrivate * priv = APPLICATION_SERVICE_APPSTORE_GET_PRIVATE(appstore);
	GList * listpntr;

	for (listpntr = priv->applications; listpntr != NULL; listpntr = g_list_next(listpntr)) {
		Application * app = (Application *)listpntr->data;

		if (!g_strcmp0(app->dbus_name, dbus_name) && !g_strcmp0(app->dbus_object, dbus_object)) {
			application_removed_cb(NULL, app);
			break; /* NOTE: Must break as the list will become inconsistent */
		}
	}

	return;
}

/* Creates a basic appstore object and attaches the
   LRU file object to it. */
ApplicationServiceAppstore *
application_service_appstore_new (AppLruFile * lrufile)
{
	g_return_val_if_fail(IS_APP_LRU_FILE(lrufile), NULL);
	ApplicationServiceAppstore * appstore = APPLICATION_SERVICE_APPSTORE(g_object_new(APPLICATION_SERVICE_APPSTORE_TYPE, NULL));
	ApplicationServiceAppstorePrivate * priv = APPLICATION_SERVICE_APPSTORE_GET_PRIVATE(appstore);
	priv->lrufile = lrufile;
	return appstore;
}

/* DBus Interface */
static gboolean
_application_service_server_get_applications (ApplicationServiceAppstore * appstore, GPtrArray ** apps, GError ** error)
{
	ApplicationServiceAppstorePrivate * priv = APPLICATION_SERVICE_APPSTORE_GET_PRIVATE(appstore);

	*apps = g_ptr_array_new();
	GList * listpntr;
	gint position = 0;

	for (listpntr = priv->applications; listpntr != NULL; listpntr = g_list_next(listpntr)) {
		GValueArray * values = g_value_array_new(5);

		GValue value = {0};

		/* Icon name */
		g_value_init(&value, G_TYPE_STRING);
		g_value_set_string(&value, ((Application *)listpntr->data)->icon);
		g_value_array_append(values, &value);
		g_value_unset(&value);

		/* Position */
		g_value_init(&value, G_TYPE_INT);
		g_value_set_int(&value, position++);
		g_value_array_append(values, &value);
		g_value_unset(&value);

		/* DBus Address */
		g_value_init(&value, G_TYPE_STRING);
		g_value_set_string(&value, ((Application *)listpntr->data)->dbus_name);
		g_value_array_append(values, &value);
		g_value_unset(&value);

		/* DBus Object */
		g_value_init(&value, DBUS_TYPE_G_OBJECT_PATH);
		g_value_set_static_boxed(&value, ((Application *)listpntr->data)->menu);
		g_value_array_append(values, &value);
		g_value_unset(&value);

		/* Icon path */
		g_value_init(&value, G_TYPE_STRING);
		g_value_set_string(&value, ((Application *)listpntr->data)->icon_path);
		g_value_array_append(values, &value);
		g_value_unset(&value);

		g_ptr_array_add(*apps, values);
	}

	return TRUE;
}

