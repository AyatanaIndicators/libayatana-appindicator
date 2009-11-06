#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dbus/dbus-glib.h>
#include "custom-service-appstore.h"
#include "dbus-shared.h"

/* DBus Prototypes */
static gboolean _custom_service_server_get_applications (CustomServiceAppstore * appstore, GArray ** apps);

static gboolean _notification_watcher_server_register_service (CustomServiceAppstore * appstore, const gchar * service, DBusGMethodInvocation * method);
static gboolean _notification_watcher_server_registered_services (CustomServiceAppstore * appstore, GArray ** apps);
static gboolean _notification_watcher_server_protocol_version (CustomServiceAppstore * appstore, char ** version);
static gboolean _notification_watcher_server_register_notification_host (CustomServiceAppstore * appstore, const gchar * host);
static gboolean _notification_watcher_server_is_notification_host_registered (CustomServiceAppstore * appstore, gboolean * haveHost);

#include "custom-service-server.h"
#include "notification-watcher-server.h"

/* Private Stuff */
typedef struct _CustomServiceAppstorePrivate CustomServiceAppstorePrivate;
struct _CustomServiceAppstorePrivate {
	int demo;
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
	                                           g_cclosure_marshal_VOID__POINTER,
	                                           G_TYPE_NONE, 4, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_NONE);
	signals[APPLICATION_REMOVED] = g_signal_new ("application-removed",
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET (CustomServiceAppstore, application_removed),
	                                           NULL, NULL,
	                                           g_cclosure_marshal_VOID__INT,
	                                           G_TYPE_NONE, 1, G_TYPE_INT, G_TYPE_NONE);


	dbus_g_object_type_install_info(CUSTOM_SERVICE_APPSTORE_TYPE,
	                                &dbus_glib__notification_watcher_server_object_info);
	dbus_g_object_type_install_info(CUSTOM_SERVICE_APPSTORE_TYPE,
	                                &dbus_glib__custom_service_server_object_info);

	return;
}

static void
custom_service_appstore_init (CustomServiceAppstore *self)
{
	
	GError * error = NULL;
	DBusGConnection * session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_error("Unable to get session bus: %s", error->message);
		g_error_free(error);
		return;
	}

	dbus_g_connection_register_g_object(session_bus,
	                                    INDICATOR_CUSTOM_DBUS_OBJ,
	                                    G_OBJECT(self));

	return;
}

static void
custom_service_appstore_dispose (GObject *object)
{

	G_OBJECT_CLASS (custom_service_appstore_parent_class)->dispose (object);
	return;
}

static void
custom_service_appstore_finalize (GObject *object)
{

	G_OBJECT_CLASS (custom_service_appstore_parent_class)->finalize (object);
	return;
}

/* DBus Interface */
static gboolean
_custom_service_server_get_applications (CustomServiceAppstore * appstore, GArray ** apps)
{

	return FALSE;
}

static gboolean
_notification_watcher_server_register_service (CustomServiceAppstore * appstore, const gchar * service, DBusGMethodInvocation * method)
{



	dbus_g_method_return(method, G_TYPE_NONE);
	return TRUE;
}

static gboolean
_notification_watcher_server_registered_services (CustomServiceAppstore * appstore, GArray ** apps)
{

	return FALSE;
}

static gboolean
_notification_watcher_server_protocol_version (CustomServiceAppstore * appstore, char ** version)
{
	*version = g_strdup("Ayatana Version 1");
	return TRUE;
}

static gboolean
_notification_watcher_server_register_notification_host (CustomServiceAppstore * appstore, const gchar * host)
{

	return FALSE;
}

static gboolean
_notification_watcher_server_is_notification_host_registered (CustomServiceAppstore * appstore, gboolean * haveHost)
{
	*haveHost = TRUE;
	return TRUE;
}

