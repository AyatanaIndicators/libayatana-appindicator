/*
An object to represent the application as an application indicator
in the system panel.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>
    Cody Russell <cody.russell@canonical.com>

This program is free software: you can redistribute it and/or modify it
under the terms of either or both of the following licenses:

1) the GNU Lesser General Public License version 3, as published by the
   Free Software Foundation; and/or
2) the GNU Lesser General Public License version 2.1, as published by
   the Free Software Foundation.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranties of
MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the applicable version of the GNU Lesser General Public
License for more details.

You should have received a copy of both the GNU Lesser General Public
License version 3 and version 2.1 along with this program.  If not, see
<http://www.gnu.org/licenses/>
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dbus/dbus-glib.h>
#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-gtk/client.h>

#include "app-indicator.h"
#include "app-indicator-enum-types.h"
#include "application-service-marshal.h"

#include "notification-item-server.h"
#include "notification-watcher-client.h"

#include "dbus-shared.h"
#include "generate-id.h"

#define PANEL_ICON_SUFFIX  "panel"

/**
	AppIndicatorPrivate:

	All of the private data in an instance of a
	application indicator.
*/
/*  Private Fields
	@id: The ID of the indicator.  Maps to AppIndicator:id.
	@category: Which category the indicator is.  Maps to AppIndicator:category.
	@status: The status of the indicator.  Maps to AppIndicator:status.
	@icon_name: The name of the icon to use.  Maps to AppIndicator:icon-name.
	@attention_icon_name: The name of the attention icon to use.  Maps to AppIndicator:attention-icon-name.
	@menu: The menu for this indicator.  Maps to AppIndicator:menu
	@watcher_proxy: The proxy connection to the watcher we're connected to.  If we're not connected to one this will be %NULL.
*/
struct _AppIndicatorPrivate {
	/*< Private >*/
	/* Properties */
	gchar                *id;
	gchar                *clean_id;
	AppIndicatorCategory  category;
	AppIndicatorStatus    status;
	gchar                *icon_name;
	gchar                *attention_icon_name;
	gchar                *icon_theme_path;
	DbusmenuServer       *menuservice;
	GtkWidget            *menu;
	guint32               ordering_index;
	gchar *               label;
	gchar *               label_guide;
	guint                 label_change_idle;

	GtkStatusIcon *       status_icon;
	gint                  fallback_timer;

	/* Fun stuff */
	DBusGProxy           *watcher_proxy;
	DBusGConnection      *connection;
	DBusGProxy *          dbus_proxy;
};

/* Signals Stuff */
enum {
	NEW_ICON,
	NEW_ATTENTION_ICON,
	NEW_STATUS,
	NEW_LABEL,
	X_NEW_LABEL,
	CONNECTION_CHANGED,
    NEW_ICON_THEME_PATH,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* Enum for the properties so that they can be quickly
   found and looked up. */
enum {
	PROP_0,
	PROP_ID,
	PROP_CATEGORY,
	PROP_STATUS,
	PROP_ICON_NAME,
	PROP_ATTENTION_ICON_NAME,
	PROP_ICON_THEME_PATH,
	PROP_MENU,
	PROP_CONNECTED,
	PROP_LABEL,
	PROP_LABEL_GUIDE,
	PROP_X_LABEL,
	PROP_X_LABEL_GUIDE,
	PROP_ORDERING_INDEX,
	PROP_X_ORDERING_INDEX
};

/* The strings so that they can be slowly looked up. */
#define PROP_ID_S                    "id"
#define PROP_CATEGORY_S              "category"
#define PROP_STATUS_S                "status"
#define PROP_ICON_NAME_S             "icon-name"
#define PROP_ATTENTION_ICON_NAME_S   "attention-icon-name"
#define PROP_ICON_THEME_PATH_S       "icon-theme-path"
#define PROP_MENU_S                  "menu"
#define PROP_CONNECTED_S             "connected"
#define PROP_LABEL_S                 "label"
#define PROP_LABEL_GUIDE_S           "label-guide"
#define PROP_X_LABEL_S               ("x-ayatana-" PROP_LABEL_S)
#define PROP_X_LABEL_GUIDE_S         ("x-ayatana-" PROP_LABEL_GUIDE_S)
#define PROP_ORDERING_INDEX_S        "ordering-index"
#define PROP_X_ORDERING_INDEX_S      ("x-ayatana-" PROP_ORDERING_INDEX_S)

/* Private macro, shhhh! */
#define APP_INDICATOR_GET_PRIVATE(o) \
                             (G_TYPE_INSTANCE_GET_PRIVATE ((o), APP_INDICATOR_TYPE, AppIndicatorPrivate))

/* Signal wrapper */
#define APP_INDICATOR_SIGNAL_X_NEW_LABEL ("x-ayatana-" APP_INDICATOR_SIGNAL_NEW_LABEL)

/* Default Path */
#define DEFAULT_ITEM_PATH   "/org/ayatana/NotificationItem"

/* More constants */
#define DEFAULT_FALLBACK_TIMER  100 /* in milliseconds */

/* Boiler plate */
static void app_indicator_class_init (AppIndicatorClass *klass);
static void app_indicator_init       (AppIndicator *self);
static void app_indicator_dispose    (GObject *object);
static void app_indicator_finalize   (GObject *object);
/* Property functions */
static void app_indicator_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void app_indicator_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);
/* Other stuff */
static void signal_label_change (AppIndicator * self);
static void check_connect (AppIndicator * self);
static void register_service_cb (DBusGProxy * proxy, GError * error, gpointer data);
static void start_fallback_timer (AppIndicator * self, gboolean disable_timeout);
static gboolean fallback_timer_expire (gpointer data);
static GtkStatusIcon * fallback (AppIndicator * self);
static void status_icon_status_wrapper (AppIndicator * self, const gchar * status, gpointer data);
static void status_icon_changes (AppIndicator * self, gpointer data);
static void status_icon_activate (GtkStatusIcon * icon, gpointer data);
static void unfallback (AppIndicator * self, GtkStatusIcon * status_icon);
static gchar * append_panel_icon_suffix (const gchar * icon_name);
static void watcher_proxy_destroyed (GObject * object, gpointer data);
static void client_menu_changed (GtkWidget *widget, GtkWidget *child, AppIndicator *indicator);
static void submenu_changed (GtkWidget *widget, GtkWidget *child, gpointer data);

static void theme_changed_cb (GtkIconTheme * theme, gpointer user_data);

/* GObject type */
G_DEFINE_TYPE (AppIndicator, app_indicator, G_TYPE_OBJECT);

static void
app_indicator_class_init (AppIndicatorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (AppIndicatorPrivate));

	/* Clean up */
	object_class->dispose = app_indicator_dispose;
	object_class->finalize = app_indicator_finalize;

	/* Property funcs */
	object_class->set_property = app_indicator_set_property;
	object_class->get_property = app_indicator_get_property;

	/* Our own funcs */
	klass->fallback = fallback;
	klass->unfallback = unfallback;

	/* Properties */

	/**
		AppIndicator:id:
		
		The ID for this indicator, which should be unique, but used consistently
		by this program and its indicator.
	*/
	g_object_class_install_property (object_class,
                                         PROP_ID,
                                         g_param_spec_string(PROP_ID_S,
                                                             "The ID for this indicator",
                                                             "An ID that should be unique, but used consistently by this program and its indicator.",
                                                             NULL,
                                                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY));

	/**
		AppIndicator:category:
		
		The type of indicator that this represents.  Please don't use 'Other'. 
		Defaults to 'ApplicationStatus'.
	*/
	g_object_class_install_property (object_class,
                                         PROP_CATEGORY,
                                         g_param_spec_string (PROP_CATEGORY_S,
                                                              "Indicator Category",
                                                              "The type of indicator that this represents.  Please don't use 'other'. Defaults to 'ApplicationStatus'.",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY));

	/**
		AppIndicator:status:
		
		Whether the indicator is shown or requests attention. Defaults to
		'Passive'.
	*/
	g_object_class_install_property (object_class,
                                         PROP_STATUS,
                                         g_param_spec_string (PROP_STATUS_S,
                                                              "Indicator Status",
                                                              "Whether the indicator is shown or requests attention. Defaults to 'Passive'.",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/**
		AppIndicator:icon-name:
		
		The name of the regular icon that is shown for the indicator.
	*/
	g_object_class_install_property(object_class,
                                    PROP_ICON_NAME,
	                                g_param_spec_string (PROP_ICON_NAME_S,
                                                             "An icon for the indicator",
                                                             "The default icon that is shown for the indicator.",
                                                             NULL,
                                                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT));

	/**
		AppIndicator:attention-icon-name:
		
		If the indicator sets it's status to %APP_INDICATOR_STATUS_ATTENTION
		then this icon is shown.
	*/
	g_object_class_install_property (object_class,
                                         PROP_ATTENTION_ICON_NAME,
                                         g_param_spec_string (PROP_ATTENTION_ICON_NAME_S,
                                                              "An icon to show when the indicator request attention.",
                                                              "If the indicator sets it's status to 'attention' then this icon is shown.",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	/**
		AppIndicator:icon-theme-path:
		
		An additional place to look for icon names that may be installed by the
		application.
	*/
	g_object_class_install_property(object_class,
	                                PROP_ICON_THEME_PATH,
	                                g_param_spec_string (PROP_ICON_THEME_PATH_S,
                                                             "An additional path for custom icons.",
                                                             "An additional place to look for icon names that may be installed by the application.",
                                                             NULL,
                                                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT));
	
	/**
		AppIndicator:menu:
		
		A method for getting the menu path as a string for DBus.
	*/
    g_object_class_install_property(object_class,
                                        PROP_MENU,
                                        g_param_spec_boxed (PROP_MENU_S,
                                                             "The object path of the menu on DBus.",
                                                             "A method for getting the menu path as a string for DBus.",
                                                             DBUS_TYPE_G_OBJECT_PATH,
                                                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	/**
		AppIndicator:connected:
		
		Pretty simple, %TRUE if we have a reasonable expectation of being 
		displayed through this object. You should hide your TrayIcon if so.
	*/
	g_object_class_install_property (object_class,
                                         PROP_CONNECTED,
                                         g_param_spec_boolean (PROP_CONNECTED_S,
                                                               "Whether we're conneced to a watcher",
                                                               "Pretty simple, true if we have a reasonable expectation of being displayed through this object.  You should hide your TrayIcon if so.",
                                                               FALSE,
                                                               G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
	/**
		AppIndicator:label:
		
		A label that can be shown next to the string in the application
		indicator.  The label will not be shown unless there is an icon
		as well.  The label is useful for numerical and other frequently
		updated information.  In general, it shouldn't be shown unless a
		user requests it as it can take up a significant amount of space
		on the user's panel.  This may not be shown in all visualizations.
	*/
	g_object_class_install_property(object_class,
	                                PROP_LABEL,
	                                g_param_spec_string (PROP_LABEL_S,
	                                                     "A label next to the icon",
	                                                     "A label to provide dynamic information.",
	                                                     NULL,
	                                                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	/**
		AppIndicator:label-guide:
		
		An optional string to provide guidance to the panel on how big
		the #AppIndicator:label string could get.  If this is set correctly
		then the panel should never 'jiggle' as the string adjusts through
		out the range of options.  For instance, if you were providing a
		percentage like "54% thrust" in #AppIndicator:label you'd want to
		set this string to "100% thrust" to ensure space when Scotty can
		get you enough power.
	*/
	g_object_class_install_property(object_class,
	                                PROP_LABEL_GUIDE,
	                                g_param_spec_string (PROP_LABEL_GUIDE_S,
	                                                     "A string to size the space available for the label.",
	                                                     "To ensure that the label does not cause the panel to 'jiggle' this string should provide information on how much space it could take.",
	                                                     NULL,
	                                                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	/**
		AppIndicator:ordering-index:

		The ordering index is an odd parameter, and if you think you don't need
		it you're probably right.  In general, the application indicator try
		to place the applications in a recreatable place taking into account
		which category they're in to try and group them.  But, there are some
		cases where you'd want to ensure indicators are next to each other.
		To do that you can override the generated ordering index and replace it
		with a new one.  Again, you probably don't want to be doing this, but
		in case you do, this is the way.
	*/
	g_object_class_install_property(object_class,
	                                PROP_ORDERING_INDEX,
	                                g_param_spec_uint (PROP_ORDERING_INDEX_S,
	                                                   "The location that this app indicator should be in the list.",
	                                                   "A way to override the default ordering of the applications by providing a very specific idea of where this entry should be placed.",
	                                                   0, G_MAXUINT32, 0,
	                                                   G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	/**
		AppIndicator:x-ayatana-ordering-index:

		A wrapper for #AppIndicator:ordering-index so that it can match the
		dbus interface currently.  It will hopefully be retired, please don't
		use it anywhere.
	*/
	g_object_class_install_property(object_class,
	                                PROP_X_ORDERING_INDEX,
	                                g_param_spec_uint (PROP_X_ORDERING_INDEX_S,
	                                                   "A wrapper, please don't use.",
	                                                   "A wrapper, please don't use.",
	                                                   0, G_MAXUINT32, 0,
	                                                   G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/**
		AppIndicator:x-ayatana-label:

		Wrapper for #AppIndicator:label.  Please use that in all of your
		code.
	*/
	g_object_class_install_property(object_class,
	                                PROP_X_LABEL,
	                                g_param_spec_string (PROP_X_LABEL_S,
	                                                     "A wrapper, please don't use.",
	                                                     "A wrapper, please don't use.",
	                                                     NULL,
	                                                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	/**
		AppIndicator:x-ayatana-label-guide:

		Wrapper for #AppIndicator:label-guide.  Please use that in all of your
		code.
	*/
	g_object_class_install_property(object_class,
	                                PROP_X_LABEL_GUIDE,
	                                g_param_spec_string (PROP_X_LABEL_GUIDE_S,
	                                                     "A wrapper, please don't use.",
	                                                     "A wrapper, please don't use.",
	                                                     NULL,
	                                                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/* Signals */

	/**
		AppIndicator::new-icon:
		@arg0: The #AppIndicator object

		Emitted when #AppIndicator:icon-name is changed
	*/
	signals[NEW_ICON] = g_signal_new (APP_INDICATOR_SIGNAL_NEW_ICON,
	                                  G_TYPE_FROM_CLASS(klass),
	                                  G_SIGNAL_RUN_LAST,
	                                  G_STRUCT_OFFSET (AppIndicatorClass, new_icon),
	                                  NULL, NULL,
	                                  g_cclosure_marshal_VOID__VOID,
	                                  G_TYPE_NONE, 0, G_TYPE_NONE);

	/**
		AppIndicator::new-attention-icon:
		@arg0: The #AppIndicator object

		Emitted when #AppIndicator:attention-icon-name is changed
	*/
	signals[NEW_ATTENTION_ICON] = g_signal_new (APP_INDICATOR_SIGNAL_NEW_ATTENTION_ICON,
	                                            G_TYPE_FROM_CLASS(klass),
	                                            G_SIGNAL_RUN_LAST,
	                                            G_STRUCT_OFFSET (AppIndicatorClass, new_attention_icon),
	                                            NULL, NULL,
	                                            g_cclosure_marshal_VOID__VOID,
	                                            G_TYPE_NONE, 0, G_TYPE_NONE);

	/**
		AppIndicator::new-status:
		@arg0: The #AppIndicator object
		@arg1: The string value of the #AppIndicatorStatus enum.

		Emitted when #AppIndicator:status is changed
	*/
	signals[NEW_STATUS] = g_signal_new (APP_INDICATOR_SIGNAL_NEW_STATUS,
	                                    G_TYPE_FROM_CLASS(klass),
	                                    G_SIGNAL_RUN_LAST,
	                                    G_STRUCT_OFFSET (AppIndicatorClass, new_status),
	                                    NULL, NULL,
	                                    g_cclosure_marshal_VOID__STRING,
	                                    G_TYPE_NONE, 1,
                                            G_TYPE_STRING);

	/**
		AppIndicator::new-label:
		@arg0: The #AppIndicator object
		@arg1: The string for the label
		@arg1: The string for the guide

		Emitted when either #AppIndicator:label or #AppIndicator:label-guide are
		changed.
	*/
	signals[NEW_LABEL] = g_signal_new (APP_INDICATOR_SIGNAL_NEW_LABEL,
	                                    G_TYPE_FROM_CLASS(klass),
	                                    G_SIGNAL_RUN_LAST,
	                                    G_STRUCT_OFFSET (AppIndicatorClass, new_label),
	                                    NULL, NULL,
	                                    _application_service_marshal_VOID__STRING_STRING,
	                                    G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

	/**
		AppIndicator::x-ayatana-new-label:
		@arg0: The #AppIndicator object
		@arg1: The string for the label
		@arg1: The string for the guide

		Wrapper for #AppIndicator::new-label, please don't use this signal
		use the other one.
	*/
	signals[X_NEW_LABEL] = g_signal_new (APP_INDICATOR_SIGNAL_X_NEW_LABEL,
	                                    G_TYPE_FROM_CLASS(klass),
	                                    G_SIGNAL_RUN_LAST,
	                                    G_STRUCT_OFFSET (AppIndicatorClass, new_label),
	                                    NULL, NULL,
	                                    _application_service_marshal_VOID__STRING_STRING,
	                                    G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

	/**
		AppIndicator::connection-changed:
		@arg0: The #AppIndicator object
		@arg1: Whether we're connected or not

		Signaled when we connect to a watcher, or when it drops away.
	*/
	signals[CONNECTION_CHANGED] = g_signal_new (APP_INDICATOR_SIGNAL_CONNECTION_CHANGED,
	                                            G_TYPE_FROM_CLASS(klass),
	                                            G_SIGNAL_RUN_LAST,
	                                            G_STRUCT_OFFSET (AppIndicatorClass, connection_changed),
	                                            NULL, NULL,
	                                            g_cclosure_marshal_VOID__BOOLEAN,
	                                            G_TYPE_NONE, 1, G_TYPE_BOOLEAN, G_TYPE_NONE);

	/**
		AppIndicator::new-icon-theme-path:
		@arg0: The #AppIndicator object

		Signaled when there is a new icon set for the
		object.
	*/
	signals[NEW_ICON_THEME_PATH] = g_signal_new (APP_INDICATOR_SIGNAL_NEW_ICON_THEME_PATH,
	                                  G_TYPE_FROM_CLASS(klass),
	                                  G_SIGNAL_RUN_LAST,
	                                  G_STRUCT_OFFSET (AppIndicatorClass, new_icon_theme_path),
	                                  NULL, NULL,
	                                  g_cclosure_marshal_VOID__STRING,
	                                  G_TYPE_NONE, 1, G_TYPE_STRING);

	/* Initialize the object as a DBus type */
	dbus_g_object_type_install_info(APP_INDICATOR_TYPE,
	                                &dbus_glib__notification_item_server_object_info);

	return;
}

static void
app_indicator_init (AppIndicator *self)
{
	AppIndicatorPrivate * priv = APP_INDICATOR_GET_PRIVATE(self);

	priv->id = NULL;
	priv->clean_id = NULL;
	priv->category = APP_INDICATOR_CATEGORY_OTHER;
	priv->status = APP_INDICATOR_STATUS_PASSIVE;
	priv->icon_name = NULL;
	priv->attention_icon_name = NULL;
	priv->icon_theme_path = NULL;
	priv->menu = NULL;
	priv->menuservice = NULL;
	priv->ordering_index = 0;
	priv->label = NULL;
	priv->label_guide = NULL;
	priv->label_change_idle = 0;

	priv->watcher_proxy = NULL;
	priv->connection = NULL;
	priv->dbus_proxy = NULL;

	priv->status_icon = NULL;
	priv->fallback_timer = 0;

	/* Put the object on DBus */
	GError * error = NULL;
	priv->connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_error("Unable to connect to the session bus when creating application indicator: %s", error->message);
		g_error_free(error);
		return;
	}
	dbus_g_connection_ref(priv->connection);

	g_signal_connect(G_OBJECT(gtk_icon_theme_get_default()),
		"changed", G_CALLBACK(theme_changed_cb), self);

	self->priv = priv;

	return;
}

/* Free all objects, make sure that all the dbus
   signals are sent out before we shut this down. */
static void
app_indicator_dispose (GObject *object)
{
	AppIndicator *self = APP_INDICATOR (object);
	AppIndicatorPrivate *priv = self->priv;

	if (priv->status != APP_INDICATOR_STATUS_PASSIVE) {
		app_indicator_set_status(self, APP_INDICATOR_STATUS_PASSIVE);
	}

	if (priv->status_icon != NULL) {
		AppIndicatorClass * class = APP_INDICATOR_GET_CLASS(object);
		if (class->unfallback != NULL) {
			class->unfallback(self, priv->status_icon);
		}
		priv->status_icon = NULL;
	}

	if (priv->fallback_timer != 0) {
		g_source_remove(priv->fallback_timer);
		priv->fallback_timer = 0;
	}

	if (priv->label_change_idle != 0) {
		g_source_remove(priv->label_change_idle);
		priv->label_change_idle = 0;
	}

	if (priv->menu != NULL) {
                g_signal_handlers_disconnect_by_func (G_OBJECT (priv->menu),
                                                      client_menu_changed,
                                                      self);
                g_object_unref(G_OBJECT(priv->menu));
		priv->menu = NULL;
	}

        if (priv->menuservice != NULL) {
                g_object_unref (priv->menuservice);
        }

	if (priv->dbus_proxy != NULL) {
		g_object_unref(G_OBJECT(priv->dbus_proxy));
		priv->dbus_proxy = NULL;
	}

	if (priv->watcher_proxy != NULL) {
		dbus_g_connection_flush(priv->connection);
		g_signal_handlers_disconnect_by_func(G_OBJECT(priv->watcher_proxy), watcher_proxy_destroyed, self);
		g_object_unref(G_OBJECT(priv->watcher_proxy));
		priv->watcher_proxy = NULL;

	    /* Emit the AppIndicator::connection-changed signal*/
        g_signal_emit (self, signals[CONNECTION_CHANGED], 0, FALSE);
	}

	if (priv->connection != NULL) {
		dbus_g_connection_unref(priv->connection);
		priv->connection = NULL;
	}

	G_OBJECT_CLASS (app_indicator_parent_class)->dispose (object);
	return;
}

/* Free all of the memory that we could be using in
   the object. */
static void
app_indicator_finalize (GObject *object)
{
        AppIndicator * self = APP_INDICATOR(object);
        AppIndicatorPrivate *priv = self->priv;

	if (priv->status != APP_INDICATOR_STATUS_PASSIVE) {
		g_warning("Finalizing Application Status with the status set to: %d", priv->status);
	}

	if (priv->id != NULL) {
		g_free(priv->id);
		priv->id = NULL;
	}

	if (priv->clean_id != NULL) {
		g_free(priv->clean_id);
		priv->clean_id = NULL;
	}

	if (priv->icon_name != NULL) {
		g_free(priv->icon_name);
		priv->icon_name = NULL;
	}

	if (priv->attention_icon_name != NULL) {
		g_free(priv->attention_icon_name);
		priv->attention_icon_name = NULL;
	}

	if (priv->icon_theme_path != NULL) {
		g_free(priv->icon_theme_path);
		priv->icon_theme_path = NULL;
	}
	
	if (priv->label != NULL) {
		g_free(priv->label);
		priv->label = NULL;
	}

	if (priv->label_guide != NULL) {
		g_free(priv->label_guide);
		priv->label_guide = NULL;
	}

	G_OBJECT_CLASS (app_indicator_parent_class)->finalize (object);
	return;
}

#define WARN_BAD_TYPE(prop, value)  g_warning("Can not work with property '%s' with value of type '%s'.", prop, G_VALUE_TYPE_NAME(value))

/* Set some properties */
static void
app_indicator_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec)
{
        AppIndicator *self = APP_INDICATOR (object);
        AppIndicatorPrivate *priv = self->priv;
        GEnumValue *enum_val;

        switch (prop_id) {
        case PROP_ID:
          if (priv->id != NULL) {
            g_warning ("Resetting ID value when I already had a value of: %s", priv->id);
            break;
          }

          priv->id = g_strdup (g_value_get_string (value));

          priv->clean_id = g_strdup(priv->id);
          gchar * cleaner;
          for (cleaner = priv->clean_id; *cleaner != '\0'; cleaner++) {
            if (!g_ascii_isalnum(*cleaner)) {
              *cleaner = '_';
            }
          }

          check_connect (self);
          break;

        case PROP_CATEGORY:
          enum_val = g_enum_get_value_by_nick ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_CATEGORY),
                                               g_value_get_string (value));

          if (priv->category != enum_val->value)
            {
              priv->category = enum_val->value;
            }

          break;

        case PROP_STATUS:
          enum_val = g_enum_get_value_by_nick ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_STATUS),
                                               g_value_get_string (value));

          app_indicator_set_status (APP_INDICATOR (object),
                                    enum_val->value);
          break;

        case PROP_ICON_NAME:
          app_indicator_set_icon (APP_INDICATOR (object),
                                  g_value_get_string (value));
          check_connect (self);
          break;

        case PROP_ATTENTION_ICON_NAME:
          app_indicator_set_attention_icon (APP_INDICATOR (object),
                                            g_value_get_string (value));
          break;

        case PROP_ICON_THEME_PATH:
          app_indicator_set_icon_theme_path (APP_INDICATOR (object),
                                            g_value_get_string (value));
          check_connect (self);
          break;

		case PROP_X_LABEL:
		case PROP_LABEL: {
		  gchar * oldlabel = priv->label;
		  priv->label = g_value_dup_string(value);

		  if (g_strcmp0(oldlabel, priv->label) != 0) {
		    signal_label_change(APP_INDICATOR(object));
		  }

		  if (priv->label != NULL && priv->label[0] == '\0') {
		  	g_free(priv->label);
			priv->label = NULL;
		  }

		  if (oldlabel != NULL) {
		  	g_free(oldlabel);
		  }
		  break;
		}
		case PROP_X_LABEL_GUIDE:
		case PROP_LABEL_GUIDE: {
		  gchar * oldguide = priv->label_guide;
		  priv->label_guide = g_value_dup_string(value);

		  if (g_strcmp0(oldguide, priv->label_guide) != 0) {
		    signal_label_change(APP_INDICATOR(object));
		  }

		  if (priv->label_guide != NULL && priv->label_guide[0] == '\0') {
		  	g_free(priv->label_guide);
			priv->label_guide = NULL;
		  }

		  if (oldguide != NULL) {
		  	g_free(oldguide);
		  }
		  break;
		}
		case PROP_X_ORDERING_INDEX:
		case PROP_ORDERING_INDEX:
		  priv->ordering_index = g_value_get_uint(value);
		  break;

        default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
          break;
        }

	return;
}

/* Function to fill our value with the property it's requesting. */
static void
app_indicator_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec)
{
        AppIndicator *self = APP_INDICATOR (object);
        AppIndicatorPrivate *priv = self->priv;
        GEnumValue *enum_value;

        switch (prop_id) {
        case PROP_ID:
          g_value_set_string (value, priv->id);
          break;

        case PROP_CATEGORY:
          enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_CATEGORY), priv->category);
          g_value_set_string (value, enum_value->value_nick);
          break;

        case PROP_STATUS:
          enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_STATUS), priv->status);
          g_value_set_string (value, enum_value->value_nick);
          break;

        case PROP_ICON_NAME:
          g_value_set_string (value, priv->icon_name);
          break;

        case PROP_ATTENTION_ICON_NAME:
          g_value_set_string (value, priv->attention_icon_name);
          break;

        case PROP_ICON_THEME_PATH:
          g_value_set_string (value, priv->icon_theme_path);
          break;

        case PROP_MENU:
          if (priv->menuservice != NULL) {
            GValue strval = { 0 };
            g_value_init(&strval, G_TYPE_STRING);
            g_object_get_property (G_OBJECT (priv->menuservice), DBUSMENU_SERVER_PROP_DBUS_OBJECT, &strval);
            g_value_set_boxed(value, g_value_get_string(&strval));
            g_value_unset(&strval);
          }
          break;

        case PROP_CONNECTED:
          g_value_set_boolean (value, priv->watcher_proxy != NULL ? TRUE : FALSE);
          break;

		case PROP_X_LABEL:
        case PROP_LABEL:
          g_value_set_string (value, priv->label);
          break;

        case PROP_X_LABEL_GUIDE:
        case PROP_LABEL_GUIDE:
          g_value_set_string (value, priv->label_guide);
          break;

		case PROP_X_ORDERING_INDEX:
		case PROP_ORDERING_INDEX:
		  g_value_set_uint(value, priv->ordering_index);
		  break;

        default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
          break;
        }

	return;
}

/* Sends the label changed signal and resets the source ID */
static gboolean
signal_label_change_idle (gpointer user_data)
{
	AppIndicator * self = (AppIndicator *)user_data;
	AppIndicatorPrivate *priv = self->priv;

	g_signal_emit(G_OBJECT(self), signals[NEW_LABEL], 0,
	              priv->label != NULL ? priv->label : "",
	              priv->label_guide != NULL ? priv->label_guide : "",
	              TRUE);
	g_signal_emit(G_OBJECT(self), signals[X_NEW_LABEL], 0,
	              priv->label != NULL ? priv->label : "",
	              priv->label_guide != NULL ? priv->label_guide : "",
	              TRUE);

	priv->label_change_idle = 0;

	return FALSE;
}

/* Sets up an idle function to send the label changed signal
   so that we don't send it too many times. */
static void
signal_label_change (AppIndicator * self)
{
	AppIndicatorPrivate *priv = self->priv;

	/* don't set it twice */
	if (priv->label_change_idle != 0) {
		return;
	}

	priv->label_change_idle = g_idle_add(signal_label_change_idle, self);
	return;
}

/* This function is used to see if we have enough information to
   connect to things.  If we do, and we're not connected, it
   connects for us. */
static void
check_connect (AppIndicator *self)
{
	AppIndicatorPrivate *priv = self->priv;

	/* We're alreadying connecting or trying to connect. */
	if (priv->watcher_proxy != NULL) return;

	/* Do we have enough information? */
	if (priv->menu == NULL) return;
	if (priv->icon_name == NULL) return;
	if (priv->id == NULL) return;

	gchar * path = g_strdup_printf(DEFAULT_ITEM_PATH "/%s", priv->clean_id);

	dbus_g_connection_register_g_object(priv->connection,
	                                    path,
	                                    G_OBJECT(self));

	GError * error = NULL;
	priv->watcher_proxy = dbus_g_proxy_new_for_name_owner(priv->connection,
	                                                      NOTIFICATION_WATCHER_DBUS_ADDR,
	                                                      NOTIFICATION_WATCHER_DBUS_OBJ,
	                                                      NOTIFICATION_WATCHER_DBUS_IFACE,
	                                                      &error);
	if (error != NULL) {
		/* Unable to get proxy, but we're handling that now so
		   it's not a warning anymore. */
		g_error_free(error);
		dbus_g_connection_unregister_g_object(priv->connection,
						      G_OBJECT(self));
		start_fallback_timer(self, FALSE);
		g_free(path);
		return;
	}

	g_signal_connect(G_OBJECT(priv->watcher_proxy), "destroy", G_CALLBACK(watcher_proxy_destroyed), self);
	org_kde_StatusNotifierWatcher_register_status_notifier_item_async(priv->watcher_proxy, path, register_service_cb, self);
	g_free(path);

	/* Emit the AppIndicator::connection-changed signal*/
    g_signal_emit (self, signals[CONNECTION_CHANGED], 0, TRUE);

	return;
}

/* A function that gets called when the watcher dies.  Like
   dies dies.  Not our friend anymore. */
static void
watcher_proxy_destroyed (GObject * object, gpointer data)
{
	AppIndicator * self = APP_INDICATOR(data);
	g_return_if_fail(self != NULL);

	dbus_g_connection_unregister_g_object(self->priv->connection,
					      G_OBJECT(self));
	self->priv->watcher_proxy = NULL;

    /* Emit the AppIndicator::connection-changed signal*/
    g_signal_emit (self, signals[CONNECTION_CHANGED], 0, FALSE);
	
	start_fallback_timer(self, FALSE);
	return;
}

/* Responce from the DBus command to register a service
   with a NotificationWatcher. */
static void
register_service_cb (DBusGProxy * proxy, GError * error, gpointer data)
{
	g_return_if_fail(IS_APP_INDICATOR(data));
	AppIndicatorPrivate * priv = APP_INDICATOR(data)->priv;

	if (error != NULL) {
		/* They didn't respond, ewww.  Not sure what they could
		   be doing */
		g_warning("Unable to connect to the Notification Watcher: %s", error->message);
		dbus_g_connection_unregister_g_object(priv->connection,
						      G_OBJECT(data));
		g_object_unref(G_OBJECT(priv->watcher_proxy));
		priv->watcher_proxy = NULL;
		start_fallback_timer(APP_INDICATOR(data), TRUE);
	}

	if (priv->status_icon) {
		AppIndicatorClass * class = APP_INDICATOR_GET_CLASS(data);
		if (class->unfallback != NULL) {
			class->unfallback(APP_INDICATOR(data), priv->status_icon);
			priv->status_icon = NULL;
		} 
	}

	return;
}

/* A helper function to get the nick out of a given
   category enum value. */
static const gchar *
category_from_enum (AppIndicatorCategory category)
{
  GEnumValue *value;

  value = g_enum_get_value ((GEnumClass *)g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_CATEGORY), category);
  return value->value_nick;
}

/* Watching the dbus owner change events to see if someone
   we care about pops up! */
static void
dbus_owner_change (DBusGProxy * proxy, const gchar * name, const gchar * prev, const gchar * new, gpointer data)
{
	if (new == NULL || new[0] == '\0') {
		/* We only care about folks coming on the bus.  Exit quickly otherwise. */
		return;
	}

	if (g_strcmp0(name, NOTIFICATION_WATCHER_DBUS_ADDR)) {
		/* We only care about this address, reject all others. */
		return;
	}

	/* Woot, there's a new notification watcher in town. */

	AppIndicatorPrivate * priv = APP_INDICATOR_GET_PRIVATE(data);

	if (priv->fallback_timer != 0) {
		/* Stop a timer */
		g_source_remove(priv->fallback_timer);

		/* Stop listening to bus events */
		g_object_unref(G_OBJECT(priv->dbus_proxy));
		priv->dbus_proxy = NULL;
	}

	/* Let's start from the very beginning */
	check_connect(APP_INDICATOR(data));

	return;
}

/* This is an idle function to create the proxy.  This is mostly
   because start_fallback_timer can get called in the distruction
   of a proxy and thus the proxy manager gets confused when creating
   a new proxy as part of destroying an old one.  This function being
   on idle means that we'll just do it outside of the same stack where
   the previous proxy is being destroyed. */
static gboolean
setup_name_owner_proxy (gpointer data)
{
	g_return_val_if_fail(IS_APP_INDICATOR(data), FALSE);
	AppIndicatorPrivate * priv = APP_INDICATOR(data)->priv;

	if (priv->dbus_proxy == NULL) {
		priv->dbus_proxy = dbus_g_proxy_new_for_name(priv->connection,
		                                             DBUS_SERVICE_DBUS,
		                                             DBUS_PATH_DBUS,
		                                             DBUS_INTERFACE_DBUS);
		dbus_g_proxy_add_signal(priv->dbus_proxy, "NameOwnerChanged",
		                        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		                        G_TYPE_INVALID);
		dbus_g_proxy_connect_signal(priv->dbus_proxy, "NameOwnerChanged",
		                            G_CALLBACK(dbus_owner_change), data, NULL);
	}

	return FALSE;
}

/* A function that will start the fallback timer if it's not
   already started.  It sets up the DBus watcher to see if
   there is a change.  Also, provides an override mode for cases
   where it's unlikely that a timer will help anything. */
static void
start_fallback_timer (AppIndicator * self, gboolean disable_timeout)
{
	g_return_if_fail(IS_APP_INDICATOR(self));
	AppIndicatorPrivate * priv = APP_INDICATOR(self)->priv;

	if (priv->fallback_timer != 0) {
		/* The timer is set, let's just be happy with the one
		   we've already got running */
		return;
	}

	if (priv->status_icon != NULL) {
		/* We're already fallen back.  Let's not do it again. */
		return;
	}

	if (priv->dbus_proxy == NULL) {
		/* NOTE: Read the comment on setup_name_owner_proxy */
		g_idle_add(setup_name_owner_proxy, self);
	}

	if (disable_timeout) {
		fallback_timer_expire(self);
	} else {
		priv->fallback_timer = g_timeout_add(DEFAULT_FALLBACK_TIMER, fallback_timer_expire, self);
	}

	return;
}

/* A function that gets executed when we want to change the
   state of the fallback. */
static gboolean
fallback_timer_expire (gpointer data)
{
	g_return_val_if_fail(IS_APP_INDICATOR(data), FALSE);

	AppIndicatorPrivate * priv = APP_INDICATOR(data)->priv;
	AppIndicatorClass * class = APP_INDICATOR_GET_CLASS(data);

	if (priv->status_icon == NULL) {
		if (class->fallback != NULL) {
			priv->status_icon = class->fallback(APP_INDICATOR(data));
		} 
	} else {
		if (class->unfallback != NULL) {
			class->unfallback(APP_INDICATOR(data), priv->status_icon);
			priv->status_icon = NULL;
		} else {
			g_warning("No 'unfallback' function but the 'fallback' function returned a non-NULL result.");
		}
	}

	priv->fallback_timer = 0;
	return FALSE;
}

/* emit a NEW_ICON signal in response for the theme change */
static void
theme_changed_cb (GtkIconTheme * theme, gpointer user_data)
{
	g_signal_emit (user_data, signals[NEW_ICON], 0, TRUE);
}

/* Creates a StatusIcon that can be used when the application
   indicator area isn't available. */
static GtkStatusIcon *
fallback (AppIndicator * self)
{
	GtkStatusIcon * icon = gtk_status_icon_new();

	gtk_status_icon_set_title(icon, app_indicator_get_id(self));
	
	g_signal_connect(G_OBJECT(self), APP_INDICATOR_SIGNAL_NEW_STATUS,
		G_CALLBACK(status_icon_status_wrapper), icon);
	g_signal_connect(G_OBJECT(self), APP_INDICATOR_SIGNAL_NEW_ICON,
		G_CALLBACK(status_icon_changes), icon);
	g_signal_connect(G_OBJECT(self), APP_INDICATOR_SIGNAL_NEW_ATTENTION_ICON,
		G_CALLBACK(status_icon_changes), icon);

	status_icon_changes(self, icon);

	g_signal_connect(G_OBJECT(icon), "activate", G_CALLBACK(status_icon_activate), self);

	return icon;
}

/* A wrapper as the status update prototype is a little
   bit different, but we want to handle it the same. */
static void
status_icon_status_wrapper (AppIndicator * self, const gchar * status, gpointer data)
{
	return status_icon_changes(self, data);
}

/* This tracks changes to either the status or the icons
   that are associated with the app indicator */
static void
status_icon_changes (AppIndicator * self, gpointer data)
{
	GtkStatusIcon * icon = GTK_STATUS_ICON(data);
	GIcon *themed_icon = NULL;
	gchar *longname = NULL;

	switch (app_indicator_get_status(self)) {
	case APP_INDICATOR_STATUS_PASSIVE:
		longname = append_panel_icon_suffix(app_indicator_get_icon(self));
		themed_icon = g_themed_icon_new_with_default_fallbacks (longname);
		gtk_status_icon_set_visible(icon, FALSE);
		gtk_status_icon_set_from_gicon(icon, themed_icon);
		break;
	case APP_INDICATOR_STATUS_ACTIVE:
		longname = append_panel_icon_suffix(app_indicator_get_icon(self));
		themed_icon = g_themed_icon_new_with_default_fallbacks (longname);
		gtk_status_icon_set_from_gicon(icon, themed_icon);
		gtk_status_icon_set_visible(icon, TRUE);
		break;
	case APP_INDICATOR_STATUS_ATTENTION:
		longname = append_panel_icon_suffix(app_indicator_get_attention_icon(self));
		themed_icon = g_themed_icon_new_with_default_fallbacks (longname);
		gtk_status_icon_set_from_gicon(icon, themed_icon);
		gtk_status_icon_set_visible(icon, TRUE);
		break;
	};

	if (themed_icon) {
		g_object_unref (themed_icon);
	}

	if (longname) {
		g_free(longname);
	}

	return;
}

/* Handles the activate action by the status icon by showing
   the menu in a popup. */
static void
status_icon_activate (GtkStatusIcon * icon, gpointer data)
{
	GtkMenu * menu = app_indicator_get_menu(APP_INDICATOR(data));
	if (menu == NULL)
		return;
	
	gtk_menu_popup(menu,
	               NULL, /* Parent Menu */
	               NULL, /* Parent item */
	               gtk_status_icon_position_menu,
	               icon,
	               1, /* Button */
	               gtk_get_current_event_time());

	return;
}

/* Removes the status icon as the application indicator area
   is now up and running again. */
static void
unfallback (AppIndicator * self, GtkStatusIcon * status_icon)
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(self), status_icon_status_wrapper, status_icon);
	g_signal_handlers_disconnect_by_func(G_OBJECT(self), status_icon_changes, status_icon);
	gtk_status_icon_set_visible(status_icon, FALSE);
	g_object_unref(G_OBJECT(status_icon));
	return;
}

/* A helper function that appends PANEL_ICON_SUFFIX to the given icon name
   if it's missing. */
static gchar *
append_panel_icon_suffix (const gchar *icon_name)
{
	gchar * long_name = NULL;

	if (!g_str_has_suffix (icon_name, PANEL_ICON_SUFFIX)) {
		long_name =
		    g_strdup_printf("%s-%s", icon_name, PANEL_ICON_SUFFIX);
        } else {
           	long_name = g_strdup (icon_name);
        }

	return long_name;	
}


/* ************************* */
/*    Public Functions       */
/* ************************* */

/**
        app_indicator_new:
        @id: The unique id of the indicator to create.
        @icon_name: The icon name for this indicator
        @category: The category of indicator.

		Creates a new #AppIndicator setting the properties:
		#AppIndicator:id with @id, #AppIndicator:category
		with @category and #AppIndicator:icon-name with
		@icon_name.

        Return value: A pointer to a new #AppIndicator object.
 */
AppIndicator *
app_indicator_new (const gchar          *id,
                   const gchar          *icon_name,
                   AppIndicatorCategory  category)
{
  AppIndicator *indicator = g_object_new (APP_INDICATOR_TYPE,
                                          PROP_ID_S, id,
                                          PROP_CATEGORY_S, category_from_enum (category),
                                          PROP_ICON_NAME_S, icon_name,
                                          NULL);

  return indicator;
}

/**
        app_indicator_new_with_path:
        @id: The unique id of the indicator to create.
        @icon_name: The icon name for this indicator
        @category: The category of indicator.
        @icon_theme_path: A custom path for finding icons.

		Creates a new #AppIndicator setting the properties:
		#AppIndicator:id with @id, #AppIndicator:category
		with @category, #AppIndicator:icon-name with
		@icon_name and #AppIndicator:icon-theme-path with @icon_theme_path.

        Return value: A pointer to a new #AppIndicator object.
 */
AppIndicator *
app_indicator_new_with_path (const gchar          *id,
                             const gchar          *icon_name,
                             AppIndicatorCategory  category,
                             const gchar          *icon_theme_path)
{
	AppIndicator *indicator = g_object_new (APP_INDICATOR_TYPE,
	                                        PROP_ID_S, id,
	                                        PROP_CATEGORY_S, category_from_enum (category),
	                                        PROP_ICON_NAME_S, icon_name,
	                                        PROP_ICON_THEME_PATH_S, icon_theme_path,
	                                        NULL);

	return indicator;
}

/**
	app_indicator_get_type:

	Generates or returns the unique #GType for #AppIndicator.

	Return value: A unique #GType for #AppIndicator objects.
*/

/**
	app_indicator_set_status:
	@self: The #AppIndicator object to use
	@status: The status to set for this indicator

	Wrapper function for property #AppIndicator:status.
*/
void
app_indicator_set_status (AppIndicator *self, AppIndicatorStatus status)
{
  g_return_if_fail (IS_APP_INDICATOR (self));

  if (self->priv->status != status)
    {
      GEnumValue *value = g_enum_get_value ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_STATUS), status);

      self->priv->status = status;
      g_signal_emit (self, signals[NEW_STATUS], 0, value->value_nick);
    }
}

/**
	app_indicator_set_attention_icon:
	@self: The #AppIndicator object to use
	@icon_name: The name of the attention icon to set for this indicator

	Wrapper function for property #AppIndicator:attention-icon-name.
*/
void
app_indicator_set_attention_icon (AppIndicator *self, const gchar *icon_name)
{
  g_return_if_fail (IS_APP_INDICATOR (self));
  g_return_if_fail (icon_name != NULL);

  if (g_strcmp0 (self->priv->attention_icon_name, icon_name) != 0)
    {
      if (self->priv->attention_icon_name)
        g_free (self->priv->attention_icon_name);

      self->priv->attention_icon_name = g_strdup(icon_name);

      g_signal_emit (self, signals[NEW_ATTENTION_ICON], 0, TRUE);
    }

  return;
}

/**
        app_indicator_set_icon:
        @self: The #AppIndicator object to use
        @icon_name: The icon name to set.

		Sets the default icon to use when the status is active but
		not set to attention.  In most cases, this should be the
		application icon for the program.
		Wrapper function for property #AppIndicator:icon-name.
**/
void
app_indicator_set_icon (AppIndicator *self, const gchar *icon_name)
{
  g_return_if_fail (IS_APP_INDICATOR (self));
  g_return_if_fail (icon_name != NULL);

  if (g_strcmp0 (self->priv->icon_name, icon_name) != 0)
    {
      if (self->priv->icon_name)
        g_free (self->priv->icon_name);

      self->priv->icon_name = g_strdup(icon_name);

      g_signal_emit (self, signals[NEW_ICON], 0, TRUE);
    }

  return;
}

/**
	app_indicator_set_label:
	@self: The #AppIndicator object to use
	@label: The label to show next to the icon.
	@guide: A guide to size the label correctly.

	This is a wrapper function for the #AppIndicator:label and
	#AppIndicator:guide properties.  This function can take #NULL
	as either @label or @guide and will clear the entries.
*/
void
app_indicator_set_label (AppIndicator *self, const gchar * label, const gchar * guide)
{
	g_return_if_fail (IS_APP_INDICATOR (self));
	/* Note: The label can be NULL, it's okay */
	/* Note: The guide can be NULL, it's okay */

	g_object_set(G_OBJECT(self),
	             PROP_LABEL_S,       label == NULL ? "" : label,
	             PROP_LABEL_GUIDE_S, guide == NULL ? "" : guide,
	             NULL);

	return;
}

/**
        app_indicator_set_icon_theme_path:
        @self: The #AppIndicator object to use
        @icon_theme_path: The icon theme path to set.

		Sets the path to use when searching for icons.
**/
void
app_indicator_set_icon_theme_path (AppIndicator *self, const gchar *icon_theme_path)
{
  g_return_if_fail (IS_APP_INDICATOR (self));

  if (g_strcmp0 (self->priv->icon_theme_path, icon_theme_path) != 0)
    {
      if (self->priv->icon_theme_path != NULL)
            g_free(self->priv->icon_theme_path);

      self->priv->icon_theme_path = g_strdup(icon_theme_path);

      g_signal_emit (self, signals[NEW_ICON_THEME_PATH], 0, g_strdup(self->priv->icon_theme_path));
    }

  return;
}

static void
activate_menuitem (DbusmenuMenuitem *mi, guint timestamp, gpointer user_data)
{
  GtkWidget *widget = (GtkWidget *)user_data;

  gtk_menu_item_activate (GTK_MENU_ITEM (widget));
}

static void
widget_toggled (GtkWidget *widget, DbusmenuMenuitem *mi)
{
  dbusmenu_menuitem_property_set_int (mi,
                                      DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                      gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget)) ? DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED : DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);
}

static void
menuitem_iterate (GtkWidget *widget,
                  gpointer   data)
{
  if (GTK_IS_LABEL (widget))
    {
      DbusmenuMenuitem *child = (DbusmenuMenuitem *)data;

      dbusmenu_menuitem_property_set (child,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      gtk_label_get_text (GTK_LABEL (widget)));
    }
}

static gboolean
should_show_image (GtkImage *image)
{
  GtkWidget *item;

  item = gtk_widget_get_ancestor (GTK_WIDGET (image),
                                  GTK_TYPE_IMAGE_MENU_ITEM);

  if (item)
    {
      GtkSettings *settings;
      gboolean gtk_menu_images;

      settings = gtk_widget_get_settings (item);

      g_object_get (settings, "gtk-menu-images", &gtk_menu_images, NULL);

      if (gtk_menu_images)
        return TRUE;

      return gtk_image_menu_item_get_always_show_image (GTK_IMAGE_MENU_ITEM (item));
    }

  return FALSE;
}

static void
update_icon_name (DbusmenuMenuitem *menuitem,
                  GtkImage         *image)
{
  if (gtk_image_get_storage_type (image) != GTK_IMAGE_ICON_NAME)
    return;

  if (should_show_image (image))
    dbusmenu_menuitem_property_set (menuitem,
                                    DBUSMENU_MENUITEM_PROP_ICON_NAME,
                                    image->data.name.icon_name);
  else
    dbusmenu_menuitem_property_remove (menuitem,
                                       DBUSMENU_MENUITEM_PROP_ICON_NAME);
}

/* return value specifies whether the label is set or not */
static gboolean
update_stock_item (DbusmenuMenuitem *menuitem,
                   GtkImage         *image)
{
  GtkStockItem stock;

  if (gtk_image_get_storage_type (image) != GTK_IMAGE_STOCK)
    return FALSE;

  gtk_stock_lookup (image->data.stock.stock_id, &stock);

  if (should_show_image (image))
    dbusmenu_menuitem_property_set (menuitem,
                                    DBUSMENU_MENUITEM_PROP_ICON_NAME,
                                    image->data.stock.stock_id);
  else
    dbusmenu_menuitem_property_remove (menuitem,
                                       DBUSMENU_MENUITEM_PROP_ICON_NAME);

  const gchar * label = dbusmenu_menuitem_property_get (menuitem,
                                  DBUSMENU_MENUITEM_PROP_LABEL);
  
  if (stock.label != NULL && label != NULL)
    {
      dbusmenu_menuitem_property_set (menuitem,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      stock.label);

      return TRUE;
    }

  return FALSE;
}

static void
image_notify_cb (GtkWidget  *widget,
                 GParamSpec *pspec,
                 gpointer    data)
{
  DbusmenuMenuitem *child = (DbusmenuMenuitem *)data;
  GtkImage *image = GTK_IMAGE (widget);

  if (pspec->name == g_intern_static_string ("stock"))
    {
      update_stock_item (child, image);
    }
  else if (pspec->name == g_intern_static_string ("icon-name"))
    {
      update_icon_name (child, image);
    }
}

static void
widget_notify_cb (GtkWidget  *widget,
                  GParamSpec *pspec,
                  gpointer    data)
{
  DbusmenuMenuitem *child = (DbusmenuMenuitem *)data;

  if (pspec->name == g_intern_static_string ("sensitive"))
    {
      dbusmenu_menuitem_property_set_bool (child,
                                           DBUSMENU_MENUITEM_PROP_ENABLED,
                                           GTK_WIDGET_IS_SENSITIVE (widget));
    }
  else if (pspec->name == g_intern_static_string ("label"))
    {
      dbusmenu_menuitem_property_set (child,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      gtk_menu_item_get_label (GTK_MENU_ITEM (widget)));
    }
  else if (pspec->name == g_intern_static_string ("visible"))
    {
      dbusmenu_menuitem_property_set_bool (child,
                                           DBUSMENU_MENUITEM_PROP_VISIBLE,
                                           gtk_widget_get_visible (widget));
    }
}

static void
action_notify_cb (GtkAction  *action,
                  GParamSpec *pspec,
                  gpointer    data)
{
  DbusmenuMenuitem *child = (DbusmenuMenuitem *)data;

  if (pspec->name == g_intern_static_string ("active"))
    {
      dbusmenu_menuitem_property_set_bool (child,
                                      DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                      gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)));
    }
    
  if (pspec->name == g_intern_static_string ("label"))
    {
      dbusmenu_menuitem_property_set (child,
                                      DBUSMENU_MENUITEM_PROP_LABEL,
                                      gtk_action_get_label (action));
    }
}

static void
container_iterate (GtkWidget *widget,
                   gpointer   data)
{
  DbusmenuMenuitem *root = (DbusmenuMenuitem *)data;
  DbusmenuMenuitem *child;
  GtkWidget *submenu = NULL;
  const gchar *label = NULL;
  gboolean label_set = FALSE;

  if (GTK_IS_TEAROFF_MENU_ITEM(widget)) {
  	return;
  }

  child = dbusmenu_menuitem_new ();

  if (GTK_IS_SEPARATOR_MENU_ITEM (widget))
    {
      dbusmenu_menuitem_property_set (child,
                                      "type",
                                      DBUSMENU_CLIENT_TYPES_SEPARATOR);
    }
  else
    {
      if (GTK_IS_CHECK_MENU_ITEM (widget))
        {
          GtkCheckMenuItem *check;

          check = GTK_CHECK_MENU_ITEM (widget);
          label = gtk_menu_item_get_label (GTK_MENU_ITEM (widget));

          dbusmenu_menuitem_property_set (child,
                                          DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE,
                                          GTK_IS_RADIO_MENU_ITEM (widget) ? DBUSMENU_MENUITEM_TOGGLE_RADIO : DBUSMENU_MENUITEM_TOGGLE_CHECK);

          dbusmenu_menuitem_property_set (child,
                                          DBUSMENU_MENUITEM_PROP_LABEL,
                                          label);

          label_set = TRUE;

          dbusmenu_menuitem_property_set_int (child,
                                              DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                              gtk_check_menu_item_get_active (check) ? DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED : DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);

          g_signal_connect (widget,
                            "toggled",
                            G_CALLBACK (widget_toggled),
                            child);
        }
      else if (GTK_IS_IMAGE_MENU_ITEM (widget))
        {
          GtkWidget *image;
          GtkImageType image_type;

          image = gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM (widget));
          image_type = gtk_image_get_storage_type (GTK_IMAGE (image));

          g_signal_connect (image,
                            "notify",
                            G_CALLBACK (image_notify_cb),
                            child);

          if (image_type == GTK_IMAGE_STOCK)
            {
              label_set = update_stock_item (child, GTK_IMAGE (image));
            }
          else if (image_type == GTK_IMAGE_ICON_NAME)
            {
              update_icon_name (child, GTK_IMAGE (image));
            }
        }
    }

  if (!label_set)
    {
      if (label != NULL)
        {
          dbusmenu_menuitem_property_set (child,
                                          DBUSMENU_MENUITEM_PROP_LABEL,
                                          label);
        }
      else
        {
          /* find label child widget */
          gtk_container_forall (GTK_CONTAINER (widget),
                                menuitem_iterate,
                                child);
        }
    }

  if (GTK_IS_MENU_ITEM (widget))
    {
      submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (widget));
      if (submenu != NULL)
        {
          gtk_container_foreach (GTK_CONTAINER (submenu),
                                container_iterate,
                                child);
          g_signal_connect_object (submenu,
                                   "child-added",
                                   G_CALLBACK (submenu_changed),
                                   child,
                                   0);
          g_signal_connect_object (submenu,
                                   "child-removed",
                                   G_CALLBACK (submenu_changed),
                                   child,
                                   0);
        }
    }

  dbusmenu_menuitem_property_set_bool (child,
                                       DBUSMENU_MENUITEM_PROP_ENABLED,
                                       GTK_WIDGET_IS_SENSITIVE (widget));
  dbusmenu_menuitem_property_set_bool (child,
                                       DBUSMENU_MENUITEM_PROP_VISIBLE,
                                       gtk_widget_get_visible (widget));

  g_signal_connect (widget, "notify",
                    G_CALLBACK (widget_notify_cb), child);

  if (GTK_IS_ACTIVATABLE (widget))
    {
      GtkActivatable *activatable = GTK_ACTIVATABLE (widget);

      if (gtk_activatable_get_use_action_appearance (activatable))
        {
          GtkAction *action = gtk_activatable_get_related_action (activatable);

          if (action)
            {
              g_signal_connect_object (action, "notify",
                                       G_CALLBACK (action_notify_cb),
                                       child,
                                       G_CONNECT_AFTER);
            }
        }
    }

  g_signal_connect (G_OBJECT (child),
                    DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                    G_CALLBACK (activate_menuitem), widget);
  dbusmenu_menuitem_child_append (root, child);

  /* Get rid of initial ref now that the root is
     holding the object */
  g_object_unref(child);

  return;
}

static void
submenu_changed (GtkWidget *widget,
                 GtkWidget *child,
                 gpointer   data)
{
  DbusmenuMenuitem *root = (DbusmenuMenuitem *)data;
  GList *children, *l;
  children = dbusmenu_menuitem_get_children (root);

  for (l = children; l;)
    {
      DbusmenuMenuitem *c = (DbusmenuMenuitem *)l->data;
      l = l->next;
      dbusmenu_menuitem_child_delete (root, c);
    }

  gtk_container_foreach (GTK_CONTAINER (widget),
                        container_iterate,
                        root);
}

static void
setup_dbusmenu (AppIndicator *self)
{
  AppIndicatorPrivate *priv;
  DbusmenuMenuitem *root;

  priv = self->priv;
  root = dbusmenu_menuitem_new ();

  if (priv->menu)
    {
      gtk_container_foreach (GTK_CONTAINER (priv->menu),
                            container_iterate,
                            root);
    }

  if (priv->menuservice == NULL)
    {
      gchar * path = g_strdup_printf(DEFAULT_ITEM_PATH "/%s/Menu", priv->clean_id);
      priv->menuservice = dbusmenu_server_new (path);
      g_free(path);
    }

  dbusmenu_server_set_root (priv->menuservice, root);

  return;
}

static void
client_menu_changed (GtkWidget    *widget,
                     GtkWidget    *child,
                     AppIndicator *indicator)
{
  setup_dbusmenu (indicator);
}

/**
        app_indicator_set_menu:
        @self: The #AppIndicator
        @menu: A #GtkMenu to set

        Sets the menu that should be shown when the Application Indicator
        is clicked on in the panel.  An application indicator will not
        be rendered unless it has a menu.
        
        Wrapper function for property #AppIndicator:menu.
**/
void
app_indicator_set_menu (AppIndicator *self, GtkMenu *menu)
{
  AppIndicatorPrivate *priv;

  g_return_if_fail (IS_APP_INDICATOR (self));
  g_return_if_fail (GTK_IS_MENU (menu));
  g_return_if_fail (self->priv->clean_id != NULL);

  priv = self->priv;

  if (priv->menu != NULL)
    {
      g_object_unref (priv->menu);
    }

  priv->menu = GTK_WIDGET (menu);
  g_object_ref (priv->menu);

  setup_dbusmenu (self);

  check_connect (self);

  g_signal_connect (menu,
                    "child-added",
                    G_CALLBACK (client_menu_changed),
                    self);
  g_signal_connect (menu,
                    "child-removed",
                    G_CALLBACK (client_menu_changed),
                    self);
}

/**
	app_indicator_set_ordering_index:
	@self: The #AppIndicator
	@ordering_index: A value for the ordering of this app indicator

	Sets the ordering index for the app indicator which effects the
	placement of it on the panel.  For almost all app indicator
	this is not the function you're looking for.

	Wrapper function for property #AppIndicator:ordering-index.
**/
void
app_indicator_set_ordering_index (AppIndicator *self, guint32 ordering_index)
{
	g_return_if_fail (IS_APP_INDICATOR (self));

	self->priv->ordering_index = ordering_index;

	return;
}

/**
	app_indicator_get_id:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator:id.

	Return value: The current ID
*/
const gchar *
app_indicator_get_id (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->id;
}

/**
	app_indicator_get_category:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator:category.

	Return value: The current category.
*/
AppIndicatorCategory
app_indicator_get_category (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

  return self->priv->category;
}

/**
	app_indicator_get_status:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator:status.

	Return value: The current status.
*/
AppIndicatorStatus
app_indicator_get_status (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), APP_INDICATOR_STATUS_PASSIVE);

  return self->priv->status;
}

/**
	app_indicator_get_icon:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator:icon-name.

	Return value: The current icon name.
*/
const gchar *
app_indicator_get_icon (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->icon_name;
}

/**
	app_indicator_get_icon_theme_path:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator:icon-theme-path.

	Return value: The current icon theme path.
*/
const gchar *
app_indicator_get_icon_theme_path (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->icon_theme_path;
}

/**
	app_indicator_get_attention_icon:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator:attention-icon-name.

	Return value: The current attention icon name.
*/
const gchar *
app_indicator_get_attention_icon (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->attention_icon_name;
}

/**
	app_indicator_get_menu:
	@self: The #AppIndicator object to use

	Gets the menu being used for this application indicator.
	Wrapper function for property #AppIndicator:menu.

	Return value: A #GtkMenu object or %NULL if one hasn't been set.
*/
GtkMenu *
app_indicator_get_menu (AppIndicator *self)
{
	AppIndicatorPrivate *priv;

	g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

	priv = self->priv;

	return GTK_MENU(priv->menu);
}

/**
	app_indicator_get_label:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator:label.

	Return value: The current label.
*/
const gchar *
app_indicator_get_label (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->label;
}

/**
	app_indicator_get_label_guide:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator:label-guide.

	Return value: The current label guide.
*/
const gchar *
app_indicator_get_label_guide (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->label_guide;
}

/**
	app_indicator_get_ordering_index:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator:ordering-index.

	Return value: The current ordering index.
*/
guint32
app_indicator_get_ordering_index (AppIndicator *self)
{
	g_return_val_if_fail (IS_APP_INDICATOR (self), 0);

	if (self->priv->ordering_index == 0) {
		return generate_id(self->priv->category, self->priv->id);
	} else {
		return self->priv->ordering_index;
	}
}

