/*
This puts the NotificationWatcher on the bus, kinda.  Enough to
trick the Item into unfalling back.

Copyright 2010 Canonical Ltd.

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

gboolean kill_func (gpointer userdata);

static GMainLoop * mainloop = NULL;

static DBusHandlerResult
dbus_filter (DBusConnection * connection, DBusMessage * message, void * user_data)
{
	if (dbus_message_is_method_call(message, NOTIFICATION_WATCHER_DBUS_ADDR, "RegisterStatusNotifierItem")) {
		DBusMessage * reply = dbus_message_new_method_return(message);
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);

		/* Let the messages get out, but we're done at this point */
		g_timeout_add(50, kill_func, NULL);

		return DBUS_HANDLER_RESULT_HANDLED;
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

gboolean
kill_func (gpointer userdata)
{
	g_main_loop_quit(mainloop);
	return FALSE;
}

int
main (int argv, char ** argc)
{
	g_debug("Waiting to init.");


	GError * error = NULL;
	DBusGConnection * session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_error("Unable to get session bus: %s", error->message);
		return 1;
	}

    DBusGProxy * bus_proxy = dbus_g_proxy_new_for_name(session_bus, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);

	gboolean has_owner = FALSE;
	gint owner_count = 0;
	while (!has_owner && owner_count < 10000) {
		org_freedesktop_DBus_name_has_owner(bus_proxy, "org.test", &has_owner, NULL);
		owner_count++;
		g_usleep(500000);
	}

	if (owner_count == 10000) {
		g_error("Unable to get name owner after 10000 tries");
		return 1;
	}

	g_usleep(500000);

	g_debug("Initing");

	guint nameret = 0;

	if (!org_freedesktop_DBus_request_name(bus_proxy, NOTIFICATION_WATCHER_DBUS_ADDR, 0, &nameret, &error)) {
		g_error("Unable to call to request name");
		return 1;
	}   

	if (nameret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		g_error("Unable to get name");
		return 1;
	}

	dbus_connection_add_filter(dbus_g_connection_get_connection(session_bus), dbus_filter, NULL, NULL);

	/* This is the final kill function.  It really shouldn't happen
	   unless we get an error. */
	g_timeout_add_seconds(20, kill_func, NULL);

	g_debug("Entering Mainloop");

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_debug("Exiting");

	return 0;
}
