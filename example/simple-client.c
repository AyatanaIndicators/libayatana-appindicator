
#include "libcustomindicator/custom-indicator.h"
#include "libdbusmenu-glib/server.h"
#include "libdbusmenu-glib/menuitem.h"

GMainLoop * mainloop = NULL;

int
main (int argc, char ** argv)
{
	g_type_init();

	CustomIndicator * ci = CUSTOM_INDICATOR(g_object_new(CUSTOM_INDICATOR_TYPE, NULL));
	g_assert(ci != NULL);

	custom_indicator_set_id(ci, "example-simple-client");
	custom_indicator_set_category(ci, CUSTOM_INDICATOR_CATEGORY_APPLICATION_STATUS);
	custom_indicator_set_status(ci, CUSTOM_INDICATOR_STATUS_ACTIVE);
	custom_indicator_set_icon(ci, "indicator-messages");
	custom_indicator_set_attention_icon(ci, "indicator-messages-new");

	DbusmenuMenuitem * root = dbusmenu_menuitem_new();
	dbusmenu_menuitem_property_set(root, "label", "Root");

	DbusmenuServer * menuservice = dbusmenu_server_new ("/need/a/menu/path");
	dbusmenu_server_set_root(menuservice, root);

	custom_indicator_set_menu(ci, menuservice);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	return 0;
}
