/*
Test that creates a small test app which links with libappindicator.

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
#include <libdbusmenu-glib/server.h>
#include <app-indicator.h>

static GMainLoop * mainloop = NULL;

int
main (int argc, char ** argv)
{
	g_type_init();

	DbusmenuServer * dms = dbusmenu_server_new("/menu");
	DbusmenuMenuitem * dmi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(dmi, "label", "Bob");

	AppIndicator * ci = APP_INDICATOR(g_object_new(APP_INDICATOR_TYPE, 
	                                               "id", "test-application",
	                                               "status-enum", APP_INDICATOR_STATUS_ACTIVE,
	                                               "icon-name", "system-shutdown",
	                                               "menu-object", dms,
	                                               NULL));

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_object_unref(G_OBJECT(ci));
	g_debug("Quiting");

	return 0;
}
