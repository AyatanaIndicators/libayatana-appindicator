
/* G Stuff */
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

/* DBus Stuff */
#include <dbus/dbus-glib.h>
#include <libdbusmenu-gtk/menu.h>

/* Indicator Stuff */
#include <libindicator/indicator.h>
#include <libindicator/indicator-object.h>
#include <libindicator/indicator-service-manager.h>

/* Local Stuff */
#include "dbus-shared.h"
#include "custom-service-client.h"
#include "custom-service-marshal.h"

#define INDICATOR_CUSTOM_TYPE            (indicator_custom_get_type ())
#define INDICATOR_CUSTOM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), INDICATOR_CUSTOM_TYPE, IndicatorCustom))
#define INDICATOR_CUSTOM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), INDICATOR_CUSTOM_TYPE, IndicatorCustomClass))
#define IS_INDICATOR_CUSTOM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), INDICATOR_CUSTOM_TYPE))
#define IS_INDICATOR_CUSTOM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), INDICATOR_CUSTOM_TYPE))
#define INDICATOR_CUSTOM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), INDICATOR_CUSTOM_TYPE, IndicatorCustomClass))

typedef struct _IndicatorCustom      IndicatorCustom;
typedef struct _IndicatorCustomClass IndicatorCustomClass;

struct _IndicatorCustomClass {
	IndicatorObjectClass parent_class;
};

struct _IndicatorCustom {
	IndicatorObject parent;
};

GType indicator_custom_get_type (void);

INDICATOR_SET_VERSION
INDICATOR_SET_TYPE(INDICATOR_CUSTOM_TYPE)


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

typedef struct _IndicatorCustomPrivate IndicatorCustomPrivate;
struct _IndicatorCustomPrivate {
	IndicatorServiceManager * sm;
	DBusGConnection * bus;
	DBusGProxy * service_proxy;
	GList * applications;
};

typedef struct _ApplicationEntry ApplicationEntry;
struct _ApplicationEntry {
	IndicatorObjectEntry entry;
};

#define INDICATOR_CUSTOM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), INDICATOR_CUSTOM_TYPE, IndicatorCustomPrivate))

static void indicator_custom_class_init (IndicatorCustomClass *klass);
static void indicator_custom_init       (IndicatorCustom *self);
static void indicator_custom_dispose    (GObject *object);
static void indicator_custom_finalize   (GObject *object);
static GList * get_entries (IndicatorObject * io);
static void connected (IndicatorServiceManager * sm, gboolean connected, IndicatorCustom * custom);
static void application_added (DBusGProxy * proxy, const gchar * iconname, gint position, const gchar * dbusaddress, const gchar * dbusobject, IndicatorCustom * custom);
static void application_removed (DBusGProxy * proxy, gint position , IndicatorCustom * custom);
static void get_applications (DBusGProxy *proxy, GPtrArray *OUT_applications, GError *error, gpointer userdata);

G_DEFINE_TYPE (IndicatorCustom, indicator_custom, INDICATOR_OBJECT_TYPE);

static void
indicator_custom_class_init (IndicatorCustomClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (IndicatorCustomPrivate));

	object_class->dispose = indicator_custom_dispose;
	object_class->finalize = indicator_custom_finalize;

	IndicatorObjectClass * io_class = INDICATOR_OBJECT_CLASS(klass);

	io_class->get_entries = get_entries;

	dbus_g_object_register_marshaller(_custom_service_marshal_VOID__STRING_INT_STRING_STRING,
	                                  G_TYPE_NONE,
	                                  G_TYPE_STRING,
	                                  G_TYPE_INT,
	                                  G_TYPE_STRING,
	                                  G_TYPE_STRING,
	                                  G_TYPE_INVALID);

	return;
}

static void
indicator_custom_init (IndicatorCustom *self)
{
	IndicatorCustomPrivate * priv = INDICATOR_CUSTOM_GET_PRIVATE(self);

	/* These are built in the connection phase */
	priv->bus = NULL;
	priv->service_proxy = NULL;

	priv->sm = indicator_service_manager_new(INDICATOR_CUSTOM_DBUS_ADDR);	
	g_signal_connect(G_OBJECT(priv->sm), INDICATOR_SERVICE_MANAGER_SIGNAL_CONNECTION_CHANGE, G_CALLBACK(connected), self);

	priv->applications = NULL;

	return;
}

static void
indicator_custom_dispose (GObject *object)
{
	IndicatorCustomPrivate * priv = INDICATOR_CUSTOM_GET_PRIVATE(object);

	while (priv->applications != NULL) {
		application_removed(priv->service_proxy,
		                    0,
		                    INDICATOR_CUSTOM(object));
	}

	if (priv->sm != NULL) {
		g_object_unref(priv->sm);
		priv->sm = NULL;
	}

	if (priv->bus != NULL) {
		/* We're not incrementing the ref count on this one. */
		priv->bus = NULL;
	}

	if (priv->service_proxy != NULL) {
		g_object_unref(G_OBJECT(priv->service_proxy));
		priv->service_proxy = NULL;
	}

	G_OBJECT_CLASS (indicator_custom_parent_class)->dispose (object);
	return;
}

static void
indicator_custom_finalize (GObject *object)
{

	G_OBJECT_CLASS (indicator_custom_parent_class)->finalize (object);
	return;
}

void
connected (IndicatorServiceManager * sm, gboolean connected, IndicatorCustom * custom)
{
	IndicatorCustomPrivate * priv = INDICATOR_CUSTOM_GET_PRIVATE(custom);
	g_debug("Connected to Custom Indicator Service.");

	GError * error = NULL;

	/* Grab the session bus */
	if (priv->bus == NULL) {
		priv->bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);

		if (error != NULL) {
			g_error("Unable to get session bus: %s", error->message);
			g_error_free(error);
			return;
		}
	}

	/* Build the service proxy */
	priv->service_proxy = dbus_g_proxy_new_for_name_owner(priv->bus,
	                                                      INDICATOR_CUSTOM_DBUS_ADDR,
	                                                      INDICATOR_CUSTOM_DBUS_OBJ,
	                                                      INDICATOR_CUSTOM_DBUS_IFACE,
	                                                      &error);

	/* Set up proxy signals */
	g_debug("Setup proxy signals");
	dbus_g_proxy_add_signal(priv->service_proxy,
	                        "ApplicationAdded",
	                        G_TYPE_STRING,
	                        G_TYPE_INT,
	                        G_TYPE_STRING,
	                        G_TYPE_STRING,
	                        G_TYPE_INVALID);
	dbus_g_proxy_add_signal(priv->service_proxy,
	                        "ApplicationRemoved",
	                        G_TYPE_INT,
	                        G_TYPE_INVALID);

	/* Connect to them */
	g_debug("Connect to them.");
	dbus_g_proxy_connect_signal(priv->service_proxy,
	                            "ApplicationAdded",
	                            G_CALLBACK(application_added),
	                            custom,
	                            NULL /* Disconnection Signal */);
	dbus_g_proxy_connect_signal(priv->service_proxy,
	                            "ApplicationRemoved",
	                            G_CALLBACK(application_removed),
	                            custom,
	                            NULL /* Disconnection Signal */);

	/* Query it for existing applications */
	g_debug("Request current apps");
	org_ayatana_indicator_custom_service_get_applications_async(priv->service_proxy,
	                                                            get_applications,
	                                                            custom);

	return;
}

/* Goes through the list of applications that we're maintaining and
   pulls out the IndicatorObjectEntry and returns that in a list
   for the caller. */
static GList *
get_entries (IndicatorObject * io)
{
	g_return_val_if_fail(IS_INDICATOR_CUSTOM(io), NULL);

	IndicatorCustomPrivate * priv = INDICATOR_CUSTOM_GET_PRIVATE(io);
	GList * retval = NULL;
	GList * apppointer = NULL;

	for (apppointer = priv->applications; apppointer != NULL; apppointer = g_list_next(apppointer)) {
		IndicatorObjectEntry * entry = &(((ApplicationEntry *)apppointer->data)->entry);
		retval = g_list_prepend(retval, entry);
	}

	if (retval != NULL) {
		retval = g_list_reverse(retval);
	}

	return retval;
}

/* Here we respond to new applications by building up the
   ApplicationEntry and signaling the indicator host that
   we've got a new indicator. */
static void
application_added (DBusGProxy * proxy, const gchar * iconname, gint position, const gchar * dbusaddress, const gchar * dbusobject, IndicatorCustom * custom)
{
	IndicatorCustomPrivate * priv = INDICATOR_CUSTOM_GET_PRIVATE(custom);
	ApplicationEntry * app = g_new(ApplicationEntry, 1);

	app->entry.image = GTK_IMAGE(gtk_image_new_from_icon_name(iconname, GTK_ICON_SIZE_MENU));
	app->entry.label = NULL;
	app->entry.menu = GTK_MENU(dbusmenu_gtkmenu_new((gchar *)dbusaddress, (gchar *)dbusobject));

	priv->applications = g_list_insert(priv->applications, app, position);

	/* TODO: Need to deal with position here somehow */
	g_signal_emit(G_OBJECT(custom), INDICATOR_OBJECT_SIGNAL_ENTRY_ADDED_ID, 0, &(app->entry), TRUE);
	return;
}

/* This removes the application from the list and free's all
   of the memory associated with it. */
static void
application_removed (DBusGProxy * proxy, gint position, IndicatorCustom * custom)
{
	IndicatorCustomPrivate * priv = INDICATOR_CUSTOM_GET_PRIVATE(custom);
	ApplicationEntry * app = (ApplicationEntry *)g_list_nth_data(priv->applications, position);

	if (app == NULL) {
		g_warning("Unable to find application at position: %d", position);
		return;
	}

	priv->applications = g_list_remove(priv->applications, app);
	g_signal_emit(G_OBJECT(custom), INDICATOR_OBJECT_SIGNAL_ENTRY_REMOVED_ID, 0, &(app->entry), TRUE);

	if (app->entry.image != NULL) {
		g_object_unref(G_OBJECT(app->entry.image));
	}
	if (app->entry.label != NULL) {
		g_warning("Odd, an application indicator with a label?");
		g_object_unref(G_OBJECT(app->entry.label));
	}
	if (app->entry.menu != NULL) {
		g_object_unref(G_OBJECT(app->entry.menu));
	}
	g_free(app);

	return;
}

/* This repsonds to the list of applications that the service
   has and calls application_added on each one of them. */
static void
get_applications (DBusGProxy *proxy, GPtrArray *OUT_applications, GError *error, gpointer userdata)
{

	return;
}
