#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include "custom-service-watcher.h"
#include "dbus-shared.h"

static gboolean _notification_watcher_server_register_service (CustomServiceWatcher * appwatcher, const gchar * service, DBusGMethodInvocation * method);
static gboolean _notification_watcher_server_registered_services (CustomServiceWatcher * appwatcher, GArray ** apps);
static gboolean _notification_watcher_server_protocol_version (CustomServiceWatcher * appwatcher, char ** version);
static gboolean _notification_watcher_server_register_notification_host (CustomServiceWatcher * appwatcher, const gchar * host);
static gboolean _notification_watcher_server_is_notification_host_registered (CustomServiceWatcher * appwatcher, gboolean * haveHost);

#include "notification-watcher-server.h"

/* Private Stuff */
typedef struct _CustomServiceWatcherPrivate CustomServiceWatcherPrivate;
struct _CustomServiceWatcherPrivate {
	CustomServiceAppstore * appstore;
};

#define CUSTOM_SERVICE_WATCHER_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), CUSTOM_SERVICE_WATCHER_TYPE, CustomServiceWatcherPrivate))

/* Signals Stuff */
enum {
	SERVICE_REGISTERED,
	SERVICE_UNREGISTERED,
	NOTIFICATION_HOST_REGISTERED,
	NOTIFICATION_HOST_UNREGISTERED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* GObject stuff */
static void custom_service_watcher_class_init (CustomServiceWatcherClass *klass);
static void custom_service_watcher_init       (CustomServiceWatcher *self);
static void custom_service_watcher_dispose    (GObject *object);
static void custom_service_watcher_finalize   (GObject *object);

G_DEFINE_TYPE (CustomServiceWatcher, custom_service_watcher, G_TYPE_OBJECT);

static void
custom_service_watcher_class_init (CustomServiceWatcherClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (CustomServiceWatcherPrivate));

	object_class->dispose = custom_service_watcher_dispose;
	object_class->finalize = custom_service_watcher_finalize;

	signals[SERVICE_REGISTERED] = g_signal_new ("service-registered",
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET (CustomServiceWatcherClass, service_registered),
	                                           NULL, NULL,
	                                           g_cclosure_marshal_VOID__STRING,
	                                           G_TYPE_NONE, 1, G_TYPE_STRING, G_TYPE_NONE);
	signals[SERVICE_UNREGISTERED] = g_signal_new ("service-unregistered",
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET (CustomServiceWatcherClass, service_unregistered),
	                                           NULL, NULL,
	                                           g_cclosure_marshal_VOID__STRING,
	                                           G_TYPE_NONE, 1, G_TYPE_STRING, G_TYPE_NONE);
	signals[NOTIFICATION_HOST_REGISTERED] = g_signal_new ("notification-host-registered",
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET (CustomServiceWatcherClass, notification_host_registered),
	                                           NULL, NULL,
	                                           g_cclosure_marshal_VOID__VOID,
	                                           G_TYPE_NONE, 0, G_TYPE_NONE);
	signals[NOTIFICATION_HOST_UNREGISTERED] = g_signal_new ("notification-host-unregistered",
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET (CustomServiceWatcherClass, notification_host_unregistered),
	                                           NULL, NULL,
	                                           g_cclosure_marshal_VOID__VOID,
	                                           G_TYPE_NONE, 0, G_TYPE_NONE);

	dbus_g_object_type_install_info(CUSTOM_SERVICE_WATCHER_TYPE,
	                                &dbus_glib__notification_watcher_server_object_info);

	return;
}

static void
custom_service_watcher_init (CustomServiceWatcher *self)
{
	CustomServiceWatcherPrivate * priv = CUSTOM_SERVICE_WATCHER_GET_PRIVATE(self);

	priv->appstore = NULL;

	GError * error = NULL;
	DBusGConnection * session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_error("Unable to get session bus: %s", error->message);
		g_error_free(error);
		return;
	}

	dbus_g_connection_register_g_object(session_bus,
	                                    NOTIFICATION_WATCHER_DBUS_OBJ,
	                                    G_OBJECT(self));

	return;
}

static void
custom_service_watcher_dispose (GObject *object)
{
	CustomServiceWatcherPrivate * priv = CUSTOM_SERVICE_WATCHER_GET_PRIVATE(object);
	
	if (priv->appstore != NULL) {
		g_object_unref(G_OBJECT(priv->appstore));
		priv->appstore = NULL;
	}

	G_OBJECT_CLASS (custom_service_watcher_parent_class)->dispose (object);
	return;
}

static void
custom_service_watcher_finalize (GObject *object)
{

	G_OBJECT_CLASS (custom_service_watcher_parent_class)->finalize (object);
	return;
}

CustomServiceWatcher *
custom_service_watcher_new (CustomServiceAppstore * appstore)
{
	GObject * obj = g_object_new(CUSTOM_SERVICE_WATCHER_TYPE, NULL);
	CustomServiceWatcherPrivate * priv = CUSTOM_SERVICE_WATCHER_GET_PRIVATE(obj);
	priv->appstore = appstore;
	g_object_ref(G_OBJECT(priv->appstore));
	return CUSTOM_SERVICE_WATCHER(obj);
}

static gboolean
_notification_watcher_server_register_service (CustomServiceWatcher * appwatcher, const gchar * service, DBusGMethodInvocation * method)
{
	CustomServiceWatcherPrivate * priv = CUSTOM_SERVICE_WATCHER_GET_PRIVATE(appwatcher);

	custom_service_appstore_application_add(priv->appstore, dbus_g_method_get_sender(method), service);

	dbus_g_method_return(method, G_TYPE_NONE);
	return TRUE;
}

static gboolean
_notification_watcher_server_registered_services (CustomServiceWatcher * appwatcher, GArray ** apps)
{

	return FALSE;
}

static gboolean
_notification_watcher_server_protocol_version (CustomServiceWatcher * appwatcher, char ** version)
{
	*version = g_strdup("Ayatana Version 1");
	return TRUE;
}

static gboolean
_notification_watcher_server_register_notification_host (CustomServiceWatcher * appwatcher, const gchar * host)
{

	return FALSE;
}

static gboolean
_notification_watcher_server_is_notification_host_registered (CustomServiceWatcher * appwatcher, gboolean * haveHost)
{
	*haveHost = TRUE;
	return TRUE;
}

