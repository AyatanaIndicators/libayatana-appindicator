/*
Tests for the libappindicator library.

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
#include <glib-object.h>

#include <app-indicator.h>

#include <libdbusmenu-glib/menuitem.h>
#include <libdbusmenu-glib/server.h>

static gboolean
allow_warnings (const gchar *log_domain, GLogLevelFlags log_level,
                const gchar *message, gpointer user_data)
{
	// By default, gtest will fail a test on even a warning message.
	// But since some of our sub-libraries are noisy (especially at-spi2),
	// only fail on critical or worse.
	return ((log_level & G_LOG_LEVEL_MASK) <= G_LOG_LEVEL_CRITICAL);
}

void
test_libappindicator_prop_signals_status_helper (AppIndicator * ci, gchar * status, gboolean * signalactivated)
{
	*signalactivated = TRUE;
	return;
}

void
test_libappindicator_prop_signals_helper (AppIndicator * ci, gboolean * signalactivated)
{
	*signalactivated = TRUE;
	return;
}

void
test_libappindicator_prop_signals (void)
{
	g_test_log_set_fatal_handler (allow_warnings, NULL);

        AppIndicator * ci = app_indicator_new ("test-app-indicator",
                                               "indicator-messages",
                                               APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	g_assert(ci != NULL);

	gboolean signaled = FALSE;
	gulong handlerid;

	handlerid = 0;
	handlerid = g_signal_connect(G_OBJECT(ci), "new-icon", G_CALLBACK(test_libappindicator_prop_signals_helper), &signaled);
	g_assert(handlerid != 0);

	handlerid = 0;
	handlerid = g_signal_connect(G_OBJECT(ci), "new-attention-icon", G_CALLBACK(test_libappindicator_prop_signals_helper), &signaled);
	g_assert(handlerid != 0);

	handlerid = 0;
	handlerid = g_signal_connect(G_OBJECT(ci), "new-status", G_CALLBACK(test_libappindicator_prop_signals_status_helper), &signaled);
	g_assert(handlerid != 0);


	signaled = FALSE;
	app_indicator_set_icon(ci, "bob");
	g_assert(signaled);

	signaled = FALSE;
	app_indicator_set_icon(ci, "bob");
	g_assert(!signaled);

	signaled = FALSE;
	app_indicator_set_icon(ci, "al");
	g_assert(signaled);


	signaled = FALSE;
	app_indicator_set_attention_icon(ci, "bob");
	g_assert(signaled);

	signaled = FALSE;
	app_indicator_set_attention_icon(ci, "bob");
	g_assert(!signaled);

	signaled = FALSE;
	app_indicator_set_attention_icon(ci, "al");
	g_assert(signaled);


	signaled = FALSE;
	app_indicator_set_status(ci, APP_INDICATOR_STATUS_PASSIVE);
	g_assert(!signaled);

	signaled = FALSE;
	app_indicator_set_status(ci, APP_INDICATOR_STATUS_ACTIVE);
	g_assert(signaled);

	signaled = FALSE;
	app_indicator_set_status(ci, APP_INDICATOR_STATUS_ACTIVE);
	g_assert(!signaled);

	signaled = FALSE;
	app_indicator_set_status(ci, APP_INDICATOR_STATUS_ATTENTION);
	g_assert(signaled);

	return;
}

void
test_libappindicator_init_set_props (void)
{
	g_test_log_set_fatal_handler (allow_warnings, NULL);

        AppIndicator * ci = app_indicator_new ("my-id",
                                               "my-name",
                                               APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	g_assert(ci != NULL);

	app_indicator_set_status(ci, APP_INDICATOR_STATUS_ACTIVE);
	app_indicator_set_attention_icon(ci, "my-attention-name");
	app_indicator_set_title(ci, "My Title");

	g_assert(!g_strcmp0("my-id", app_indicator_get_id(ci)));
	g_assert(!g_strcmp0("my-name", app_indicator_get_icon(ci)));
	g_assert(!g_strcmp0("my-attention-name", app_indicator_get_attention_icon(ci)));
	g_assert(!g_strcmp0("My Title", app_indicator_get_title(ci)));
	g_assert(app_indicator_get_status(ci) == APP_INDICATOR_STATUS_ACTIVE);
	g_assert(app_indicator_get_category(ci) == APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	g_object_unref(G_OBJECT(ci));
	return;
}

void
test_libappindicator_init_with_props (void)
{
	g_test_log_set_fatal_handler (allow_warnings, NULL);

        AppIndicator * ci = app_indicator_new ("my-id",
                                               "my-name",
                                               APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

        app_indicator_set_status (ci, APP_INDICATOR_STATUS_ACTIVE);
        app_indicator_set_attention_icon (ci, "my-attention-name");

	g_assert(ci != NULL);

	g_assert(!g_strcmp0("my-id", app_indicator_get_id(ci)));
	g_assert(!g_strcmp0("my-name", app_indicator_get_icon(ci)));
	g_assert(!g_strcmp0("my-attention-name", app_indicator_get_attention_icon(ci)));
	g_assert(app_indicator_get_status(ci) == APP_INDICATOR_STATUS_ACTIVE);
	g_assert(app_indicator_get_category(ci) == APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	g_object_unref(G_OBJECT(ci));
	return;
}

void
test_libappindicator_init (void)
{
	g_test_log_set_fatal_handler (allow_warnings, NULL);

        AppIndicator * ci = app_indicator_new ("my-id", "my-name", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	g_assert(ci != NULL);
	g_object_unref(G_OBJECT(ci));
	return;
}

void
test_libappindicator_set_label (void)
{
	g_test_log_set_fatal_handler (allow_warnings, NULL);

	AppIndicator * ci = app_indicator_new ("my-id",
	                                       "my-name",
	                                       APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	g_assert(ci != NULL);
	g_assert(app_indicator_get_label(ci) == NULL);
	g_assert(app_indicator_get_label_guide(ci) == NULL);

	/* First check all the clearing modes, this is important as
	   we're going to use them later, we need them to work. */
	app_indicator_set_label(ci, NULL, NULL);

	g_assert(app_indicator_get_label(ci) == NULL);
	g_assert(app_indicator_get_label_guide(ci) == NULL);

	app_indicator_set_label(ci, "", NULL);

	g_assert(app_indicator_get_label(ci) == NULL);
	g_assert(app_indicator_get_label_guide(ci) == NULL);

	app_indicator_set_label(ci, NULL, "");

	g_assert(app_indicator_get_label(ci) == NULL);
	g_assert(app_indicator_get_label_guide(ci) == NULL);

	app_indicator_set_label(ci, "", "");

	g_assert(app_indicator_get_label(ci) == NULL);
	g_assert(app_indicator_get_label_guide(ci) == NULL);

	app_indicator_set_label(ci, "label", "");

	g_assert(g_strcmp0(app_indicator_get_label(ci), "label") == 0);
	g_assert(app_indicator_get_label_guide(ci) == NULL);

	app_indicator_set_label(ci, NULL, NULL);

	g_assert(app_indicator_get_label(ci) == NULL);
	g_assert(app_indicator_get_label_guide(ci) == NULL);

	app_indicator_set_label(ci, "label", "guide");

	g_assert(g_strcmp0(app_indicator_get_label(ci), "label") == 0);
	g_assert(g_strcmp0(app_indicator_get_label_guide(ci), "guide") == 0);

	app_indicator_set_label(ci, "label2", "guide");

	g_assert(g_strcmp0(app_indicator_get_label(ci), "label2") == 0);
	g_assert(g_strcmp0(app_indicator_get_label_guide(ci), "guide") == 0);

	app_indicator_set_label(ci, "trick-label", "trick-guide");

	g_assert(g_strcmp0(app_indicator_get_label(ci), "trick-label") == 0);
	g_assert(g_strcmp0(app_indicator_get_label_guide(ci), "trick-guide") == 0);

	g_object_unref(G_OBJECT(ci));
	return;
}

void
test_libappindicator_set_menu (void)
{
	g_test_log_set_fatal_handler (allow_warnings, NULL);

	AppIndicator * ci = app_indicator_new ("my-id",
	                                       "my-name",
	                                       APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	g_assert(ci != NULL);

	GtkMenu * menu = GTK_MENU(gtk_menu_new());

	GtkMenuItem * item = GTK_MENU_ITEM(gtk_menu_item_new_with_label("Test Label"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(item));
	gtk_widget_show(GTK_WIDGET(item));

	app_indicator_set_menu(ci, menu);

	g_assert(app_indicator_get_menu(ci) != NULL);

	GValue serverval = {0};
	g_value_init(&serverval, DBUSMENU_TYPE_SERVER);
	g_object_get_property(G_OBJECT(ci), "dbus-menu-server", &serverval);

	DbusmenuServer * server = DBUSMENU_SERVER(g_value_get_object(&serverval));
	g_assert(server != NULL);

	GValue rootval = {0};
	g_value_init(&rootval, DBUSMENU_TYPE_MENUITEM);
	g_object_get_property(G_OBJECT(server), DBUSMENU_SERVER_PROP_ROOT_NODE, &rootval);
	DbusmenuMenuitem * root = DBUSMENU_MENUITEM(g_value_get_object(&rootval));
	g_assert(root != NULL);

	GList * children = dbusmenu_menuitem_get_children(root);
	g_assert(children != NULL);
	g_assert(g_list_length(children) == 1);

	const gchar * label = dbusmenu_menuitem_property_get(DBUSMENU_MENUITEM(children->data), DBUSMENU_MENUITEM_PROP_LABEL);
	g_assert(label != NULL);
	g_assert(g_strcmp0(label, "Test Label") == 0);

	/* Interesting, eh?  We need this because we send out events on the bus
	   but they don't come back until the idle is run.  So we need those
	   events to clear before removing the object */
	while (g_main_context_pending(NULL)) {
		g_main_context_iteration(NULL, TRUE);
	}

	g_object_unref(G_OBJECT(ci));
	return;
}

void
label_signals_cb (AppIndicator * appindicator, gchar * label, gchar * guide, gpointer user_data)
{
	gint * label_signals_count = (gint *)user_data;
	(*label_signals_count)++;
	return;
}

void
label_signals_check (void)
{
	while (g_main_context_pending(NULL)) {
		g_main_context_iteration(NULL, TRUE);
	}

	return;
}

void
test_libappindicator_label_signals (void)
{
	g_test_log_set_fatal_handler (allow_warnings, NULL);

	gint label_signals_count = 0;
	AppIndicator * ci = app_indicator_new ("my-id",
	                                       "my-name",
	                                       APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	g_assert(ci != NULL);
	g_assert(app_indicator_get_label(ci) == NULL);
	g_assert(app_indicator_get_label_guide(ci) == NULL);

	g_signal_connect(G_OBJECT(ci), APP_INDICATOR_SIGNAL_NEW_LABEL, G_CALLBACK(label_signals_cb), &label_signals_count);

	/* Shouldn't be a signal as it should be stuck in idle */
	app_indicator_set_label(ci, "label", "guide");
	g_assert(label_signals_count == 0);

	/* Should show up after idle processing */
	label_signals_check();
	g_assert(label_signals_count == 1);

	/* Shouldn't signal with no change */
	label_signals_count = 0;
	app_indicator_set_label(ci, "label", "guide");
	label_signals_check();
	g_assert(label_signals_count == 0);

	/* Change one, we should get one signal */
	app_indicator_set_label(ci, "label2", "guide");
	label_signals_check();
	g_assert(label_signals_count == 1);

	/* Change several times, one signal */
	label_signals_count = 0;
	app_indicator_set_label(ci, "label1", "guide0");
	app_indicator_set_label(ci, "label1", "guide1");
	app_indicator_set_label(ci, "label2", "guide2");
	app_indicator_set_label(ci, "label3", "guide3");
	label_signals_check();
	g_assert(label_signals_count == 1);

	/* Clear should signal too */
	label_signals_count = 0;
	app_indicator_set_label(ci, NULL, NULL);
	label_signals_check();
	g_assert(label_signals_count == 1);

	return;
}

void
test_libappindicator_desktop_menu (void)
{
	g_test_log_set_fatal_handler (allow_warnings, NULL);

	AppIndicator * ci = app_indicator_new ("my-id-desktop-menu",
	                                       "my-name",
	                                       APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	g_assert(ci != NULL);
	g_assert(app_indicator_get_label(ci) == NULL);
	g_assert(app_indicator_get_label_guide(ci) == NULL);

	app_indicator_build_menu_from_desktop(ci, SRCDIR "/test-libappindicator.desktop", "Test Program");

	GValue serverval = {0};
	g_value_init(&serverval, DBUSMENU_TYPE_SERVER);
	g_object_get_property(G_OBJECT(ci), "dbus-menu-server", &serverval);

	DbusmenuServer * server = DBUSMENU_SERVER(g_value_get_object(&serverval));
	g_assert(server != NULL);

	GValue rootval = {0};
	g_value_init(&rootval, DBUSMENU_TYPE_MENUITEM);
	g_object_get_property(G_OBJECT(server), DBUSMENU_SERVER_PROP_ROOT_NODE, &rootval);
	DbusmenuMenuitem * root = DBUSMENU_MENUITEM(g_value_get_object(&rootval));
	g_assert(root != NULL);

	GList * children = dbusmenu_menuitem_get_children(root);
	g_assert(children != NULL);
	g_assert(g_list_length(children) == 3);



	g_object_unref(G_OBJECT(ci));
	return;
}

void
test_libappindicator_desktop_menu_bad (void)
{
	g_test_log_set_fatal_handler (allow_warnings, NULL);

	AppIndicator * ci = app_indicator_new ("my-id-desktop-menu-bad",
	                                       "my-name",
	                                       APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	g_assert(ci != NULL);
	g_assert(app_indicator_get_label(ci) == NULL);
	g_assert(app_indicator_get_label_guide(ci) == NULL);

	app_indicator_build_menu_from_desktop(ci, SRCDIR "/test-libappindicator.desktop", "Not Test Program");

	GValue serverval = {0};
	g_value_init(&serverval, DBUSMENU_TYPE_SERVER);
	g_object_get_property(G_OBJECT(ci), "dbus-menu-server", &serverval);

	DbusmenuServer * server = DBUSMENU_SERVER(g_value_get_object(&serverval));
	g_assert(server != NULL);

	GValue rootval = {0};
	g_value_init(&rootval, DBUSMENU_TYPE_MENUITEM);
	g_object_get_property(G_OBJECT(server), DBUSMENU_SERVER_PROP_ROOT_NODE, &rootval);
	DbusmenuMenuitem * root = DBUSMENU_MENUITEM(g_value_get_object(&rootval));
	g_assert(root != NULL);

	GList * children = dbusmenu_menuitem_get_children(root);
	g_assert(g_list_length(children) == 0);

	g_object_unref(G_OBJECT(ci));
	return;
}

void
test_libappindicator_props_suite (void)
{
	g_test_add_func ("/indicator-application/libappindicator/init",            test_libappindicator_init);
	g_test_add_func ("/indicator-application/libappindicator/init_props",      test_libappindicator_init_with_props);
	g_test_add_func ("/indicator-application/libappindicator/init_set_props",  test_libappindicator_init_set_props);
	g_test_add_func ("/indicator-application/libappindicator/prop_signals",    test_libappindicator_prop_signals);
	g_test_add_func ("/indicator-application/libappindicator/set_label",       test_libappindicator_set_label);
	g_test_add_func ("/indicator-application/libappindicator/set_menu",        test_libappindicator_set_menu);
	g_test_add_func ("/indicator-application/libappindicator/label_signals",   test_libappindicator_label_signals);
	g_test_add_func ("/indicator-application/libappindicator/desktop_menu",    test_libappindicator_desktop_menu);
	g_test_add_func ("/indicator-application/libappindicator/desktop_menu_bad",test_libappindicator_desktop_menu_bad);

	return;
}

gint
main (gint argc, gchar * argv[])
{
	gtk_init(&argc, &argv);
	g_test_init(&argc, &argv, NULL);

	/* Test suites */
	test_libappindicator_props_suite();


	return g_test_run ();
}
