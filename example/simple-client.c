/*
A small piece of sample code demonstrating a very simple application
with an indicator.

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

#include "libappindicator/app-indicator.h"
#include "libdbusmenu-glib/server.h"
#include "libdbusmenu-glib/menuitem.h"

GMainLoop * mainloop = NULL;

static void
item_clicked_cb (GtkWidget *widget, gpointer data)
{
  const gchar *text = (const gchar *)data;

  g_print ("%s clicked!\n", text);
}

int
main (int argc, char ** argv)
{
        GtkWidget *menu = NULL;
        AppIndicator *ci = NULL;

        gtk_init (&argc, &argv);

        ci = app_indicator_new ("example-simple-client",
                                "indicator-messages",
                                APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	g_assert (IS_APP_INDICATOR (ci));
        g_assert (G_IS_OBJECT (ci));

	app_indicator_set_status (ci, APP_INDICATOR_STATUS_ACTIVE);
	app_indicator_set_attention_icon(ci, "indicator-messages-new");

        menu = gtk_menu_new ();
        GtkWidget *item = gtk_check_menu_item_new_with_label ("1");
        g_signal_connect (item, "activate",
                          G_CALLBACK (item_clicked_cb), "1");
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

        item = gtk_radio_menu_item_new_with_label (NULL, "2");
        g_signal_connect (item, "activate",
                          G_CALLBACK (item_clicked_cb), "2");
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

        item = gtk_menu_item_new_with_label ("3");
        g_signal_connect (item, "activate",
                          G_CALLBACK (item_clicked_cb), "3");
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

        app_indicator_set_menu (ci, GTK_MENU (menu));

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	return 0;
}
