
#include <glib.h>
#include <glib-object.h>

#include <libcustomindicator/custom-indicator.h>

void
test_libcustomindicator_prop_signals_status_helper (CustomIndicator * ci, gchar * status, gboolean * signalactivated)
{
	*signalactivated = TRUE;
	return;
}

void
test_libcustomindicator_prop_signals_helper (CustomIndicator * ci, gboolean * signalactivated)
{
	*signalactivated = TRUE;
	return;
}

void
test_libcustomindicator_prop_signals (void)
{
	CustomIndicator * ci = CUSTOM_INDICATOR(g_object_new(CUSTOM_INDICATOR_TYPE, NULL));
	g_assert(ci != NULL);

	gboolean signaled = FALSE;
	gulong handlerid;

	handlerid = 0;
	handlerid = g_signal_connect(G_OBJECT(ci), "new-icon", G_CALLBACK(test_libcustomindicator_prop_signals_helper), &signaled);
	g_assert(handlerid != 0);

	handlerid = 0;
	handlerid = g_signal_connect(G_OBJECT(ci), "new-attention-icon", G_CALLBACK(test_libcustomindicator_prop_signals_helper), &signaled);
	g_assert(handlerid != 0);

	handlerid = 0;
	handlerid = g_signal_connect(G_OBJECT(ci), "new-status", G_CALLBACK(test_libcustomindicator_prop_signals_status_helper), &signaled);
	g_assert(handlerid != 0);


	signaled = FALSE;
	custom_indicator_set_icon(ci, "bob");
	g_assert(signaled);

	signaled = FALSE;
	custom_indicator_set_icon(ci, "bob");
	g_assert(!signaled);

	signaled = FALSE;
	custom_indicator_set_icon(ci, "al");
	g_assert(signaled);


	signaled = FALSE;
	custom_indicator_set_attention_icon(ci, "bob");
	g_assert(signaled);

	signaled = FALSE;
	custom_indicator_set_attention_icon(ci, "bob");
	g_assert(!signaled);

	signaled = FALSE;
	custom_indicator_set_attention_icon(ci, "al");
	g_assert(signaled);


	signaled = FALSE;
	custom_indicator_set_status(ci, CUSTOM_INDICATOR_STATUS_PASSIVE);
	g_assert(!signaled);

	signaled = FALSE;
	custom_indicator_set_status(ci, CUSTOM_INDICATOR_STATUS_ACTIVE);
	g_assert(signaled);

	signaled = FALSE;
	custom_indicator_set_status(ci, CUSTOM_INDICATOR_STATUS_ACTIVE);
	g_assert(!signaled);

	signaled = FALSE;
	custom_indicator_set_status(ci, CUSTOM_INDICATOR_STATUS_ATTENTION);
	g_assert(signaled);

	return;
}

void
test_libcustomindicator_init_set_props (void)
{
	CustomIndicator * ci = CUSTOM_INDICATOR(g_object_new(CUSTOM_INDICATOR_TYPE, NULL));
	g_assert(ci != NULL);

	custom_indicator_set_id(ci, "my-id");
	custom_indicator_set_category(ci, CUSTOM_INDICATOR_CATEGORY_APPLICATION_STATUS);
	custom_indicator_set_status(ci, CUSTOM_INDICATOR_STATUS_ACTIVE);
	custom_indicator_set_icon(ci, "my-name");
	custom_indicator_set_attention_icon(ci, "my-attention-name");

	g_assert(!g_strcmp0("my-id", custom_indicator_get_id(ci)));
	g_assert(!g_strcmp0("my-name", custom_indicator_get_icon(ci)));
	g_assert(!g_strcmp0("my-attention-name", custom_indicator_get_attention_icon(ci)));
	g_assert(custom_indicator_get_status(ci) == CUSTOM_INDICATOR_STATUS_ACTIVE);
	g_assert(custom_indicator_get_category(ci) == CUSTOM_INDICATOR_CATEGORY_APPLICATION_STATUS);

	g_object_unref(G_OBJECT(ci));
	return;
}

void
test_libcustomindicator_init_with_props (void)
{
	CustomIndicator * ci = CUSTOM_INDICATOR(g_object_new(CUSTOM_INDICATOR_TYPE, 
	                                                     "id", "my-id",
	                                                     "category-enum", CUSTOM_INDICATOR_CATEGORY_APPLICATION_STATUS,
	                                                     "status-enum", CUSTOM_INDICATOR_STATUS_ACTIVE,
	                                                     "icon-name", "my-name",
	                                                     "attention-icon-name", "my-attention-name",
	                                                     NULL));
	g_assert(ci != NULL);

	g_assert(!g_strcmp0("my-id", custom_indicator_get_id(ci)));
	g_assert(!g_strcmp0("my-name", custom_indicator_get_icon(ci)));
	g_assert(!g_strcmp0("my-attention-name", custom_indicator_get_attention_icon(ci)));
	g_assert(custom_indicator_get_status(ci) == CUSTOM_INDICATOR_STATUS_ACTIVE);
	g_assert(custom_indicator_get_category(ci) == CUSTOM_INDICATOR_CATEGORY_APPLICATION_STATUS);

	g_object_unref(G_OBJECT(ci));
	return;
}

void
test_libcustomindicator_init (void)
{
	CustomIndicator * ci = CUSTOM_INDICATOR(g_object_new(CUSTOM_INDICATOR_TYPE, NULL));
	g_assert(ci != NULL);
	g_object_unref(G_OBJECT(ci));
	return;
}

void
test_libcustomindicator_props_suite (void)
{
	g_test_add_func ("/indicator-custom/libcustomindicator/init",        test_libcustomindicator_init);
	g_test_add_func ("/indicator-custom/libcustomindicator/init_props",  test_libcustomindicator_init_with_props);
	g_test_add_func ("/indicator-custom/libcustomindicator/init_set_props",  test_libcustomindicator_init_set_props);
	g_test_add_func ("/indicator-custom/libcustomindicator/prop_signals",  test_libcustomindicator_prop_signals);

	return;
}

gint
main (gint argc, gchar * argv[])
{
	g_type_init();
	g_test_init(&argc, &argv, NULL);

	/* Test suites */
	test_libcustomindicator_props_suite();


	return g_test_run ();
}
