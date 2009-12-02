/*
Tests for the libappindicator library that are over DBus.  This is
the server side of those tests.

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


#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <glib.h>
#include <libappindicator/app-indicator.h>
#include "test-defines.h"

static GMainLoop * mainloop = NULL;

gboolean
kill_func (gpointer userdata)
{
	g_main_loop_quit(mainloop);
	return FALSE;
}

gint
main (gint argc, gchar * argv[])
{
	g_type_init();

	g_debug("DBus ID: %s", dbus_connection_get_server_id(dbus_g_connection_get_connection(dbus_g_bus_get(DBUS_BUS_SESSION, NULL))));

	DbusmenuServer * dms = dbusmenu_server_new(TEST_OBJECT);

	AppIndicator * ci = APP_INDICATOR(g_object_new(APP_INDICATOR_TYPE, 
	                                               "id", TEST_ID,
	                                               "category-enum", TEST_CATEGORY,
	                                               "status-enum", TEST_STATE,
	                                               "icon-name", TEST_ICON_NAME,
	                                               "attention-icon-name", TEST_ATTENTION_ICON_NAME,
	                                               "menu-object", dms,
	                                               NULL));

	g_timeout_add_seconds(2, kill_func, NULL);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_object_unref(G_OBJECT(ci));
	g_debug("Quiting");

	return 0;
}
