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

#include <glib.h>
#include <app-indicator.h>

static GMainLoop * mainloop = NULL;

int
main (int argc, char ** argv)
{
	gtk_init(&argc, &argv);
	GtkWidget *menu_item = gtk_menu_item_new_with_label("Test");
	GtkMenu *menu = GTK_MENU(gtk_menu_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show(menu_item);

	AppIndicator * ci = app_indicator_new("test-application",
	                                      "system-shutdown",
	                                      APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	app_indicator_set_status(ci, APP_INDICATOR_STATUS_ACTIVE);
	app_indicator_set_menu(ci, menu);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_object_unref(G_OBJECT(ci));
	g_object_unref(G_OBJECT(menu));
	g_object_unref(G_OBJECT(menu_item));
	g_debug("Quiting");

	return 0;
}
