/*
The indicator application visualization object.  It takes the information
given by the service and turns it into real-world pixels that users can
actually use.  Well, GTK does that, but this asks nicely.

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


/* G Stuff */
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

/* DBus Stuff */
#include <dbus/dbus-glib.h>
#include <libdbusmenu-gtk/menu.h>

/* Indicator Stuff */
#include <libindicator/indicator.h>
#include <libindicator/indicator-object.h>
#include <libindicator/indicator-service-manager.h>

/* Local Stuff */
#include "dbus-shared.h"
#include "application-service-client.h"
#include "application-service-marshal.h"

#define PANEL_ICON_SUFFIX  "panel"

#define INDICATOR_APPLICATION_TYPE            (indicator_application_get_type ())
#define INDICATOR_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), INDICATOR_APPLICATION_TYPE, IndicatorApplication))
#define INDICATOR_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), INDICATOR_APPLICATION_TYPE, IndicatorApplicationClass))
#define IS_INDICATOR_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), INDICATOR_APPLICATION_TYPE))
#define IS_INDICATOR_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), INDICATOR_APPLICATION_TYPE))
#define INDICATOR_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), INDICATOR_APPLICATION_TYPE, IndicatorApplicationClass))

typedef struct _IndicatorApplication      IndicatorApplication;
typedef struct _IndicatorApplicationClass IndicatorApplicationClass;

struct _IndicatorApplicationClass {
	IndicatorObjectClass parent_class;
};

struct _IndicatorApplication {
	IndicatorObject parent;
};

GType indicator_application_get_type (void);

INDICATOR_SET_VERSION
INDICATOR_SET_TYPE(INDICATOR_APPLICATION_TYPE)


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

typedef struct _IndicatorApplicationPrivate IndicatorApplicationPrivate;
struct _IndicatorApplicationPrivate {
	IndicatorServiceManager * sm;
	DBusGConnection * bus;
	DBusGProxy * service_proxy;
	GList * applications;
	GHashTable * theme_dirs;
};

typedef struct _ApplicationEntry ApplicationEntry;
struct _ApplicationEntry {
	IndicatorObjectEntry entry;
	gchar * icon_path;
};

#define INDICATOR_APPLICATION_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), INDICATOR_APPLICATION_TYPE, IndicatorApplicationPrivate))

static void indicator_application_class_init (IndicatorApplicationClass *klass);
static void indicator_application_init       (IndicatorApplication *self);
static void indicator_application_dispose    (GObject *object);
static void indicator_application_finalize   (GObject *object);
static GList * get_entries (IndicatorObject * io);
static guint get_location (IndicatorObject * io, IndicatorObjectEntry * entry);
static void connected (IndicatorServiceManager * sm, gboolean connected, IndicatorApplication * application);
static void application_added (DBusGProxy * proxy, const gchar * iconname, gint position, const gchar * dbusaddress, const gchar * dbusobject, const gchar * icon_path, IndicatorApplication * application);
static void application_removed (DBusGProxy * proxy, gint position , IndicatorApplication * application);
static void application_icon_changed (DBusGProxy * proxy, gint position, const gchar * iconname, IndicatorApplication * application);
static void get_applications (DBusGProxy *proxy, GPtrArray *OUT_applications, GError *error, gpointer userdata);
static void get_applications_helper (gpointer data, gpointer user_data);
static void theme_dir_unref(IndicatorApplication * ia, const gchar * dir);
static void theme_dir_ref(IndicatorApplication * ia, const gchar * dir);

G_DEFINE_TYPE (IndicatorApplication, indicator_application, INDICATOR_OBJECT_TYPE);

static void
indicator_application_class_init (IndicatorApplicationClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (IndicatorApplicationPrivate));

	object_class->dispose = indicator_application_dispose;
	object_class->finalize = indicator_application_finalize;

	IndicatorObjectClass * io_class = INDICATOR_OBJECT_CLASS(klass);

	io_class->get_entries = get_entries;
	io_class->get_location = get_location;

	dbus_g_object_register_marshaller(_application_service_marshal_VOID__STRING_INT_STRING_STRING_STRING,
	                                  G_TYPE_NONE,
	                                  G_TYPE_STRING,
	                                  G_TYPE_INT,
	                                  G_TYPE_STRING,
	                                  G_TYPE_STRING,
	                                  G_TYPE_STRING,
	                                  G_TYPE_INVALID);
	dbus_g_object_register_marshaller(_application_service_marshal_VOID__INT_STRING,
	                                  G_TYPE_NONE,
	                                  G_TYPE_INT,
	                                  G_TYPE_STRING,
	                                  G_TYPE_INVALID);

	return;
}

static void
indicator_application_init (IndicatorApplication *self)
{
	IndicatorApplicationPrivate * priv = INDICATOR_APPLICATION_GET_PRIVATE(self);

	/* These are built in the connection phase */
	priv->bus = NULL;
	priv->service_proxy = NULL;
	priv->theme_dirs = NULL;

	priv->sm = indicator_service_manager_new(INDICATOR_APPLICATION_DBUS_ADDR);	
	g_signal_connect(G_OBJECT(priv->sm), INDICATOR_SERVICE_MANAGER_SIGNAL_CONNECTION_CHANGE, G_CALLBACK(connected), self);

	priv->applications = NULL;

	priv->theme_dirs = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	return;
}

static void
indicator_application_dispose (GObject *object)
{
	IndicatorApplicationPrivate * priv = INDICATOR_APPLICATION_GET_PRIVATE(object);

	while (priv->applications != NULL) {
		application_removed(priv->service_proxy,
		                    0,
		                    INDICATOR_APPLICATION(object));
	}

	if (priv->sm != NULL) {
		g_object_unref(priv->sm);
		priv->sm = NULL;
	}

	if (priv->bus != NULL) {
		/* We're not incrementing the ref count on this one. */
		priv->bus = NULL;
	}

	if (priv->service_proxy != NULL) {
		g_object_unref(G_OBJECT(priv->service_proxy));
		priv->service_proxy = NULL;
	}

	if (priv->theme_dirs != NULL) {
		while (g_hash_table_size(priv->theme_dirs)) {
			GList * keys = g_hash_table_get_keys(priv->theme_dirs);
			theme_dir_unref(INDICATOR_APPLICATION(object), (gchar *)keys->data);
		}
		g_hash_table_destroy(priv->theme_dirs);
		priv->theme_dirs = NULL;
	}

	G_OBJECT_CLASS (indicator_application_parent_class)->dispose (object);
	return;
}

static void
indicator_application_finalize (GObject *object)
{

	G_OBJECT_CLASS (indicator_application_parent_class)->finalize (object);
	return;
}

void
connected (IndicatorServiceManager * sm, gboolean connected, IndicatorApplication * application)
{
	IndicatorApplicationPrivate * priv = INDICATOR_APPLICATION_GET_PRIVATE(application);
	g_debug("Connected to Application Indicator Service.");

	GError * error = NULL;

	/* Grab the session bus */
	if (priv->bus == NULL) {
		priv->bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);

		if (error != NULL) {
			g_error("Unable to get session bus: %s", error->message);
			g_error_free(error);
			return;
		}
	}

	/* Build the service proxy */
	priv->service_proxy = dbus_g_proxy_new_for_name_owner(priv->bus,
	                                                      INDICATOR_APPLICATION_DBUS_ADDR,
	                                                      INDICATOR_APPLICATION_DBUS_OBJ,
	                                                      INDICATOR_APPLICATION_DBUS_IFACE,
	                                                      &error);

	/* Set up proxy signals */
	g_debug("Setup proxy signals");
	dbus_g_proxy_add_signal(priv->service_proxy,
	                        "ApplicationAdded",
	                        G_TYPE_STRING,
	                        G_TYPE_INT,
	                        G_TYPE_STRING,
	                        G_TYPE_STRING,
	                        G_TYPE_STRING,
	                        G_TYPE_INVALID);
	dbus_g_proxy_add_signal(priv->service_proxy,
	                        "ApplicationRemoved",
	                        G_TYPE_INT,
	                        G_TYPE_INVALID);
	dbus_g_proxy_add_signal(priv->service_proxy,
	                        "ApplicationIconChanged",
	                        G_TYPE_INT,
	                        G_TYPE_STRING,
	                        G_TYPE_INVALID);

	/* Connect to them */
	g_debug("Connect to them.");
	dbus_g_proxy_connect_signal(priv->service_proxy,
	                            "ApplicationAdded",
	                            G_CALLBACK(application_added),
	                            application,
	                            NULL /* Disconnection Signal */);
	dbus_g_proxy_connect_signal(priv->service_proxy,
	                            "ApplicationRemoved",
	                            G_CALLBACK(application_removed),
	                            application,
	                            NULL /* Disconnection Signal */);
	dbus_g_proxy_connect_signal(priv->service_proxy,
	                            "ApplicationIconChanged",
	                            G_CALLBACK(application_icon_changed),
	                            application,
	                            NULL /* Disconnection Signal */);

	/* Query it for existing applications */
	g_debug("Request current apps");
	org_ayatana_indicator_application_service_get_applications_async(priv->service_proxy,
	                                                                 get_applications,
	                                                                 application);

	return;
}

/* Goes through the list of applications that we're maintaining and
   pulls out the IndicatorObjectEntry and returns that in a list
   for the caller. */
static GList *
get_entries (IndicatorObject * io)
{
	g_return_val_if_fail(IS_INDICATOR_APPLICATION(io), NULL);

	IndicatorApplicationPrivate * priv = INDICATOR_APPLICATION_GET_PRIVATE(io);
	GList * retval = NULL;
	GList * apppointer = NULL;

	for (apppointer = priv->applications; apppointer != NULL; apppointer = g_list_next(apppointer)) {
		IndicatorObjectEntry * entry = &(((ApplicationEntry *)apppointer->data)->entry);
		retval = g_list_prepend(retval, entry);
	}

	if (retval != NULL) {
		retval = g_list_reverse(retval);
	}

	return retval;
}

/* Finds the location of a specific entry */
static guint 
get_location (IndicatorObject * io, IndicatorObjectEntry * entry)
{
	g_return_val_if_fail(IS_INDICATOR_APPLICATION(io), 0);
	IndicatorApplicationPrivate * priv = INDICATOR_APPLICATION_GET_PRIVATE(io);
	return g_list_index(priv->applications, entry);
}

/* Here we respond to new applications by building up the
   ApplicationEntry and signaling the indicator host that
   we've got a new indicator. */
static void
application_added (DBusGProxy * proxy, const gchar * iconname, gint position, const gchar * dbusaddress, const gchar * dbusobject, const gchar * icon_path, IndicatorApplication * application)
{
	g_debug("Building new application entry: %s  with icon: %s", dbusaddress, iconname);
	IndicatorApplicationPrivate * priv = INDICATOR_APPLICATION_GET_PRIVATE(application);
	ApplicationEntry * app = g_new(ApplicationEntry, 1);

	app->icon_path = NULL;
	if (icon_path != NULL && icon_path[0] != '\0') {
		app->icon_path = g_strdup(icon_path);
		theme_dir_ref(application, icon_path);
	}

	/* We make a long name using the suffix, and if that
	   icon is available we want to use it.  Otherwise we'll
	   just use the name we were given. */
	gchar * longname = g_strdup_printf("%s-%s", iconname, PANEL_ICON_SUFFIX);
	if (gtk_icon_theme_has_icon(gtk_icon_theme_get_default(), longname)) {
		app->entry.image = GTK_IMAGE(gtk_image_new_from_icon_name(longname, GTK_ICON_SIZE_MENU));
	} else {
		app->entry.image = GTK_IMAGE(gtk_image_new_from_icon_name(iconname, GTK_ICON_SIZE_MENU));
	}
	g_free(longname);

	app->entry.label = NULL;
	app->entry.menu = GTK_MENU(dbusmenu_gtkmenu_new((gchar *)dbusaddress, (gchar *)dbusobject));

	/* Keep copies of these for ourself, just in case. */
	g_object_ref(app->entry.image);
	g_object_ref(app->entry.menu);

	gtk_widget_show(GTK_WIDGET(app->entry.image));

	priv->applications = g_list_insert(priv->applications, app, position);

	/* TODO: Need to deal with position here somehow */
	g_signal_emit(G_OBJECT(application), INDICATOR_OBJECT_SIGNAL_ENTRY_ADDED_ID, 0, &(app->entry), TRUE);
	return;
}

/* This removes the application from the list and free's all
   of the memory associated with it. */
static void
application_removed (DBusGProxy * proxy, gint position, IndicatorApplication * application)
{
	IndicatorApplicationPrivate * priv = INDICATOR_APPLICATION_GET_PRIVATE(application);
	ApplicationEntry * app = (ApplicationEntry *)g_list_nth_data(priv->applications, position);

	if (app == NULL) {
		g_warning("Unable to find application at position: %d", position);
		return;
	}

	priv->applications = g_list_remove(priv->applications, app);
	g_signal_emit(G_OBJECT(application), INDICATOR_OBJECT_SIGNAL_ENTRY_REMOVED_ID, 0, &(app->entry), TRUE);

	if (app->icon_path != NULL) {
		theme_dir_unref(application, app->icon_path);
		g_free(app->icon_path);
	}
	if (app->entry.image != NULL) {
		g_object_unref(G_OBJECT(app->entry.image));
	}
	if (app->entry.label != NULL) {
		g_warning("Odd, an application indicator with a label?");
		g_object_unref(G_OBJECT(app->entry.label));
	}
	if (app->entry.menu != NULL) {
		g_object_unref(G_OBJECT(app->entry.menu));
	}
	g_free(app);

	return;
}

/* The callback for the signal that the icon for an application
   has changed. */
static void
application_icon_changed (DBusGProxy * proxy, gint position, const gchar * iconname, IndicatorApplication * application)
{
	IndicatorApplicationPrivate * priv = INDICATOR_APPLICATION_GET_PRIVATE(application);
	ApplicationEntry * app = (ApplicationEntry *)g_list_nth_data(priv->applications, position);

	if (app == NULL) {
		g_warning("Unable to find application at position: %d", position);
		return;
	}

	/* We make a long name using the suffix, and if that
	   icon is available we want to use it.  Otherwise we'll
	   just use the name we were given. */
	gchar * longname = g_strdup_printf("%s-%s", iconname, PANEL_ICON_SUFFIX);
	if (gtk_icon_theme_has_icon(gtk_icon_theme_get_default(), longname)) {
		gtk_image_set_from_icon_name(app->entry.image, longname, GTK_ICON_SIZE_MENU);
	} else {
		gtk_image_set_from_icon_name(app->entry.image, iconname, GTK_ICON_SIZE_MENU);
	}
	g_free(longname);

	return;
}

/* This repsonds to the list of applications that the service
   has and calls application_added on each one of them. */
static void
get_applications (DBusGProxy *proxy, GPtrArray *OUT_applications, GError *error, gpointer userdata)
{
	if (error != NULL) {
		g_warning("Unable to get application list: %s", error->message);
		return;
	}
	g_ptr_array_foreach(OUT_applications, get_applications_helper, userdata);

	return;
}

/* A little helper that takes apart the DBus structure and calls
   application_added on every entry in the list. */
static void
get_applications_helper (gpointer data, gpointer user_data)
{
	GValueArray * array = (GValueArray *)data;

	g_return_if_fail(array->n_values == 5);

	const gchar * icon_name = g_value_get_string(g_value_array_get_nth(array, 0));
	gint position = g_value_get_int(g_value_array_get_nth(array, 1));
	const gchar * dbus_address = g_value_get_string(g_value_array_get_nth(array, 2));
	const gchar * dbus_object = g_value_get_boxed(g_value_array_get_nth(array, 3));
	const gchar * icon_path = g_value_get_string(g_value_array_get_nth(array, 4));

	return application_added(NULL, icon_name, position, dbus_address, dbus_object, icon_path, user_data);
}

/* Refs a theme directory, and it may add it to the search
   path */
static void
theme_dir_unref(IndicatorApplication * ia, const gchar * dir)
{
	IndicatorApplicationPrivate * priv = INDICATOR_APPLICATION_GET_PRIVATE(ia);

	/* Grab the count for this dir */
	int count = GPOINTER_TO_INT(g_hash_table_lookup(priv->theme_dirs, dir));

	/* Is this a simple deprecation, if so, we can just lower the
	   number and move on. */
	if (count > 1) {
		count--;
		g_hash_table_insert(priv->theme_dirs, g_strdup(dir), GINT_TO_POINTER(count));
		return;
	}

	/* Try to remove it from the hash table, this makes sure
	   that it existed */
	if (!g_hash_table_remove(priv->theme_dirs, dir)) {
		g_warning("Unref'd a directory that wasn't in the theme dir hash table.");
		return;
	}

	GtkIconTheme * icon_theme = gtk_icon_theme_get_default();
	gchar ** paths;
	gint path_count;

	gtk_icon_theme_get_search_path(icon_theme, &paths, &path_count);

	gint i;
	gboolean found = FALSE;
	for (i = 0; i < path_count; i++) {
		if (found) {
			/* If we've already found the right entry */
			paths[i - 1] = paths[i];
		} else {
			/* We're still looking, is this the one? */
			if (!g_strcmp0(paths[i], dir)) {
				found = TRUE;
				/* We're freeing this here as it won't be captured by the
				   g_strfreev() below as it's out of the array. */
				g_free(paths[i]);
			}
		}
	}
	
	/* If we found one we need to reset the path to
	   accomidate the changes */
	if (found) {
		paths[path_count - 1] = NULL; /* Clear the last one */
		gtk_icon_theme_set_search_path(icon_theme, (const gchar **)paths, path_count - 1);
	}

	g_strfreev(paths);

	return;
}

/* Unrefs a theme directory.  This may involve removing it from
   the search path. */
static void
theme_dir_ref(IndicatorApplication * ia, const gchar * dir)
{
	IndicatorApplicationPrivate * priv = INDICATOR_APPLICATION_GET_PRIVATE(ia);

	int count = 0;
	if ((count = GPOINTER_TO_INT(g_hash_table_lookup(priv->theme_dirs, dir))) != 0) {
		/* It exists so what we need to do is increase the ref
		   count of this dir. */
		count++;
	} else {
		/* It doesn't exist, so we need to add it to the table
		   and to the search path. */
		gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), dir);
		g_debug("\tAppending search path: %s", dir);
		count = 1;
	}

	g_hash_table_insert(priv->theme_dirs, g_strdup(dir), GINT_TO_POINTER(count));

	return;
}

