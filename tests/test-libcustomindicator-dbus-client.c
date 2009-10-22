
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <libcustomindicator/custom-indicator.h>
#include "test-defines.h"

static GMainLoop * mainloop = NULL;
static gboolean passed = TRUE;
static int propcount = 0;

static void
check_propcount (void)
{
	if (propcount >= 6) {
		g_main_loop_quit(mainloop);
	}
	return;
}


static void
prop_id_cb (DBusGProxy * proxy, DBusGProxyCall * call, void * data)
{
	propcount++;

	GError * error = NULL;
	GValue value = {0};

	if (!dbus_g_proxy_end_call(proxy, call, &error, G_TYPE_VALUE, &value, G_TYPE_INVALID)) {
		g_warning("Getting ID failed: %s", error->message);
		g_error_free(error);
		passed = FALSE;
		check_propcount();
		return;
	}

	if (g_strcmp0(TEST_ID, g_value_get_string(&value))) {
		g_debug("Property ID Returned: FAILED");
		passed = FALSE;
	} else {
		g_debug("Property ID Returned: PASSED");
	}

	check_propcount();
	return;
}

static void
prop_category_cb (DBusGProxy * proxy, DBusGProxyCall * call, void * data)
{
	propcount++;

	check_propcount();
	return;
}

static void
prop_status_cb (DBusGProxy * proxy, DBusGProxyCall * call, void * data)
{
	propcount++;

	check_propcount();
	return;
}

static void
prop_icon_name_cb (DBusGProxy * proxy, DBusGProxyCall * call, void * data)
{
	propcount++;

	check_propcount();
	return;
}

static void
prop_attention_icon_name_cb (DBusGProxy * proxy, DBusGProxyCall * call, void * data)
{
	propcount++;

	check_propcount();
	return;
}

static void
prop_menu_cb (DBusGProxy * proxy, DBusGProxyCall * call, void * data)
{
	propcount++;

	check_propcount();
	return;
}

gboolean
kill_func (gpointer userdata)
{
	g_main_loop_quit(mainloop);
	g_warning("Forced to Kill");
	passed = FALSE;
	return FALSE;
}

gint
main (gint argc, gchar * argv[])
{
	g_type_init();

	g_usleep(500000);

	GError * error = NULL;
	DBusGConnection * session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_error("Unable to get session bus: %s", error->message);
		return 1;
	}

	DBusGProxy * props = dbus_g_proxy_new_for_name_owner(session_bus,
	                                                     ":1.0",
	                                                     "/need/a/path",
	                                                     DBUS_INTERFACE_PROPERTIES,
	                                                     &error);
	if (error != NULL) {
		g_error("Unable to get property proxy: %s", error->message);
		return 1;
	}

	dbus_g_proxy_begin_call (props,
	                         "Get",
	                         prop_id_cb,
	                         NULL, NULL,
	                         G_TYPE_STRING, "org.ayatana.indicator.custom.NotificationItem",
	                         G_TYPE_STRING, "Id",
	                         G_TYPE_INVALID);
	dbus_g_proxy_begin_call (props,
	                         "Get",
	                         prop_category_cb,
	                         NULL, NULL,
	                         G_TYPE_STRING, "org.ayatana.indicator.custom.NotificationItem",
	                         G_TYPE_STRING, "Category",
	                         G_TYPE_INVALID);
	dbus_g_proxy_begin_call (props,
	                         "Get",
	                         prop_status_cb,
	                         NULL, NULL,
	                         G_TYPE_STRING, "org.ayatana.indicator.custom.NotificationItem",
	                         G_TYPE_STRING, "Status",
	                         G_TYPE_INVALID);
	dbus_g_proxy_begin_call (props,
	                         "Get",
	                         prop_icon_name_cb,
	                         NULL, NULL,
	                         G_TYPE_STRING, "org.ayatana.indicator.custom.NotificationItem",
	                         G_TYPE_STRING, "IconName",
	                         G_TYPE_INVALID);
	dbus_g_proxy_begin_call (props,
	                         "Get",
	                         prop_attention_icon_name_cb,
	                         NULL, NULL,
	                         G_TYPE_STRING, "org.ayatana.indicator.custom.NotificationItem",
	                         G_TYPE_STRING, "AttentionIconName",
	                         G_TYPE_INVALID);
	dbus_g_proxy_begin_call (props,
	                         "Get",
	                         prop_menu_cb,
	                         NULL, NULL,
	                         G_TYPE_STRING, "org.ayatana.indicator.custom.NotificationItem",
	                         G_TYPE_STRING, "Menu",
	                         G_TYPE_INVALID);

	g_timeout_add_seconds(2, kill_func, NULL);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	if (passed) {
		g_debug("Quiting");
		return 0;
	} else {
		g_debug("Quiting as we're a failure");
		return 1;
	}
	return 0;
}
