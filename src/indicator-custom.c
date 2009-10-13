
#include "libindicator/indicator.h"

INDICATOR_SET_VERSION
INDICATOR_SET_NAME("indicator-custom")

GtkLabel *
get_label (void)
{
	return NULL;
}

GtkImage *
get_icon (void)
{
	return GTK_IMAGE(gtk_image_new());
}

GtkMenu *
get_menu (void)
{
	GtkMenu * main_menu = GTK_MENU(gtk_menu_new());
	GtkWidget * loading_item = gtk_menu_item_new_with_label("Loading...");
	gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), loading_item);
	gtk_widget_show(GTK_WIDGET(loading_item));

	return main_menu;
}
