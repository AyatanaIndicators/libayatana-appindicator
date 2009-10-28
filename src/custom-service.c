
#include "notification-item-client.h"
#include "custom-service-appstore.h"

static GMainLoop * mainloop = NULL;
static CustomServiceAppstore * appstore = NULL;
 
int
main (int argc, char ** argv)
{
	g_type_init();

	appstore = CUSTOM_SERVICE_APPSTORE(g_object_new(CUSTOM_SERVICE_APPSTORE_TYPE, NULL));

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	return 0;
}
