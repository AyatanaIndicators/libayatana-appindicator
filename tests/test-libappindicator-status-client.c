/*
Tests for the libappindicator library that are over DBus.  This is
the client side of those tests.

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


#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>
#include "../src/dbus-shared.h"

static GMainLoop * mainloop = NULL;
static gboolean passed = TRUE;
static gboolean watchdog_hit = TRUE;
static gboolean active = FALSE;
static guint toggle_count = 0;

#define PASSIVE_STR  "Passive"
#define ACTIVE_STR   "Active"
#define ATTN_STR     "NeedsAttention"

static DBusHandlerResult
dbus_reg_filter (DBusConnection * connection, DBusMessage * message, void * user_data)
{
	if (dbus_message_is_method_call(message, NOTIFICATION_WATCHER_DBUS_ADDR, "RegisterStatusNotifierItem")) {
		DBusMessage * reply = dbus_message_new_method_return(message);
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
		return DBUS_HANDLER_RESULT_HANDLED;
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


static DBusHandlerResult
dbus_filter (DBusConnection * connection, DBusMessage * message, void * user_data)
{
	if (!dbus_message_is_signal(message, NOTIFICATION_ITEM_DBUS_IFACE, "NewStatus")) {
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	gchar * string;

	DBusError derror;
	dbus_error_init(&derror);
	if (!dbus_message_get_args(message, &derror,
				DBUS_TYPE_STRING, &string,
				DBUS_TYPE_INVALID)) {
		g_warning("Couldn't get parameters");
		dbus_error_free(&derror);
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	watchdog_hit = TRUE;

	if (g_strcmp0(string, ACTIVE_STR) == 0) {
		if (active) {
			g_warning("Got active when already active");
			passed = FALSE;
			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
		}
		active = TRUE;
	} else {
		active = FALSE;
	}

	toggle_count++;

	if (toggle_count == 100) {
		g_main_loop_quit(mainloop);
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

gboolean
kill_func (gpointer userdata)
{
	if (watchdog_hit == FALSE) {
		g_main_loop_quit(mainloop);
		g_warning("Forced to Kill");
		g_warning("Toggle count: %d", toggle_count);
		passed = FALSE;
		return FALSE;
	}
	watchdog_hit = FALSE;
	return TRUE;
}

gint
main (gint argc, gchar * argv[])
{
	GError * error = NULL;
	DBusGConnection * session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_error("Unable to get session bus: %s", error->message);
		return 1;
	}

	DBusGProxy * bus_proxy = dbus_g_proxy_new_for_name(session_bus, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
	guint nameret = 0;

	if (!org_freedesktop_DBus_request_name(bus_proxy, NOTIFICATION_WATCHER_DBUS_ADDR, 0, &nameret, &error)) {
		g_error("Unable to call to request name");
		return 1;
	}   

	if (nameret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		g_error("Unable to get name");
		return 1;
	}

	dbus_connection_add_filter(dbus_g_connection_get_connection(session_bus), dbus_reg_filter, NULL, NULL);

	dbus_connection_add_filter(dbus_g_connection_get_connection(session_bus), dbus_filter, NULL, NULL);
	dbus_bus_add_match(dbus_g_connection_get_connection(session_bus), "type='signal',interface='" NOTIFICATION_ITEM_DBUS_IFACE "',member='NewStatus'", NULL);

	watchdog_hit = TRUE;
	g_timeout_add_seconds(20, kill_func, NULL);

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
