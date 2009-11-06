#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dbus/dbus-glib.h>
#include "custom-service-watcher.h"
#include "dbus-shared.h"

static gboolean _notification_watcher_server_register_service (CustomServiceWatcher * appstore, const gchar * service, DBusGMethodInvocation * method);
static gboolean _notification_watcher_server_registered_services (CustomServiceWatcher * appstore, GArray ** apps);
static gboolean _notification_watcher_server_protocol_version (CustomServiceWatcher * appstore, char ** version);
static gboolean _notification_watcher_server_register_notification_host (CustomServiceWatcher * appstore, const gchar * host);
static gboolean _notification_watcher_server_is_notification_host_registered (CustomServiceWatcher * appstore, gboolean * haveHost);

#include "notification-watcher-server.h"

typedef struct _CustomServiceWatcherPrivate CustomServiceWatcherPrivate;
struct _CustomServiceWatcherPrivate {
	int dummy;
};

#define CUSTOM_SERVICE_WATCHER_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), CUSTOM_SERVICE_WATCHER_TYPE, CustomServiceWatcherPrivate))

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

	dbus_g_object_type_install_info(CUSTOM_SERVICE_WATCHER_TYPE,
	                                &dbus_glib__notification_watcher_server_object_info);

	return;
}

static void
custom_service_watcher_init (CustomServiceWatcher *self)
{
	GError * error = NULL;
	DBusGConnection * session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_error("Unable to get session bus: %s", error->message);
		g_error_free(error);
		return;
	}

	dbus_g_connection_register_g_object(session_bus,
	                                    INDICATOR_CUSTOM_DBUS_OBJ "/more",
	                                    G_OBJECT(self));

	return;
}

static void
custom_service_watcher_dispose (GObject *object)
{

	G_OBJECT_CLASS (custom_service_watcher_parent_class)->dispose (object);
	return;
}

static void
custom_service_watcher_finalize (GObject *object)
{

	G_OBJECT_CLASS (custom_service_watcher_parent_class)->finalize (object);
	return;
}

static gboolean
_notification_watcher_server_register_service (CustomServiceWatcher * appstore, const gchar * service, DBusGMethodInvocation * method)
{



	dbus_g_method_return(method, G_TYPE_NONE);
	return TRUE;
}

static gboolean
_notification_watcher_server_registered_services (CustomServiceWatcher * appstore, GArray ** apps)
{

	return FALSE;
}

static gboolean
_notification_watcher_server_protocol_version (CustomServiceWatcher * appstore, char ** version)
{
	*version = g_strdup("Ayatana Version 1");
	return TRUE;
}

static gboolean
_notification_watcher_server_register_notification_host (CustomServiceWatcher * appstore, const gchar * host)
{

	return FALSE;
}

static gboolean
_notification_watcher_server_is_notification_host_registered (CustomServiceWatcher * appstore, gboolean * haveHost)
{
	*haveHost = TRUE;
	return TRUE;
}

