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

#include "../src/dbus-shared.h"

static GMainLoop * mainloop = NULL;

gboolean
kill_func (gpointer userdata)
{
	g_main_loop_quit(mainloop);
	return FALSE;
}

int
main (int argv, char ** argc)
{
	g_type_init();

	/* Wait 1/4 a second, which should trigger the fallback */
	g_usleep(250000);

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

	/* After we've got the name, let it unfallback, and then we'll drop again */
	g_timeout_add(250, kill_func, NULL);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	return 0;
}
