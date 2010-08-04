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
        AppIndicator * ci = app_indicator_new ("my-id",
                                               "my-name",
                                               APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	g_assert(ci != NULL);

	app_indicator_set_status(ci, APP_INDICATOR_STATUS_ACTIVE);
	app_indicator_set_attention_icon(ci, "my-attention-name");

	g_assert(!g_strcmp0("my-id", app_indicator_get_id(ci)));
	g_assert(!g_strcmp0("my-name", app_indicator_get_icon(ci)));
	g_assert(!g_strcmp0("my-attention-name", app_indicator_get_attention_icon(ci)));
	g_assert(app_indicator_get_status(ci) == APP_INDICATOR_STATUS_ACTIVE);
	g_assert(app_indicator_get_category(ci) == APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	g_object_unref(G_OBJECT(ci));
	return;
}

void
test_libappindicator_init_with_props (void)
{
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
        AppIndicator * ci = app_indicator_new ("my-id", "my-name", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	g_assert(ci != NULL);
	g_object_unref(G_OBJECT(ci));
	return;
}

void
test_libappindicator_set_label (void)
{
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
test_libappindicator_props_suite (void)
{
	g_test_add_func ("/indicator-application/libappindicator/init",            test_libappindicator_init);
	g_test_add_func ("/indicator-application/libappindicator/init_props",      test_libappindicator_init_with_props);
	g_test_add_func ("/indicator-application/libappindicator/init_set_props",  test_libappindicator_init_set_props);
	g_test_add_func ("/indicator-application/libappindicator/prop_signals",    test_libappindicator_prop_signals);
	g_test_add_func ("/indicator-application/libappindicator/set_label",       test_libappindicator_set_label);
	g_test_add_func ("/indicator-application/libappindicator/label_signals",   test_libappindicator_label_signals);

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
