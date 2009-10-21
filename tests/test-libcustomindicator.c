
#include <glib.h>
#include <glib-object.h>

gint
main (gint argc, gchar * argv[])
{
	g_type_init();
	g_test_init(&argc, &argv, NULL);

	/* Test suites */

	return g_test_run ();
}
