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


#include <gtk/gtk.h>
#include <app-indicator.h>
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
	gtk_init(&argc, &argv);

	AppIndicator * ci = app_indicator_new (TEST_ID, TEST_ICON_NAME, TEST_CATEGORY);

	app_indicator_set_status (ci, TEST_STATE);
	app_indicator_set_attention_icon (ci, TEST_ATTENTION_ICON_NAME);

	GtkMenu * menu = GTK_MENU(gtk_menu_new());
	GtkMenuItem * item = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Label"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(item));

	app_indicator_set_menu(ci, menu);

	g_timeout_add_seconds(2, kill_func, NULL);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_object_unref(G_OBJECT(ci));
	g_debug("Quiting");

	return 0;
}
