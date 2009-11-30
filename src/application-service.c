
#include "libindicator/indicator-service.h"
#include "notification-item-client.h"
#include "application-service-appstore.h"
#include "application-service-watcher.h"
#include "dbus-shared.h"

/* The base main loop */
static GMainLoop * mainloop = NULL;
/* Where the application registry lives */
static CustomServiceAppstore * appstore = NULL;
/* Interface for applications */
static CustomServiceWatcher * watcher = NULL;
/* The service management interface */
static IndicatorService * service = NULL;

/* Recieves the disonnection signal from the service
   object and closes the mainloop. */
static void
service_disconnected (IndicatorService * service, gpointer data)
{
	g_debug("Service disconnected");
	if (mainloop != NULL) {
		g_main_loop_quit(mainloop);
	}
	return;
}
 
/* Builds up the core objects and puts us spinning into
   a main loop. */
int
main (int argc, char ** argv)
{
	g_type_init();

	/* Bring us up as a basic indicator service */
	service = indicator_service_new(INDICATOR_CUSTOM_DBUS_ADDR);
	g_signal_connect(G_OBJECT(service), "disconnected", G_CALLBACK(service_disconnected), NULL);

	/* Building our app store */
	appstore = CUSTOM_SERVICE_APPSTORE(g_object_new(CUSTOM_SERVICE_APPSTORE_TYPE, NULL));

	/* Adding a watcher for the Apps coming up */
	watcher = custom_service_watcher_new(appstore);

	/* Building and executing our main loop */
	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	/* Unref'ing all the objects */
	g_object_unref(G_OBJECT(watcher));
	g_object_unref(G_OBJECT(appstore));
	g_object_unref(G_OBJECT(service));

	return 0;
}
