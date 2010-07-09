#include <glib.h>
#include <glib-object.h>

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
	DBusGConnection * session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);

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

int
main (int argc, char ** argv)
{
	g_type_init();

	TestApprover * approver = g_object_new(TEST_APPROVER_TYPE, NULL);

	GMainLoop * main_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(main_loop);

	g_object_unref(approver);

	return 0;
}
