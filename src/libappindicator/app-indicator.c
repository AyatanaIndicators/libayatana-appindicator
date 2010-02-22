/*
An object to represent the application as an application indicator
in the system panel.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>
    Cody Russell <cody.russell@canonical.com>

This program is free software: you can redistribute it and/or modify it
under the terms of either or both of the following licenses:

1) the GNU Lesser General Public License version 3, as published by the
   Free Software Foundation; and/or
2) the GNU Lesser General Public License version 2.1, as published by
   the Free Software Foundation.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranties of
MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the applicable version of the GNU Lesser General Public
License for more details.

You should have received a copy of both the GNU Lesser General Public
License version 3 and version 2.1 along with this program.  If not, see
<http://www.gnu.org/licenses/>
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dbus/dbus-glib.h>
#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-gtk/client.h>

#include "libappindicator/app-indicator.h"
#include "libappindicator/app-indicator-enum-types.h"

#include "notification-item-server.h"
#include "notification-watcher-client.h"

#include "dbus-shared.h"

/**
	AppIndicatorPrivate:
	@id: The ID of the indicator.  Maps to AppIndicator::id.
	@category: Which category the indicator is.  Maps to AppIndicator::category.
	@status: The status of the indicator.  Maps to AppIndicator::status.
	@icon_name: The name of the icon to use.  Maps to AppIndicator::icon-name.
	@attention_icon_name: The name of the attention icon to use.  Maps to AppIndicator::attention-icon-name.
	@menu: The menu for this indicator.  Maps to AppIndicator::menu
	@watcher_proxy: The proxy connection to the watcher we're connected to.  If we're not connected to one this will be #NULL.

	All of the private data in an instance of a
	application indicator.
*/
struct _AppIndicatorPrivate {
	/* Properties */
	gchar                *id;
	gchar                *clean_id;
	AppIndicatorCategory  category;
	AppIndicatorStatus    status;
	gchar                *icon_name;
	gchar                *attention_icon_name;
	gchar *               icon_path;
	DbusmenuServer       *menuservice;
	GtkWidget            *menu;

	GtkStatusIcon *       status_icon;
	gint                  fallback_timer;

	/* Fun stuff */
	DBusGProxy           *watcher_proxy;
	DBusGConnection      *connection;
	DBusGProxy *          dbus_proxy;
};

/* Signals Stuff */
enum {
	NEW_ICON,
	NEW_ATTENTION_ICON,
	NEW_STATUS,
	CONNECTION_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* Enum for the properties so that they can be quickly
   found and looked up. */
enum {
	PROP_0,
	PROP_ID,
	PROP_CATEGORY,
	PROP_STATUS,
	PROP_ICON_NAME,
	PROP_ATTENTION_ICON_NAME,
	PROP_ICON_THEME_PATH,
	PROP_MENU,
	PROP_CONNECTED
};

/* The strings so that they can be slowly looked up. */
#define PROP_ID_S                    "id"
#define PROP_CATEGORY_S              "category"
#define PROP_STATUS_S                "status"
#define PROP_ICON_NAME_S             "icon-name"
#define PROP_ATTENTION_ICON_NAME_S   "attention-icon-name"
#define PROP_ICON_THEME_PATH_S       "icon-theme-path"
#define PROP_MENU_S                  "menu"
#define PROP_CONNECTED_S             "connected"

/* Private macro, shhhh! */
#define APP_INDICATOR_GET_PRIVATE(o) \
                             (G_TYPE_INSTANCE_GET_PRIVATE ((o), APP_INDICATOR_TYPE, AppIndicatorPrivate))

/* Default Path */
#define DEFAULT_ITEM_PATH   "/org/ayatana/NotificationItem"

/* More constants */
#define DEFAULT_FALLBACK_TIMER  100 /* in milliseconds */

/* Boiler plate */
static void app_indicator_class_init (AppIndicatorClass *klass);
static void app_indicator_init       (AppIndicator *self);
static void app_indicator_dispose    (GObject *object);
static void app_indicator_finalize   (GObject *object);
/* Property functions */
static void app_indicator_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void app_indicator_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);
/* Other stuff */
static void check_connect (AppIndicator * self);
static void register_service_cb (DBusGProxy * proxy, GError * error, gpointer data);
static void start_fallback_timer (AppIndicator * self, gboolean disable_timeout);
static gboolean fallback_timer_expire (gpointer data);
static GtkStatusIcon * fallback (AppIndicator * self);
static void status_icon_status_wrapper (AppIndicator * self, const gchar * status, gpointer data);
static void status_icon_changes (AppIndicator * self, gpointer data);
static void status_icon_activate (GtkStatusIcon * icon, gpointer data);
static void unfallback (AppIndicator * self, GtkStatusIcon * status_icon);
static void watcher_proxy_destroyed (GObject * object, gpointer data);
static void client_menu_changed (GtkWidget *widget, GtkWidget *child, AppIndicator *indicator);

/* GObject type */
G_DEFINE_TYPE (AppIndicator, app_indicator, G_TYPE_OBJECT);

static void
app_indicator_class_init (AppIndicatorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (AppIndicatorPrivate));

	/* Clean up */
	object_class->dispose = app_indicator_dispose;
	object_class->finalize = app_indicator_finalize;

	/* Property funcs */
	object_class->set_property = app_indicator_set_property;
	object_class->get_property = app_indicator_get_property;

	/* Our own funcs */
	klass->fallback = fallback;
	klass->unfallback = unfallback;

	/* Properties */
	g_object_class_install_property (object_class,
                                         PROP_ID,
                                         g_param_spec_string(PROP_ID_S,
                                                             "The ID for this indicator",
                                                             "An ID that should be unique, but used consistently by this program and it's indicator.",
                                                             NULL,
                                                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
                                         PROP_CATEGORY,
                                         g_param_spec_string (PROP_CATEGORY_S,
                                                              "Indicator Category",
                                                              "The type of indicator that this represents.  Please don't use 'other'.  Defaults to 'Application Status'.",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
                                         PROP_STATUS,
                                         g_param_spec_string (PROP_STATUS_S,
                                                              "Indicator Status",
                                                              "Whether the indicator is shown or requests attention.  Defaults to 'off'.",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,
                                        PROP_ICON_NAME,
	                                g_param_spec_string (PROP_ICON_NAME_S,
                                                             "An icon for the indicator",
                                                             "The default icon that is shown for the indicator.",
                                                             NULL,
                                                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class,
                                         PROP_ATTENTION_ICON_NAME,
                                         g_param_spec_string (PROP_ATTENTION_ICON_NAME_S,
                                                              "An icon to show when the indicator request attention.",
                                                              "If the indicator sets it's status to 'attention' then this icon is shown.",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,
	                                PROP_ICON_THEME_PATH,
	                                g_param_spec_string (PROP_ICON_THEME_PATH_S,
                                                             "An additional path for custom icons.",
                                                             "An additional place to look for icon names that may be installed by the application.",
                                                             NULL,
                                                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY));

        g_object_class_install_property(object_class,
                                        PROP_MENU,
                                        g_param_spec_boxed (PROP_MENU_S,
                                                             "The object path of the menu on DBus.",
                                                             "A method for getting the menu path as a string for DBus.",
                                                             DBUS_TYPE_G_OBJECT_PATH,
                                                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class,
                                         PROP_CONNECTED,
                                         g_param_spec_boolean (PROP_CONNECTED_S,
                                                               "Whether we're conneced to a watcher",
                                                               "Pretty simple, true if we have a reasonable expectation of being displayed through this object.  You should hide your TrayIcon if so.",
                                                               FALSE,
                                                               G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));


	/* Signals */

	/**
		AppIndicator::new-icon:
		@arg0: The #AppIndicator object

		Signaled when there is a new icon set for the
		object.
	*/
	signals[NEW_ICON] = g_signal_new (APP_INDICATOR_SIGNAL_NEW_ICON,
	                                  G_TYPE_FROM_CLASS(klass),
	                                  G_SIGNAL_RUN_LAST,
	                                  G_STRUCT_OFFSET (AppIndicatorClass, new_icon),
	                                  NULL, NULL,
	                                  g_cclosure_marshal_VOID__VOID,
	                                  G_TYPE_NONE, 0, G_TYPE_NONE);

	/**
		AppIndicator::new-attention-icon:
		@arg0: The #AppIndicator object

		Signaled when there is a new attention icon set for the
		object.
	*/
	signals[NEW_ATTENTION_ICON] = g_signal_new (APP_INDICATOR_SIGNAL_NEW_ATTENTION_ICON,
	                                            G_TYPE_FROM_CLASS(klass),
	                                            G_SIGNAL_RUN_LAST,
	                                            G_STRUCT_OFFSET (AppIndicatorClass, new_attention_icon),
	                                            NULL, NULL,
	                                            g_cclosure_marshal_VOID__VOID,
	                                            G_TYPE_NONE, 0, G_TYPE_NONE);

	/**
		AppIndicator::new-status:
		@arg0: The #AppIndicator object
		@arg1: The string value of the #AppIndicatorStatus enum.

		Signaled when the status of the indicator changes.
	*/
	signals[NEW_STATUS] = g_signal_new (APP_INDICATOR_SIGNAL_NEW_STATUS,
	                                    G_TYPE_FROM_CLASS(klass),
	                                    G_SIGNAL_RUN_LAST,
	                                    G_STRUCT_OFFSET (AppIndicatorClass, new_status),
	                                    NULL, NULL,
	                                    g_cclosure_marshal_VOID__STRING,
	                                    G_TYPE_NONE, 1,
                                            G_TYPE_STRING);

	/**
		AppIndicator::connection-changed:
		@arg0: The #AppIndicator object
		@arg1: Whether we're connected or not

		Signaled when we connect to a watcher, or when it drops
		away.
	*/
	signals[CONNECTION_CHANGED] = g_signal_new (APP_INDICATOR_SIGNAL_CONNECTION_CHANGED,
	                                            G_TYPE_FROM_CLASS(klass),
	                                            G_SIGNAL_RUN_LAST,
	                                            G_STRUCT_OFFSET (AppIndicatorClass, connection_changed),
	                                            NULL, NULL,
	                                            g_cclosure_marshal_VOID__BOOLEAN,
	                                            G_TYPE_NONE, 1, G_TYPE_BOOLEAN, G_TYPE_NONE);

	/* Initialize the object as a DBus type */
	dbus_g_object_type_install_info(APP_INDICATOR_TYPE,
	                                &dbus_glib__notification_item_server_object_info);

	return;
}

static void
app_indicator_init (AppIndicator *self)
{
	AppIndicatorPrivate * priv = APP_INDICATOR_GET_PRIVATE(self);

	priv->id = NULL;
	priv->clean_id = NULL;
	priv->category = APP_INDICATOR_CATEGORY_OTHER;
	priv->status = APP_INDICATOR_STATUS_PASSIVE;
	priv->icon_name = NULL;
	priv->attention_icon_name = NULL;
	priv->icon_path = NULL;
	priv->menu = NULL;
	priv->menuservice = NULL;

	priv->watcher_proxy = NULL;
	priv->connection = NULL;
	priv->dbus_proxy = NULL;

	priv->status_icon = NULL;
	priv->fallback_timer = 0;

	/* Put the object on DBus */
	GError * error = NULL;
	priv->connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_error("Unable to connect to the session bus when creating application indicator: %s", error->message);
		g_error_free(error);
		return;
	}
	dbus_g_connection_ref(priv->connection);

	self->priv = priv;

	return;
}

/* Free all objects, make sure that all the dbus
   signals are sent out before we shut this down. */
static void
app_indicator_dispose (GObject *object)
{
	AppIndicator *self = APP_INDICATOR (object);
	AppIndicatorPrivate *priv = self->priv;

	if (priv->status != APP_INDICATOR_STATUS_PASSIVE) {
		app_indicator_set_status(self, APP_INDICATOR_STATUS_PASSIVE);
	}

	if (priv->status_icon != NULL) {
		AppIndicatorClass * class = APP_INDICATOR_GET_CLASS(object);
		if (class->unfallback != NULL) {
			class->unfallback(self, priv->status_icon);
		}
		priv->status_icon = NULL;
	}

	if (priv->fallback_timer != 0) {
		g_source_remove(priv->fallback_timer);
		priv->fallback_timer = 0;
	}

	if (priv->menu != NULL) {
                g_signal_handlers_disconnect_by_func (G_OBJECT (priv->menu),
                                                      client_menu_changed,
                                                      self);
                g_object_unref(G_OBJECT(priv->menu));
		priv->menu = NULL;
	}

        if (priv->menuservice != NULL) {
                g_object_unref (priv->menuservice);
        }

	if (priv->dbus_proxy != NULL) {
		g_object_unref(G_OBJECT(priv->dbus_proxy));
		priv->dbus_proxy = NULL;
	}

	if (priv->watcher_proxy != NULL) {
		dbus_g_connection_flush(priv->connection);
		g_signal_handlers_disconnect_by_func(G_OBJECT(priv->watcher_proxy), watcher_proxy_destroyed, self);
		g_object_unref(G_OBJECT(priv->watcher_proxy));
		priv->watcher_proxy = NULL;
	}

	if (priv->connection != NULL) {
		dbus_g_connection_unref(priv->connection);
		priv->connection = NULL;
	}

	G_OBJECT_CLASS (app_indicator_parent_class)->dispose (object);
	return;
}

/* Free all of the memory that we could be using in
   the object. */
static void
app_indicator_finalize (GObject *object)
{
        AppIndicator * self = APP_INDICATOR(object);
        AppIndicatorPrivate *priv = self->priv;

	if (priv->status != APP_INDICATOR_STATUS_PASSIVE) {
		g_warning("Finalizing Application Status with the status set to: %d", priv->status);
	}

	if (priv->id != NULL) {
		g_free(priv->id);
		priv->id = NULL;
	}

	if (priv->clean_id != NULL) {
		g_free(priv->clean_id);
		priv->clean_id = NULL;
	}

	if (priv->icon_name != NULL) {
		g_free(priv->icon_name);
		priv->icon_name = NULL;
	}

	if (priv->attention_icon_name != NULL) {
		g_free(priv->attention_icon_name);
		priv->attention_icon_name = NULL;
	}

	if (priv->icon_path != NULL) {
		g_free(priv->icon_path);
		priv->icon_path = NULL;
	}

	G_OBJECT_CLASS (app_indicator_parent_class)->finalize (object);
	return;
}

#define WARN_BAD_TYPE(prop, value)  g_warning("Can not work with property '%s' with value of type '%s'.", prop, G_VALUE_TYPE_NAME(value))

/* Set some properties */
static void
app_indicator_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec)
{
        AppIndicator *self = APP_INDICATOR (object);
        AppIndicatorPrivate *priv = self->priv;
        const gchar *instr;
        GEnumValue *enum_val;

        switch (prop_id) {
        case PROP_ID:
          if (priv->id != NULL) {
            g_warning ("Resetting ID value when I already had a value of: %s", priv->id);
            break;
          }

          priv->id = g_strdup (g_value_get_string (value));

          priv->clean_id = g_strdup(priv->id);
          gchar * cleaner;
          for (cleaner = priv->clean_id; *cleaner != '\0'; cleaner++) {
            if (!g_ascii_isalnum(*cleaner)) {
              *cleaner = '_';
            }
          }

          check_connect (self);
          break;

        case PROP_CATEGORY:
          enum_val = g_enum_get_value_by_nick ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_CATEGORY),
                                               g_value_get_string (value));

          if (priv->category != enum_val->value)
            {
              priv->category = enum_val->value;
            }

          break;

        case PROP_STATUS:
          enum_val = g_enum_get_value_by_nick ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_STATUS),
                                               g_value_get_string (value));

          app_indicator_set_status (APP_INDICATOR (object),
                                    enum_val->value);
          break;

        case PROP_ICON_NAME:
          instr = g_value_get_string (value);

          if (g_strcmp0 (priv->icon_name, instr) != 0)
            {
              if (priv->icon_name)
                g_free (priv->icon_name);

              priv->icon_name = g_strdup (instr);

              g_signal_emit (self, signals[NEW_ICON], 0, TRUE);
            }

          check_connect (self);
          break;

        case PROP_ATTENTION_ICON_NAME:
          app_indicator_set_attention_icon (APP_INDICATOR (object),
                                            g_value_get_string (value));
        break;

        case PROP_ICON_THEME_PATH:
          if (priv->icon_path != NULL) {
            g_free(priv->icon_path);
          }
          priv->icon_path = g_value_dup_string(value);
          break;

        default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
          break;
        }

	return;
}

/* Function to fill our value with the property it's requesting. */
static void
app_indicator_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec)
{
        AppIndicator *self = APP_INDICATOR (object);
        AppIndicatorPrivate *priv = self->priv;
        GEnumValue *enum_value;

        switch (prop_id) {
        case PROP_ID:
          g_value_set_string (value, priv->id);
          break;

        case PROP_CATEGORY:
          enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_CATEGORY), priv->category);
          g_value_set_string (value, enum_value->value_nick);
          break;

        case PROP_STATUS:
          enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_STATUS), priv->status);
          g_value_set_string (value, enum_value->value_nick);
          break;

        case PROP_ICON_NAME:
          g_value_set_string (value, priv->icon_name);
          break;

        case PROP_ATTENTION_ICON_NAME:
          g_value_set_string (value, priv->attention_icon_name);
          break;

        case PROP_ICON_THEME_PATH:
          g_value_set_string (value, priv->icon_path);
          break;

        case PROP_MENU:
          if (priv->menuservice != NULL) {
            GValue strval = { 0 };
            g_value_init(&strval, G_TYPE_STRING);
            g_object_get_property (G_OBJECT (priv->menuservice), DBUSMENU_SERVER_PROP_DBUS_OBJECT, &strval);
            g_value_set_boxed(value, g_value_get_string(&strval));
            g_value_unset(&strval);
          }
          break;

        case PROP_CONNECTED:
          g_value_set_boolean (value, priv->watcher_proxy != NULL ? TRUE : FALSE);
          break;

        default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
          break;
        }

	return;
}

/* This function is used to see if we have enough information to
   connect to things.  If we do, and we're not connected, it
   connects for us. */
static void
check_connect (AppIndicator *self)
{
	AppIndicatorPrivate *priv = self->priv;

	/* We're alreadying connecting or trying to connect. */
	if (priv->watcher_proxy != NULL) return;

	/* Do we have enough information? */
	if (priv->menu == NULL) return;
	if (priv->icon_name == NULL) return;
	if (priv->id == NULL) return;

	gchar * path = g_strdup_printf(DEFAULT_ITEM_PATH "/%s", priv->clean_id);

	dbus_g_connection_register_g_object(priv->connection,
	                                    path,
	                                    G_OBJECT(self));

	GError * error = NULL;
	priv->watcher_proxy = dbus_g_proxy_new_for_name_owner(priv->connection,
	                                                      NOTIFICATION_WATCHER_DBUS_ADDR,
	                                                      NOTIFICATION_WATCHER_DBUS_OBJ,
	                                                      NOTIFICATION_WATCHER_DBUS_IFACE,
	                                                      &error);
	if (error != NULL) {
		/* Unable to get proxy, but we're handling that now so
		   it's not a warning anymore. */
		g_error_free(error);
		dbus_g_connection_unregister_g_object(priv->connection,
						      G_OBJECT(self));
		start_fallback_timer(self, FALSE);
		g_free(path);
		return;
	}

	g_signal_connect(G_OBJECT(priv->watcher_proxy), "destroy", G_CALLBACK(watcher_proxy_destroyed), self);
	org_freedesktop_StatusNotifierWatcher_register_status_notifier_item_async(priv->watcher_proxy, path, register_service_cb, self);
	g_free(path);

	return;
}

/* A function that gets called when the watcher dies.  Like
   dies dies.  Not our friend anymore. */
static void
watcher_proxy_destroyed (GObject * object, gpointer data)
{
	AppIndicator * self = APP_INDICATOR(data);
	g_return_if_fail(self != NULL);

	dbus_g_connection_unregister_g_object(self->priv->connection,
					      G_OBJECT(self));
	self->priv->watcher_proxy = NULL;
	start_fallback_timer(self, FALSE);
	return;
}

/* Responce from the DBus command to register a service
   with a NotificationWatcher. */
static void
register_service_cb (DBusGProxy * proxy, GError * error, gpointer data)
{
	g_return_if_fail(IS_APP_INDICATOR(data));
	AppIndicatorPrivate * priv = APP_INDICATOR(data)->priv;

	if (error != NULL) {
		/* They didn't respond, ewww.  Not sure what they could
		   be doing */
		g_warning("Unable to connect to the Notification Watcher: %s", error->message);
		dbus_g_connection_unregister_g_object(priv->connection,
						      G_OBJECT(data));
		g_object_unref(G_OBJECT(priv->watcher_proxy));
		priv->watcher_proxy = NULL;
		start_fallback_timer(APP_INDICATOR(data), TRUE);
	}

	if (priv->status_icon) {
		AppIndicatorClass * class = APP_INDICATOR_GET_CLASS(data);
		if (class->unfallback != NULL) {
			class->unfallback(APP_INDICATOR(data), priv->status_icon);
			priv->status_icon = NULL;
		} 
	}

	return;
}

/* A helper function to get the nick out of a given
   category enum value. */
static const gchar *
category_from_enum (AppIndicatorCategory category)
{
  GEnumValue *value;

  value = g_enum_get_value ((GEnumClass *)g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_CATEGORY), category);
  return value->value_nick;
}

/* Watching the dbus owner change events to see if someone
   we care about pops up! */
static void
dbus_owner_change (DBusGProxy * proxy, const gchar * name, const gchar * prev, const gchar * new, gpointer data)
{
	if (new == NULL || new[0] == '\0') {
		/* We only care about folks coming on the bus.  Exit quickly otherwise. */
		return;
	}

	if (g_strcmp0(name, NOTIFICATION_WATCHER_DBUS_ADDR)) {
		/* We only care about this address, reject all others. */
		return;
	}

	/* Woot, there's a new notification watcher in town. */

	AppIndicatorPrivate * priv = APP_INDICATOR_GET_PRIVATE(data);

	if (priv->fallback_timer != 0) {
		/* Stop a timer */
		g_source_remove(priv->fallback_timer);

		/* Stop listening to bus events */
		g_object_unref(G_OBJECT(priv->dbus_proxy));
		priv->dbus_proxy = NULL;
	}

	/* Let's start from the very beginning */
	check_connect(APP_INDICATOR(data));

	return;
}

/* This is an idle function to create the proxy.  This is mostly
   because start_fallback_timer can get called in the distruction
   of a proxy and thus the proxy manager gets confused when creating
   a new proxy as part of destroying an old one.  This function being
   on idle means that we'll just do it outside of the same stack where
   the previous proxy is being destroyed. */
static gboolean
setup_name_owner_proxy (gpointer data)
{
	g_return_val_if_fail(IS_APP_INDICATOR(data), FALSE);
	AppIndicatorPrivate * priv = APP_INDICATOR(data)->priv;

	if (priv->dbus_proxy == NULL) {
		priv->dbus_proxy = dbus_g_proxy_new_for_name(priv->connection,
		                                             DBUS_SERVICE_DBUS,
		                                             DBUS_PATH_DBUS,
		                                             DBUS_INTERFACE_DBUS);
		dbus_g_proxy_add_signal(priv->dbus_proxy, "NameOwnerChanged",
		                        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		                        G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(priv->dbus_proxy, "NameOwnerChanged",
		                            G_CALLBACK(dbus_owner_change), data, NULL);
	}

	return FALSE;
}

/* A function that will start the fallback timer if it's not
   already started.  It sets up the DBus watcher to see if
   there is a change.  Also, provides an override mode for cases
   where it's unlikely that a timer will help anything. */
static void
start_fallback_timer (AppIndicator * self, gboolean disable_timeout)
{
	g_return_if_fail(IS_APP_INDICATOR(self));
	AppIndicatorPrivate * priv = APP_INDICATOR(self)->priv;

	if (priv->fallback_timer != 0) {
		/* The timer is set, let's just be happy with the one
		   we've already got running */
		return;
	}

	if (priv->dbus_proxy == NULL) {
		/* NOTE: Read the comment on setup_name_owner_proxy */
		g_idle_add(setup_name_owner_proxy, self);
	}

	if (disable_timeout) {
		fallback_timer_expire(self);
	} else {
		priv->fallback_timer = g_timeout_add(DEFAULT_FALLBACK_TIMER, fallback_timer_expire, self);
	}

	return;
}

/* A function that gets executed when we want to change the
   state of the fallback. */
static gboolean
fallback_timer_expire (gpointer data)
{
	g_return_val_if_fail(IS_APP_INDICATOR(data), FALSE);

	AppIndicatorPrivate * priv = APP_INDICATOR(data)->priv;
	AppIndicatorClass * class = APP_INDICATOR_GET_CLASS(data);

	if (priv->status_icon == NULL) {
		if (class->fallback != NULL) {
			priv->status_icon = class->fallback(APP_INDICATOR(data));
		} 
	} else {
		if (class->unfallback != NULL) {
			class->unfallback(APP_INDICATOR(data), priv->status_icon);
			priv->status_icon = NULL;
		} else {
			g_warning("No 'unfallback' function but the 'fallback' function returned a non-NULL result.");
		}
	}

	priv->fallback_timer = 0;
	return FALSE;
}

/* Creates a StatusIcon that can be used when the application
   indicator area isn't available. */
static GtkStatusIcon *
fallback (AppIndicator * self)
{
	GtkStatusIcon * icon = gtk_status_icon_new();

	gtk_status_icon_set_title(icon, app_indicator_get_id(self));
	
	g_signal_connect(G_OBJECT(self), APP_INDICATOR_SIGNAL_NEW_STATUS,
		G_CALLBACK(status_icon_status_wrapper), icon);
	g_signal_connect(G_OBJECT(self), APP_INDICATOR_SIGNAL_NEW_ICON,
		G_CALLBACK(status_icon_changes), icon);
	g_signal_connect(G_OBJECT(self), APP_INDICATOR_SIGNAL_NEW_ATTENTION_ICON,
		G_CALLBACK(status_icon_changes), icon);

	status_icon_changes(self, icon);

	g_signal_connect(G_OBJECT(icon), "activate", G_CALLBACK(status_icon_activate), self);

	return icon;
}

/* A wrapper as the status update prototype is a little
   bit different, but we want to handle it the same. */
static void
status_icon_status_wrapper (AppIndicator * self, const gchar * status, gpointer data)
{
	return status_icon_changes(self, data);
}

/* This tracks changes to either the status or the icons
   that are associated with the app indicator */
static void
status_icon_changes (AppIndicator * self, gpointer data)
{
	GtkStatusIcon * icon = GTK_STATUS_ICON(data);

	switch (app_indicator_get_status(self)) {
	case APP_INDICATOR_STATUS_PASSIVE:
		gtk_status_icon_set_visible(icon, FALSE);
		gtk_status_icon_set_from_icon_name(icon, app_indicator_get_icon(self));
		break;
	case APP_INDICATOR_STATUS_ACTIVE:
		gtk_status_icon_set_from_icon_name(icon, app_indicator_get_icon(self));
		gtk_status_icon_set_visible(icon, TRUE);
		break;
	case APP_INDICATOR_STATUS_ATTENTION:
		gtk_status_icon_set_from_icon_name(icon, app_indicator_get_attention_icon(self));
		gtk_status_icon_set_visible(icon, TRUE);
		break;
	};

	return;
}

/* Handles the activate action by the status icon by showing
   the menu in a popup. */
static void
status_icon_activate (GtkStatusIcon * icon, gpointer data)
{
	GtkMenu * menu = app_indicator_get_menu(APP_INDICATOR(data));
	if (menu == NULL)
		return;
	
	gtk_menu_popup(menu,
	               NULL, /* Parent Menu */
	               NULL, /* Parent item */
	               gtk_status_icon_position_menu,
	               icon,
	               1, /* Button */
	               gtk_get_current_event_time());

	return;
}

/* Removes the status icon as the application indicator area
   is now up and running again. */
static void
unfallback (AppIndicator * self, GtkStatusIcon * status_icon)
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(self), status_icon_status_wrapper, status_icon);
	g_signal_handlers_disconnect_by_func(G_OBJECT(self), status_icon_changes, status_icon);
	g_object_unref(G_OBJECT(status_icon));
	return;
}


/* ************************* */
/*    Public Functions       */
/* ************************* */

/**
        app_indicator_new:
        @id: The unique id of the indicator to create.
        @icon_name: The icon name for this indicator
        @category: The category of indicator.

		Creates a new #AppIndicator setting the properties:
		#AppIndicator::id with @id, #AppIndicator::category
		with @category and #AppIndicator::icon-name with
		@icon_name.

        Return value: A pointer to a new #AppIndicator object.
 */
AppIndicator *
app_indicator_new (const gchar          *id,
                   const gchar          *icon_name,
                   AppIndicatorCategory  category)
{
  AppIndicator *indicator = g_object_new (APP_INDICATOR_TYPE,
                                          PROP_ID_S, id,
                                          PROP_CATEGORY_S, category_from_enum (category),
                                          PROP_ICON_NAME_S, icon_name,
                                          NULL);

  return indicator;
}

/**
        app_indicator_new_with_path:
        @id: The unique id of the indicator to create.
        @icon_name: The icon name for this indicator
        @category: The category of indicator.
        @icon_path: A custom path for finding icons.

		Creates a new #AppIndicator setting the properties:
		#AppIndicator::id with @id, #AppIndicator::category
		with @category, #AppIndicator::icon-name with
		@icon_name and #AppIndicator::icon-theme-path with @icon_path.

        Return value: A pointer to a new #AppIndicator object.
 */
AppIndicator *
app_indicator_new_with_path (const gchar          *id,
                             const gchar          *icon_name,
                             AppIndicatorCategory  category,
                             const gchar          *icon_path)
{
	AppIndicator *indicator = g_object_new (APP_INDICATOR_TYPE,
	                                        PROP_ID_S, id,
	                                        PROP_CATEGORY_S, category_from_enum (category),
	                                        PROP_ICON_NAME_S, icon_name,
	                                        PROP_ICON_THEME_PATH_S, icon_path,
	                                        NULL);

	return indicator;
}

/**
	app_indicator_get_type:

	Generates or returns the unique #GType for #AppIndicator.

	Return value: A unique #GType for #AppIndicator objects.
*/

/**
	app_indicator_set_status:
	@self: The #AppIndicator object to use
	@status: The status to set for this indicator

	Wrapper function for property #AppIndicator::status.
*/
void
app_indicator_set_status (AppIndicator *self, AppIndicatorStatus status)
{
  g_return_if_fail (IS_APP_INDICATOR (self));

  if (self->priv->status != status)
    {
      GEnumValue *value = g_enum_get_value ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_STATUS), status);

      self->priv->status = status;
      g_signal_emit (self, signals[NEW_STATUS], 0, value->value_nick);
    }
}

/**
	app_indicator_set_attention_icon:
	@self: The #AppIndicator object to use
	@icon_name: The name of the attention icon to set for this indicator

	Wrapper function for property #AppIndicator::attention-icon.
*/
void
app_indicator_set_attention_icon (AppIndicator *self, const gchar *icon_name)
{
  g_return_if_fail (IS_APP_INDICATOR (self));
  g_return_if_fail (icon_name != NULL);

  if (g_strcmp0 (self->priv->attention_icon_name, icon_name) != 0)
    {
      if (self->priv->attention_icon_name)
        g_free (self->priv->attention_icon_name);

      self->priv->attention_icon_name = g_strdup (icon_name);

      g_signal_emit (self, signals[NEW_ATTENTION_ICON], 0, TRUE);
    }
}

/**
        app_indicator_set_icon:
        @self: The #AppIndicator object to use
        @icon_name: The icon name to set.

		Sets the default icon to use when the status is active but
		not set to attention.  In most cases, this should be the
		application icon for the program.
**/
void
app_indicator_set_icon (AppIndicator *self, const gchar *icon_name)
{
  g_return_if_fail (IS_APP_INDICATOR (self));
  g_return_if_fail (icon_name != NULL);

  if (g_strcmp0 (self->priv->icon_name, icon_name) != 0)
    {
      if (self->priv->icon_name)
        g_free (self->priv->icon_name);

      self->priv->icon_name = g_strdup (icon_name);

      g_signal_emit (self, signals[NEW_ICON], 0, TRUE);
    }
}

static void
activate_menuitem (DbusmenuMenuitem *mi, guint timestamp, gpointer user_data)
{
  GtkWidget *widget = (GtkWidget *)user_data;

  gtk_menu_item_activate (GTK_MENU_ITEM (widget));
}

static void
widget_toggled (GtkWidget *widget, DbusmenuMenuitem *mi)
{
  dbusmenu_menuitem_property_set_int (mi,
                                      DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                      gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget)) ? DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED : DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);
}

static void
menuitem_iterate (GtkWidget *widget,
                  gpointer   data)
{
  if (GTK_IS_LABEL (widget))
    {
      DbusmenuMenuitem *child = (DbusmenuMenuitem *)data;

      dbusmenu_menuitem_property_set (child,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      gtk_label_get_text (GTK_LABEL (widget)));
    }
}

static void
update_icon_name (DbusmenuMenuitem *menuitem,
                  GtkImage         *image)
{
  if (gtk_image_get_storage_type (image) != GTK_IMAGE_ICON_NAME)
    return;

  dbusmenu_menuitem_property_set (menuitem,
                                  DBUSMENU_MENUITEM_PROP_ICON_NAME,
                                  image->data.name.icon_name);
}

/* return value specifies whether the label is set or not */
static gboolean
update_stock_item (DbusmenuMenuitem *menuitem,
                   GtkImage         *image)
{
  GtkStockItem stock;

  if (gtk_image_get_storage_type (image) != GTK_IMAGE_STOCK)
    return FALSE;

  gtk_stock_lookup (image->data.stock.stock_id, &stock);

  dbusmenu_menuitem_property_set (menuitem,
                                  DBUSMENU_MENUITEM_PROP_ICON_NAME,
                                  image->data.stock.stock_id);

  const gchar * label = dbusmenu_menuitem_property_get (menuitem,
                                  DBUSMENU_MENUITEM_PROP_LABEL);
  
  if (stock.label != NULL && label != NULL)
    {
      dbusmenu_menuitem_property_set (menuitem,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      stock.label);

      return TRUE;
    }

  return FALSE;
}

static void
image_notify_cb (GtkWidget  *widget,
                 GParamSpec *pspec,
                 gpointer    data)
{
  DbusmenuMenuitem *child = (DbusmenuMenuitem *)data;
  GtkImage *image = GTK_IMAGE (widget);

  if (pspec->name == g_intern_static_string ("stock"))
    {
      update_stock_item (child, image);
    }
  else if (pspec->name == g_intern_static_string ("icon-name"))
    {
      update_icon_name (child, image);
    }
}

static void
widget_notify_cb (GtkWidget  *widget,
                  GParamSpec *pspec,
                  gpointer    data)
{
  DbusmenuMenuitem *child = (DbusmenuMenuitem *)data;

  if (pspec->name == g_intern_static_string ("sensitive"))
    {
      dbusmenu_menuitem_property_set_bool (child,
                                           DBUSMENU_MENUITEM_PROP_ENABLED,
                                           GTK_WIDGET_IS_SENSITIVE (widget));
    }
  else if (pspec->name == g_intern_static_string ("label"))
    {
      dbusmenu_menuitem_property_set (child,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      gtk_menu_item_get_label (GTK_MENU_ITEM (widget)));
    }
  else if (pspec->name == g_intern_static_string ("visible"))
    {
      dbusmenu_menuitem_property_set_bool (child,
                                           DBUSMENU_MENUITEM_PROP_VISIBLE,
                                           gtk_widget_get_visible (widget));
    }
}

static void
container_iterate (GtkWidget *widget,
                   gpointer   data)
{
  DbusmenuMenuitem *root = (DbusmenuMenuitem *)data;
  DbusmenuMenuitem *child;
  GtkWidget *submenu = NULL;
  const gchar *label = NULL;
  gboolean label_set = FALSE;

  child = dbusmenu_menuitem_new ();

  if (GTK_IS_SEPARATOR_MENU_ITEM (widget))
    {
      dbusmenu_menuitem_property_set (child,
                                      "type",
                                      DBUSMENU_CLIENT_TYPES_SEPARATOR);
    }
  else
    {
      if (GTK_IS_CHECK_MENU_ITEM (widget))
        {
          GtkCheckMenuItem *check;

          check = GTK_CHECK_MENU_ITEM (widget);
          label = gtk_menu_item_get_label (GTK_MENU_ITEM (widget));

          dbusmenu_menuitem_property_set (child,
                                          DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE,
                                          GTK_IS_RADIO_MENU_ITEM (widget) ? DBUSMENU_MENUITEM_TOGGLE_RADIO : DBUSMENU_MENUITEM_TOGGLE_CHECK);

          dbusmenu_menuitem_property_set (child,
                                          DBUSMENU_MENUITEM_PROP_LABEL,
                                          label);

          label_set = TRUE;

          dbusmenu_menuitem_property_set_int (child,
                                              DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                              gtk_check_menu_item_get_active (check) ? DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED : DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);

          g_signal_connect (widget,
                            "toggled",
                            G_CALLBACK (widget_toggled),
                            child);
        }
      else if (GTK_IS_IMAGE_MENU_ITEM (widget))
        {
          GtkWidget *image;
          GtkImageType image_type;

          image = gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM (widget));
          image_type = gtk_image_get_storage_type (GTK_IMAGE (image));

          g_signal_connect (image,
                            "notify",
                            G_CALLBACK (image_notify_cb),
                            child);

          if (image_type == GTK_IMAGE_STOCK)
            {
              label_set = update_stock_item (child, GTK_IMAGE (image));
            }
          else if (image_type == GTK_IMAGE_ICON_NAME)
            {
              update_icon_name (child, GTK_IMAGE (image));
            }
        }
    }

  if (!label_set)
    {
      if (label != NULL)
        {
          dbusmenu_menuitem_property_set (child,
                                          DBUSMENU_MENUITEM_PROP_LABEL,
                                          label);
        }
      else
        {
          /* find label child widget */
          gtk_container_forall (GTK_CONTAINER (widget),
                                menuitem_iterate,
                                child);
        }
    }

  if (GTK_IS_MENU_ITEM (widget))
    {
      submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (widget));
      if (submenu != NULL)
        {
          gtk_container_forall (GTK_CONTAINER (submenu),
                                container_iterate,
                                child);
        }
    }

  dbusmenu_menuitem_property_set_bool (child,
                                       DBUSMENU_MENUITEM_PROP_ENABLED,
                                       GTK_WIDGET_IS_SENSITIVE (widget));

  g_signal_connect (widget, "notify",
                    G_CALLBACK (widget_notify_cb), child);

  g_signal_connect (G_OBJECT (child),
                    DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                    G_CALLBACK (activate_menuitem), widget);
  dbusmenu_menuitem_child_append (root, child);
}

static void
setup_dbusmenu (AppIndicator *self)
{
  AppIndicatorPrivate *priv;
  DbusmenuMenuitem *root;

  priv = self->priv;
  root = dbusmenu_menuitem_new ();

  gtk_container_forall (GTK_CONTAINER (priv->menu),
                        container_iterate,
                        root);

  if (priv->menuservice == NULL)
    {
      gchar * path = g_strdup_printf(DEFAULT_ITEM_PATH "/%s/Menu", priv->clean_id);
      priv->menuservice = dbusmenu_server_new (path);
      g_free(path);
    }

  dbusmenu_server_set_root (priv->menuservice, root);

  return;
}

static void
client_menu_changed (GtkWidget    *widget,
                     GtkWidget    *child,
                     AppIndicator *indicator)
{
  setup_dbusmenu (indicator);
}

/**
        app_indicator_set_menu:
        @self: The #AppIndicator
        @menu: A #GtkMenu to set

        Sets the menu that should be shown when the Application Indicator
        is clicked on in the panel.  An application indicator will not
        be rendered unless it has a menu.
**/
void
app_indicator_set_menu (AppIndicator *self, GtkMenu *menu)
{
  AppIndicatorPrivate *priv;

  g_return_if_fail (IS_APP_INDICATOR (self));
  g_return_if_fail (GTK_IS_MENU (menu));
  g_return_if_fail (self->priv->clean_id != NULL);

  priv = self->priv;

  if (priv->menu != NULL)
    {
      g_object_unref (priv->menu);
    }

  priv->menu = GTK_WIDGET (menu);
  g_object_ref (priv->menu);

  setup_dbusmenu (self);

  check_connect (self);

  g_signal_connect (menu,
                    "add",
                    G_CALLBACK (client_menu_changed),
                    self);
  g_signal_connect (menu,
                    "remove",
                    G_CALLBACK (client_menu_changed),
                    self);
}

/**
	app_indicator_get_id:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator::id.

	Return value: The current ID
*/
const gchar *
app_indicator_get_id (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->id;
}

/**
	app_indicator_get_category:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator::category.

	Return value: The current category.
*/
AppIndicatorCategory
app_indicator_get_category (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

  return self->priv->category;
}

/**
	app_indicator_get_status:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator::status.

	Return value: The current status.
*/
AppIndicatorStatus
app_indicator_get_status (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), APP_INDICATOR_STATUS_PASSIVE);

  return self->priv->status;
}

/**
	app_indicator_get_icon:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator::icon-name.

	Return value: The current icon name.
*/
const gchar *
app_indicator_get_icon (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->icon_name;
}

/**
	app_indicator_get_attention_icon:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator::attention-icon-name.

	Return value: The current attention icon name.
*/
const gchar *
app_indicator_get_attention_icon (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->attention_icon_name;
}

/**
	app_indicator_get_menu:
	@self: The #AppIndicator object to use

	Gets the menu being used for this application indicator.

	Return value: A menu object or #NULL if one hasn't been set.
*/
GtkMenu *
app_indicator_get_menu (AppIndicator *self)
{
	AppIndicatorPrivate *priv;

	g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

	priv = self->priv;

	return GTK_MENU(priv->menu);
}
