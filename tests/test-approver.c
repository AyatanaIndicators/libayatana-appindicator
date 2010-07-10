#include <glib.h>
#include <glib-object.h>

#include <dbus/dbus-glib-bindings.h>

#include "notification-watcher-client.h"
#include "dbus-shared.h"

#define APPROVER_PATH  "/my/approver"

#define TEST_APPROVER_TYPE            (test_approver_get_type ())
#define TEST_APPROVER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEST_APPROVER_TYPE, TestApprover))
#define TEST_APPROVER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TEST_APPROVER_TYPE, TestApproverClass))
#define IS_TEST_APPROVER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEST_APPROVER_TYPE))
#define IS_TEST_APPROVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TEST_APPROVER_TYPE))
#define TEST_APPROVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TEST_APPROVER_TYPE, TestApproverClass))

typedef struct _TestApprover      TestApprover;
typedef struct _TestApproverClass TestApproverClass;

struct _TestApproverClass {
	GObjectClass parent_class;
};

struct _TestApprover {
	GObject parent;
};

GType test_approver_get_type (void);

static void test_approver_class_init (TestApproverClass *klass);
static void test_approver_init       (TestApprover *self);
static void _notification_approver_server_approve_item (void);

#include "../src/notification-approver-server.h"

GMainLoop * main_loop = NULL;
DBusGConnection * session_bus = NULL;
DBusGProxy * bus_proxy = NULL;

G_DEFINE_TYPE (TestApprover, test_approver, G_TYPE_OBJECT);

static void
test_approver_class_init (TestApproverClass *klass)
{
	dbus_g_object_type_install_info(TEST_APPROVER_TYPE,
	                                &dbus_glib__notification_approver_server_object_info);

	return;
}

static void
test_approver_init (TestApprover *self)
{
	dbus_g_connection_register_g_object(session_bus,
	                                    APPROVER_PATH,
	                                    G_OBJECT(self));

	return;
}

static void
_notification_approver_server_approve_item (void)
{
	return;
}

gint owner_count = 0;
gboolean
check_for_service (gpointer user_data)
{
	g_debug("Checking for Watcher");

	if (owner_count > 100) {
		g_main_loop_quit(main_loop);
		return FALSE;
	}

	owner_count++;

	gboolean has_owner = FALSE;
	org_freedesktop_DBus_name_has_owner(bus_proxy, "org.test", &has_owner, NULL);

	if (has_owner) {
		const char * cats = NULL;
		DBusGProxy * proxy = dbus_g_proxy_new_for_name(session_bus,
		                                               NOTIFICATION_WATCHER_DBUS_ADDR,
		                                               NOTIFICATION_WATCHER_DBUS_OBJ,
		                                               NOTIFICATION_WATCHER_DBUS_IFACE);

		g_debug("Registering Approver");
		org_kde_StatusNotifierWatcher_register_notification_approver (proxy, APPROVER_PATH, &cats, NULL);
		return FALSE;
	}

	return TRUE;
}

int
main (int argc, char ** argv)
{
	g_type_init();
	g_debug("Initing");

	session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);

	TestApprover * approver = g_object_new(TEST_APPROVER_TYPE, NULL);

	bus_proxy = dbus_g_proxy_new_for_name(session_bus, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);

	g_timeout_add(100, check_for_service, NULL);

	main_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(main_loop);

	g_object_unref(approver);

	return 0;
}
