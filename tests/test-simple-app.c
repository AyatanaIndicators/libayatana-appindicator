
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <glib.h>
#include <libcustomindicator/custom-indicator.h>

static GMainLoop * mainloop = NULL;

int
main (int argc, char ** argv)
{
	g_type_init();

	DbusmenuServer * dms = dbusmenu_server_new("/menu");
	DbusmenuMenuitem * dmi = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(dmi, "label", "Bob");

	CustomIndicator * ci = CUSTOM_INDICATOR(g_object_new(CUSTOM_INDICATOR_TYPE, 
	                                                     "id", "test-application",
	                                                     "status-enum", CUSTOM_INDICATOR_STATUS_ACTIVE,
	                                                     "icon-name", "system-shutdown",
	                                                     "menu-object", dms,
	                                                     NULL));

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_object_unref(G_OBJECT(ci));
	g_debug("Quiting");

	return 0;
}
