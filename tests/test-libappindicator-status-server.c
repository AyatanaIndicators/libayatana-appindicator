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


#include <stdlib.h>
#include <glib.h>
#include <app-indicator.h>

static GMainLoop * mainloop = NULL;
static gboolean active = FALSE;
static guint toggle_count = 0;
static GDBusConnection * connection = NULL;

static gboolean
times_up (gpointer unused G_GNUC_UNUSED)
{
	g_dbus_connection_flush_sync (connection, NULL, NULL);
	g_clear_object (&connection);

	g_main_loop_quit (mainloop);
	return G_SOURCE_REMOVE;
}

gboolean
toggle (gpointer userdata)
{
	const AppIndicatorStatus new_status = active ? APP_INDICATOR_STATUS_ATTENTION
                                                 : APP_INDICATOR_STATUS_ACTIVE;
	app_indicator_set_status (APP_INDICATOR(userdata), new_status);
	++toggle_count;
	active = !active;

	if (toggle_count == 100) {
		g_timeout_add (100, times_up, NULL);
		return G_SOURCE_REMOVE;
	}

	return G_SOURCE_CONTINUE;
}

gint
main (gint argc, gchar * argv[])
{
	gtk_init(&argc, &argv);

	g_usleep(100000);

	connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
	g_debug("DBus Name: %s", g_dbus_connection_get_unique_name (connection));

	AppIndicator * ci = app_indicator_new ("my-id", "my-icon-name", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	app_indicator_set_attention_icon (ci, "my-attention-icon");

	GtkMenu * menu = GTK_MENU(gtk_menu_new());
	GtkMenuItem * item = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Label"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(item));

	app_indicator_set_menu(ci, menu);

	g_timeout_add(50, toggle, ci);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_object_unref(G_OBJECT(ci));

	g_debug("Quiting");

	return 0;
}
