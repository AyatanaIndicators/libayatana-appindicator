
#include <glib.h>
#include <glib-object.h>

#include <libcustomindicator/custom-indicator.h>

void
test_libcustomindicator_init (void)
{
	CustomIndicator * ci = CUSTOM_INDICATOR(g_object_new(CUSTOM_INDICATOR_TYPE, NULL));
	g_assert(ci != NULL);
	g_object_unref(G_OBJECT(ci));
}

void
test_libcustomindicator_props_suite (void)
{
	g_test_add_func ("/indicator-custom/libcustomindicator/init", test_libcustomindicator_init);


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
