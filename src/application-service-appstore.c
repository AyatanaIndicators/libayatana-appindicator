#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dbus/dbus-glib.h>
#include "application-service-appstore.h"
#include "application-service-marshal.h"
#include "dbus-properties-client.h"
#include "dbus-shared.h"

/* DBus Prototypes */
static gboolean _application_service_server_get_applications (CustomServiceAppstore * appstore, GArray ** apps);

#include "application-service-server.h"

#define NOTIFICATION_ITEM_PROP_ID         "Id"
#define NOTIFICATION_ITEM_PROP_CATEGORY   "Category"
#define NOTIFICATION_ITEM_PROP_STATUS     "Status"
#define NOTIFICATION_ITEM_PROP_ICON_NAME  "IconName"
#define NOTIFICATION_ITEM_PROP_AICON_NAME "AttentionIconName"
#define NOTIFICATION_ITEM_PROP_MENU       "Menu"

/* Private Stuff */
typedef struct _CustomServiceAppstorePrivate CustomServiceAppstorePrivate;
struct _CustomServiceAppstorePrivate {
	DBusGConnection * bus;
	GList * applications;
};

typedef struct _Application Application;
struct _Application {
	gchar * dbus_name;
	gchar * dbus_object;
	CustomServiceAppstore * appstore; /* not ref'd */
	DBusGProxy * dbus_proxy;
	DBusGProxy * prop_proxy;
};

#define CUSTOM_SERVICE_APPSTORE_GET_PRIVATE(o) \
			(G_TYPE_INSTANCE_GET_PRIVATE ((o), CUSTOM_SERVICE_APPSTORE_TYPE, CustomServiceAppstorePrivate))

/* Signals Stuff */
enum {
	APPLICATION_ADDED,
	APPLICATION_REMOVED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* GObject stuff */
static void custom_service_appstore_class_init (CustomServiceAppstoreClass *klass);
static void custom_service_appstore_init       (CustomServiceAppstore *self);
static void custom_service_appstore_dispose    (GObject *object);
static void custom_service_appstore_finalize   (GObject *object);

G_DEFINE_TYPE (CustomServiceAppstore, custom_service_appstore, G_TYPE_OBJECT);

static void
custom_service_appstore_class_init (CustomServiceAppstoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (CustomServiceAppstorePrivate));

	object_class->dispose = custom_service_appstore_dispose;
	object_class->finalize = custom_service_appstore_finalize;

	signals[APPLICATION_ADDED] = g_signal_new ("application-added",
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET (CustomServiceAppstore, application_added),
	                                           NULL, NULL,
	                                           _application_service_marshal_VOID__STRING_INT_STRING_STRING,
	                                           G_TYPE_NONE, 4, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_NONE);
	signals[APPLICATION_REMOVED] = g_signal_new ("application-removed",
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET (CustomServiceAppstore, application_removed),
	                                           NULL, NULL,
	                                           g_cclosure_marshal_VOID__INT,
	                                           G_TYPE_NONE, 1, G_TYPE_INT, G_TYPE_NONE);


	dbus_g_object_type_install_info(CUSTOM_SERVICE_APPSTORE_TYPE,
	                                &dbus_glib__application_service_server_object_info);

	return;
}

static void
custom_service_appstore_init (CustomServiceAppstore *self)
{
	CustomServiceAppstorePrivate * priv = CUSTOM_SERVICE_APPSTORE_GET_PRIVATE(self);

	priv->applications = NULL;
	
	GError * error = NULL;
	priv->bus = dbus_g_bus_get(DBUS_BUS_STARTER, &error);
	if (error != NULL) {
		g_error("Unable to get session bus: %s", error->message);
		g_error_free(error);
		return;
	}

	dbus_g_connection_register_g_object(priv->bus,
	                                    INDICATOR_CUSTOM_DBUS_OBJ,
	                                    G_OBJECT(self));

	return;
}

static void
custom_service_appstore_dispose (GObject *object)
{
	CustomServiceAppstorePrivate * priv = CUSTOM_SERVICE_APPSTORE_GET_PRIVATE(object);

	while (priv->applications != NULL) {
		custom_service_appstore_application_remove(CUSTOM_SERVICE_APPSTORE(object),
		                                           ((Application *)priv->applications->data)->dbus_name,
		                                           ((Application *)priv->applications->data)->dbus_object);
	}

	G_OBJECT_CLASS (custom_service_appstore_parent_class)->dispose (object);
	return;
}

static void
custom_service_appstore_finalize (GObject *object)
{

	G_OBJECT_CLASS (custom_service_appstore_parent_class)->finalize (object);
	return;
}

static void
get_all_properties_cb (DBusGProxy * proxy, GHashTable * properties, GError * error, gpointer data)
{
	if (error != NULL) {
		g_warning("Unable to get properties: %s", error->message);
		return;
	}

	Application * app = (Application *)data;

	if (g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_MENU) == NULL ||
			g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_ICON_NAME) == NULL) {
		g_warning("Notification Item on object %s of %s doesn't have enough properties.", app->dbus_object, app->dbus_name);
		g_free(app); // Need to do more than this, but it gives the idea of the flow we're going for.
		return;
	}

	CustomServiceAppstorePrivate * priv = CUSTOM_SERVICE_APPSTORE_GET_PRIVATE(app->appstore);
	priv->applications = g_list_prepend(priv->applications, app);
	
	/* TODO: We need to have the position determined better.  This
	   would involve looking at the name and category and sorting
	   it with the other entries. */

	g_signal_emit(G_OBJECT(app->appstore),
	              signals[APPLICATION_ADDED], 0, 
	              g_value_get_string(g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_ICON_NAME)),
	              0, /* Position */
	              app->dbus_name,
	              g_value_get_string(g_hash_table_lookup(properties, NOTIFICATION_ITEM_PROP_MENU)),
	              TRUE);

	return;
}

/* A simple global function for dealing with freeing the information
   in an Application structure */
static void
application_free (Application * app)
{
	if (app == NULL) return;

	if (app->dbus_name != NULL) {
		g_free(app->dbus_name);
	}
	if (app->dbus_object != NULL) {
		g_free(app->dbus_object);
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
	CustomServiceAppstore * appstore = app->appstore;
	CustomServiceAppstorePrivate * priv = CUSTOM_SERVICE_APPSTORE_GET_PRIVATE(appstore);

	GList * applistitem = g_list_find(priv->applications, app);
	if (applistitem == NULL) {
		g_warning("Removing an application that isn't in the application list?");
		return;
	}

	gint position = g_list_position(priv->applications, applistitem);

	g_signal_emit(G_OBJECT(appstore),
	              signals[APPLICATION_REMOVED], 0, 
	              position, TRUE);

	priv->applications = g_list_remove(priv->applications, app);

	application_free(app);
	return;
}

/* Adding a new NotificationItem object from DBus in to the
   appstore.  First, we need to get the information on it
   though. */
void
custom_service_appstore_application_add (CustomServiceAppstore * appstore, const gchar * dbus_name, const gchar * dbus_object)
{
	g_debug("Adding new application: %s:%s", dbus_name, dbus_object);

	/* Make sure we got a sensible request */
	g_return_if_fail(IS_CUSTOM_SERVICE_APPSTORE(appstore));
	g_return_if_fail(dbus_name != NULL && dbus_name[0] != '\0');
	g_return_if_fail(dbus_object != NULL && dbus_object[0] != '\0');
	CustomServiceAppstorePrivate * priv = CUSTOM_SERVICE_APPSTORE_GET_PRIVATE(appstore);

	/* Build the application entry.  This will be carried
	   along until we're sure we've got everything. */
	Application * app = g_new(Application, 1);

	app->dbus_name = g_strdup(dbus_name);
	app->dbus_object = g_strdup(dbus_object);
	app->appstore = appstore;

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

	/* Get all the propertiees */
	org_freedesktop_DBus_Properties_get_all_async(app->prop_proxy,
	                                              NOTIFICATION_ITEM_DBUS_IFACE,
	                                              get_all_properties_cb,
	                                              app);

	/* We're returning, nothing is yet added until the properties
	   come back and give us more info. */
	return;
}

void
custom_service_appstore_application_remove (CustomServiceAppstore * appstore, const gchar * dbus_name, const gchar * dbus_object)
{
	g_return_if_fail(IS_CUSTOM_SERVICE_APPSTORE(appstore));
	g_return_if_fail(dbus_name != NULL && dbus_name[0] != '\0');
	g_return_if_fail(dbus_object != NULL && dbus_object[0] != '\0');


	return;
}

/* DBus Interface */
static gboolean
_application_service_server_get_applications (CustomServiceAppstore * appstore, GArray ** apps)
{

	return FALSE;
}

