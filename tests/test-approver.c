#include <glib.h>
#include <glib-object.h>

#include <dbus/dbus-glib-bindings.h>

#include "notification-watcher-client.h"
#include "dbus-shared.h"
#include "app-indicator.h"

#define APPROVER_PATH  "/my/approver"

#define INDICATOR_ID        "test-indicator-id"
#define INDICATOR_ICON      "test-indicator-icon-name"
#define INDICATOR_CATEGORY  APP_INDICATOR_CATEGORY_APPLICATION_STATUS

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
static gboolean _notification_approver_server_approve_item (TestApprover * ta, const gchar * id, const gchar * category, guint pid, const gchar * address, const gchar * path, gboolean * approved, GError ** error);

#include "../src/notification-approver-server.h"

GMainLoop * main_loop = NULL;
DBusGConnection * session_bus = NULL;
DBusGProxy * bus_proxy = NULL;
AppIndicator * app_indicator = NULL;
gboolean passed = FALSE;

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

static gboolean 
_notification_approver_server_approve_item (TestApprover * ta, const gchar * id, const gchar * category, guint pid, const gchar * address, const gchar * path, gboolean * approved, GError ** error)
{
	*approved = TRUE;
	g_debug("Asked to approve indicator");

	if (g_strcmp0(id, INDICATOR_ID) == 0) {
		passed = TRUE;
	}

	g_main_loop_quit(main_loop);

	return TRUE;
}

static void
register_cb (DBusGProxy * proxy, GError * error, gpointer user_data)
{
	if (error != NULL) {
		g_warning("Unable to register approver: %s", error->message);
		g_error_free(error);
		g_main_loop_quit(main_loop);
		return;
	}

	g_debug("Building App Indicator");
	app_indicator = app_indicator_new(INDICATOR_ID, INDICATOR_ICON, INDICATOR_CATEGORY);

	GtkWidget * menu = gtk_menu_new();
	GtkWidget * mi = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

	app_indicator_set_menu(app_indicator, GTK_MENU(menu));

	return;
}

gint owner_count = 0;
gboolean
check_for_service (gpointer user_data)
{
	g_debug("Checking for Watcher");

	if (owner_count > 100) {
		g_warning("Couldn't find watcher after 100 tries.");
		g_main_loop_quit(main_loop);
		return FALSE;
	}

	owner_count++;

	gboolean has_owner = FALSE;
	org_freedesktop_DBus_name_has_owner(bus_proxy, NOTIFICATION_WATCHER_DBUS_ADDR, &has_owner, NULL);

	if (has_owner) {
		const char * cats = NULL;
		DBusGProxy * proxy = dbus_g_proxy_new_for_name(session_bus,
		                                               NOTIFICATION_WATCHER_DBUS_ADDR,
		                                               NOTIFICATION_WATCHER_DBUS_OBJ,
		                                               NOTIFICATION_WATCHER_DBUS_IFACE);

		g_debug("Registering Approver");
		org_kde_StatusNotifierWatcher_register_notification_approver_async (proxy, APPROVER_PATH, &cats, register_cb, NULL);

		return FALSE;
	}

	return TRUE;
}

gboolean
fail_timeout (gpointer user_data)
{
	g_debug("Failure timeout initiated.");
	g_main_loop_quit(main_loop);
	return FALSE;
}

int
main (int argc, char ** argv)
{
	GError * error = NULL;

	gtk_init(&argc, &argv);
	g_debug("Initing");

	session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_warning("Unable to get session bus: %s", error->message);
		g_error_free(error);
		return -1;
	}

	TestApprover * approver = g_object_new(TEST_APPROVER_TYPE, NULL);

	bus_proxy = dbus_g_proxy_new_for_name(session_bus, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);

	g_timeout_add(100, check_for_service, NULL);
	g_timeout_add_seconds(2, fail_timeout, NULL);

	main_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(main_loop);

	g_object_unref(approver);

	if (!passed) {
		return -1;
	}

	return 0;
}
