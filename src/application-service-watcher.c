/*
An object implementing the NotificationWatcher interface and passes
the information into the app-store.

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
#include <dbus/dbus-glib-lowlevel.h>
#include "application-service-watcher.h"
#include "dbus-shared.h"

static gboolean _notification_watcher_server_register_service (ApplicationServiceWatcher * appwatcher, const gchar * service, DBusGMethodInvocation * method);
static gboolean _notification_watcher_server_registered_services (ApplicationServiceWatcher * appwatcher, GArray ** apps);
static gboolean _notification_watcher_server_protocol_version (ApplicationServiceWatcher * appwatcher, char ** version);
static gboolean _notification_watcher_server_register_notification_host (ApplicationServiceWatcher * appwatcher, const gchar * host);
static gboolean _notification_watcher_server_is_notification_host_registered (ApplicationServiceWatcher * appwatcher, gboolean * haveHost);

#include "notification-watcher-server.h"

/* Private Stuff */
typedef struct _ApplicationServiceWatcherPrivate ApplicationServiceWatcherPrivate;
struct _ApplicationServiceWatcherPrivate {
	ApplicationServiceAppstore * appstore;
};

#define APPLICATION_SERVICE_WATCHER_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), APPLICATION_SERVICE_WATCHER_TYPE, ApplicationServiceWatcherPrivate))

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
static void application_service_watcher_class_init (ApplicationServiceWatcherClass *klass);
static void application_service_watcher_init       (ApplicationServiceWatcher *self);
static void application_service_watcher_dispose    (GObject *object);
static void application_service_watcher_finalize   (GObject *object);

G_DEFINE_TYPE (ApplicationServiceWatcher, application_service_watcher, G_TYPE_OBJECT);

static void
application_service_watcher_class_init (ApplicationServiceWatcherClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (ApplicationServiceWatcherPrivate));

	object_class->dispose = application_service_watcher_dispose;
	object_class->finalize = application_service_watcher_finalize;

	signals[SERVICE_REGISTERED] = g_signal_new ("service-registered",
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET (ApplicationServiceWatcherClass, service_registered),
	                                           NULL, NULL,
	                                           g_cclosure_marshal_VOID__STRING,
	                                           G_TYPE_NONE, 1, G_TYPE_STRING, G_TYPE_NONE);
	signals[SERVICE_UNREGISTERED] = g_signal_new ("service-unregistered",
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET (ApplicationServiceWatcherClass, service_unregistered),
	                                           NULL, NULL,
	                                           g_cclosure_marshal_VOID__STRING,
	                                           G_TYPE_NONE, 1, G_TYPE_STRING, G_TYPE_NONE);
	signals[NOTIFICATION_HOST_REGISTERED] = g_signal_new ("notification-host-registered",
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET (ApplicationServiceWatcherClass, notification_host_registered),
	                                           NULL, NULL,
	                                           g_cclosure_marshal_VOID__VOID,
	                                           G_TYPE_NONE, 0, G_TYPE_NONE);
	signals[NOTIFICATION_HOST_UNREGISTERED] = g_signal_new ("notification-host-unregistered",
	                                           G_TYPE_FROM_CLASS(klass),
	                                           G_SIGNAL_RUN_LAST,
	                                           G_STRUCT_OFFSET (ApplicationServiceWatcherClass, notification_host_unregistered),
	                                           NULL, NULL,
	                                           g_cclosure_marshal_VOID__VOID,
	                                           G_TYPE_NONE, 0, G_TYPE_NONE);

	dbus_g_object_type_install_info(APPLICATION_SERVICE_WATCHER_TYPE,
	                                &dbus_glib__notification_watcher_server_object_info);

	return;
}

static void
application_service_watcher_init (ApplicationServiceWatcher *self)
{
	ApplicationServiceWatcherPrivate * priv = APPLICATION_SERVICE_WATCHER_GET_PRIVATE(self);

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
application_service_watcher_dispose (GObject *object)
{
	ApplicationServiceWatcherPrivate * priv = APPLICATION_SERVICE_WATCHER_GET_PRIVATE(object);
	
	if (priv->appstore != NULL) {
		g_object_unref(G_OBJECT(priv->appstore));
		priv->appstore = NULL;
	}

	G_OBJECT_CLASS (application_service_watcher_parent_class)->dispose (object);
	return;
}

static void
application_service_watcher_finalize (GObject *object)
{

	G_OBJECT_CLASS (application_service_watcher_parent_class)->finalize (object);
	return;
}

ApplicationServiceWatcher *
application_service_watcher_new (ApplicationServiceAppstore * appstore)
{
	GObject * obj = g_object_new(APPLICATION_SERVICE_WATCHER_TYPE, NULL);
	ApplicationServiceWatcherPrivate * priv = APPLICATION_SERVICE_WATCHER_GET_PRIVATE(obj);
	priv->appstore = appstore;
	g_object_ref(G_OBJECT(priv->appstore));
	return APPLICATION_SERVICE_WATCHER(obj);
}

static gboolean
_notification_watcher_server_register_service (ApplicationServiceWatcher * appwatcher, const gchar * service, DBusGMethodInvocation * method)
{
	ApplicationServiceWatcherPrivate * priv = APPLICATION_SERVICE_WATCHER_GET_PRIVATE(appwatcher);

	application_service_appstore_application_add(priv->appstore, dbus_g_method_get_sender(method), service);

	dbus_g_method_return(method, G_TYPE_NONE);
	return TRUE;
}

static gboolean
_notification_watcher_server_registered_services (ApplicationServiceWatcher * appwatcher, GArray ** apps)
{

	return FALSE;
}

static gboolean
_notification_watcher_server_protocol_version (ApplicationServiceWatcher * appwatcher, char ** version)
{
	*version = g_strdup("Ayatana Version 1");
	return TRUE;
}

static gboolean
_notification_watcher_server_register_notification_host (ApplicationServiceWatcher * appwatcher, const gchar * host)
{

	return FALSE;
}

static gboolean
_notification_watcher_server_is_notification_host_registered (ApplicationServiceWatcher * appwatcher, gboolean * haveHost)
{
	*haveHost = TRUE;
	return TRUE;
}

