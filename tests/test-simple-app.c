
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <glib.h>
#include <libappindicator/app-indicator.h>

static GMainLoop * mainloop = NULL;

int
main (int argc, char ** argv)
{
	g_type_init();

	DbusmenuServer * dms = dbusmenu_server_new("/menu");
	DbusmenuMenuitem * dmi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(dmi, "label", "Bob");

	AppIndicator * ci = APP_INDICATOR(g_object_new(APP_INDICATOR_TYPE, 
	                                               "id", "test-application",
	                                               "status-enum", APP_INDICATOR_STATUS_ACTIVE,
	                                               "icon-name", "system-shutdown",
	                                               "menu-object", dms,
	                                               NULL));

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_object_unref(G_OBJECT(ci));
	g_debug("Quiting");

	return 0;
}
