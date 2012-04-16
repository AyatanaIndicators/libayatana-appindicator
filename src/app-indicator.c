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

#include <libdbusmenu-glib/menuitem.h>
#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-gtk/client.h>
#include <libdbusmenu-gtk/parser.h>

#include <libindicator/indicator-desktop-shortcuts.h>

#include "app-indicator.h"
#include "app-indicator-enum-types.h"
#include "application-service-marshal.h"

#include "gen-notification-watcher.xml.h"
#include "gen-notification-item.xml.h"

#include "dbus-shared.h"
#include "generate-id.h"

#define PANEL_ICON_SUFFIX  "panel"

/**
 * AppIndicatorPrivate:
 * All of the private data in an instance of an application indicator.
 *
 * Private Fields
 * @id: The ID of the indicator.  Maps to AppIndicator:id.
 * @category: Which category the indicator is.  Maps to AppIndicator:category.
 * @status: The status of the indicator.  Maps to AppIndicator:status.
 * @icon_name: The name of the icon to use.  Maps to AppIndicator:icon-name.
 * @attention_icon_name: The name of the attention icon to use.  Maps to AppIndicator:attention-icon-name.
 * @menu: The menu for this indicator.  Maps to AppIndicator:menu
 * @watcher_proxy: The proxy connection to the watcher we're connected to.  If we're not connected to one this will be %NULL.
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
	GtkWidget            *sec_activate_target;
	gboolean              sec_activate_enabled;
	guint32               ordering_index;
	gchar *               title;
	gchar *               label;
	gchar *               label_guide;
	gchar *               accessible_desc;
	gchar *               att_accessible_desc;
	guint                 label_change_idle;

	GtkStatusIcon *       status_icon;
	gint                  fallback_timer;

	/* Fun stuff */
	GDBusProxy           *watcher_proxy;
	GDBusConnection      *connection;
	guint                 dbus_registration;
	gchar *               path;

	/* Might be used */
	IndicatorDesktopShortcuts * shorties;
};

/* Signals Stuff */
enum {
	NEW_ICON,
	NEW_ATTENTION_ICON,
	NEW_STATUS,
	NEW_LABEL,
	CONNECTION_CHANGED,
	NEW_ICON_THEME_PATH,
	SCROLL_EVENT,
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
	PROP_ICON_DESC,
	PROP_ATTENTION_ICON_NAME,
	PROP_ATTENTION_ICON_DESC,
	PROP_ICON_THEME_PATH,
	PROP_CONNECTED,
	PROP_LABEL,
	PROP_LABEL_GUIDE,
	PROP_ORDERING_INDEX,
	PROP_DBUS_MENU_SERVER,
	PROP_TITLE
};

/* The strings so that they can be slowly looked up. */
#define PROP_ID_S                    "id"
#define PROP_CATEGORY_S              "category"
#define PROP_STATUS_S                "status"
#define PROP_ICON_NAME_S             "icon-name"
#define PROP_ICON_DESC_S             "icon-desc"
#define PROP_ATTENTION_ICON_NAME_S   "attention-icon-name"
#define PROP_ATTENTION_ICON_DESC_S   "attention-icon-desc"
#define PROP_ICON_THEME_PATH_S       "icon-theme-path"
#define PROP_CONNECTED_S             "connected"
#define PROP_LABEL_S                 "label"
#define PROP_LABEL_GUIDE_S           "label-guide"
#define PROP_ORDERING_INDEX_S        "ordering-index"
#define PROP_DBUS_MENU_SERVER_S      "dbus-menu-server"
#define PROP_TITLE_S                 "title"

/* Private macro, shhhh! */
#define APP_INDICATOR_GET_PRIVATE(o) \
                             (G_TYPE_INSTANCE_GET_PRIVATE ((o), APP_INDICATOR_TYPE, AppIndicatorPrivate))

/* Default Path */
#define DEFAULT_ITEM_PATH   "/org/ayatana/NotificationItem"

/* More constants */
#define DEFAULT_FALLBACK_TIMER  100 /* in milliseconds */

/* Globals */
static GDBusNodeInfo *            item_node_info = NULL;
static GDBusInterfaceInfo *       item_interface_info = NULL;
static GDBusNodeInfo *            watcher_node_info = NULL;
static GDBusInterfaceInfo *       watcher_interface_info = NULL;

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
static void register_service_cb (GObject * obj, GAsyncResult * res, gpointer user_data);
static void start_fallback_timer (AppIndicator * self, gboolean disable_timeout);
static gboolean fallback_timer_expire (gpointer data);
static GtkStatusIcon * fallback (AppIndicator * self);
static void status_icon_status_wrapper (AppIndicator * self, const gchar * status, gpointer data);
static gboolean scroll_event_wrapper(GtkWidget *status_icon, GdkEventScroll *event, gpointer user_data);
static gboolean middle_click_wrapper(GtkWidget *status_icon, GdkEventButton *event, gpointer user_data);
static void status_icon_changes (AppIndicator * self, gpointer data);
static void status_icon_activate (GtkStatusIcon * icon, gpointer data);
static void status_icon_menu_activate (GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data);
static void unfallback (AppIndicator * self, GtkStatusIcon * status_icon);
static gchar * append_panel_icon_suffix (const gchar * icon_name);
static void watcher_owner_changed (GObject * obj, GParamSpec * pspec, gpointer user_data);
static void theme_changed_cb (GtkIconTheme * theme, gpointer user_data);
static void sec_activate_target_parent_changed(GtkWidget *menuitem, GtkWidget *old_parent, gpointer   user_data);
static GVariant * bus_get_prop (GDBusConnection * connection, const gchar * sender, const gchar * path, const gchar * interface, const gchar * property, GError ** error, gpointer user_data);
static void bus_method_call (GDBusConnection * connection, const gchar * sender, const gchar * path, const gchar * interface, const gchar * method, GVariant * params, GDBusMethodInvocation * invocation, gpointer user_data);
static void bus_creation (GObject * obj, GAsyncResult * res, gpointer user_data);
static void bus_watcher_ready (GObject * obj, GAsyncResult * res, gpointer user_data);

static const GDBusInterfaceVTable item_interface_table = {
	method_call:    bus_method_call,
	get_property:   bus_get_prop,
	set_property:   NULL /* No properties that can be set */
};

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
	 * AppIndicator:id:
	 *
	 * The ID for this indicator, which should be unique, but used consistently
	 * by this program and its indicator.
	 */
	g_object_class_install_property (object_class,
                                         PROP_ID,
                                         g_param_spec_string(PROP_ID_S,
                                                             "The ID for this indicator",
                                                             "An ID that should be unique, but used consistently by this program and its indicator.",
                                                             NULL,
                                                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY));

	/**
	 * AppIndicator:category:
	 *
	 * The type of indicator that this represents.  Please don't use 'Other'. 
	 * Defaults to 'ApplicationStatus'.
	 */
	g_object_class_install_property (object_class,
                                         PROP_CATEGORY,
                                         g_param_spec_string (PROP_CATEGORY_S,
                                                              "Indicator Category",
                                                              "The type of indicator that this represents.  Please don't use 'other'. Defaults to 'ApplicationStatus'.",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY));

	/**
	 * AppIndicator:status:
	 *
	 * Whether the indicator is shown or requests attention. Defaults to
	 * 'Passive'.
	 */
	g_object_class_install_property (object_class,
                                         PROP_STATUS,
                                         g_param_spec_string (PROP_STATUS_S,
                                                              "Indicator Status",
                                                              "Whether the indicator is shown or requests attention. Defaults to 'Passive'.",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/**
	 * AppIndicator:icon-name:
	 *
	 * The name of the regular icon that is shown for the indicator.
	 */
	g_object_class_install_property(object_class,
	                                PROP_ICON_NAME,
	                                g_param_spec_string (PROP_ICON_NAME_S,
	                                                     "An icon for the indicator",
	                                                     "The default icon that is shown for the indicator.",
	                                                     NULL,
	                                                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	/**
	 * AppIndicator:icon-desc:
	 *
	 * The description of the regular icon that is shown for the indicator.
	 */
	g_object_class_install_property(object_class,
	                                PROP_ICON_DESC,
	                                g_param_spec_string (PROP_ICON_DESC_S,
	                                                     "A description of the icon for the indicator",
	                                                     "A description of the default icon that is shown for the indicator.",
	                                                     NULL,
	                                                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/**
	 * AppIndicator:attention-icon-name:
	 *
	 * If the indicator sets it's status to %APP_INDICATOR_STATUS_ATTENTION
	 * then this icon is shown.
	 */
	g_object_class_install_property (object_class,
                                         PROP_ATTENTION_ICON_NAME,
                                         g_param_spec_string (PROP_ATTENTION_ICON_NAME_S,
                                                              "An icon to show when the indicator request attention.",
                                                              "If the indicator sets it's status to 'attention' then this icon is shown.",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	/**
	 * AppIndicator:attention-icon-desc:
	 *
	 * If the indicator sets it's status to %APP_INDICATOR_STATUS_ATTENTION
	 * then this textual description of the icon shown.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_ATTENTION_ICON_DESC,
	                                 g_param_spec_string (PROP_ATTENTION_ICON_DESC_S,
	                                                      "A description of the icon to show when the indicator request attention.",
	                                                      "When the indicator is an attention mode this should describe the icon shown",
	                                                      NULL,
	                                                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	/**
	 * AppIndicator:icon-theme-path:
	 *
	 * An additional place to look for icon names that may be installed by the
	 * application.
	 */
	g_object_class_install_property(object_class,
	                                PROP_ICON_THEME_PATH,
	                                g_param_spec_string (PROP_ICON_THEME_PATH_S,
                                                             "An additional path for custom icons.",
                                                             "An additional place to look for icon names that may be installed by the application.",
                                                             NULL,
                                                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT));
	
	/**
	 * AppIndicator:connected:
	 * 
	 * Pretty simple, %TRUE if we have a reasonable expectation of being 
	 * displayed through this object. You should hide your TrayIcon if so.
	 */
	g_object_class_install_property (object_class,
                                         PROP_CONNECTED,
                                         g_param_spec_boolean (PROP_CONNECTED_S,
                                                               "Whether we're conneced to a watcher",
                                                               "Pretty simple, true if we have a reasonable expectation of being displayed through this object.  You should hide your TrayIcon if so.",
                                                               FALSE,
                                                               G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
	/**
	 * AppIndicator:label:
	 *
	 * A label that can be shown next to the string in the application
	 * indicator.  The label will not be shown unless there is an icon
	 * as well.  The label is useful for numerical and other frequently
	 * updated information.  In general, it shouldn't be shown unless a
	 * user requests it as it can take up a significant amount of space
	 * on the user's panel.  This may not be shown in all visualizations.
	 */
	g_object_class_install_property(object_class,
	                                PROP_LABEL,
	                                g_param_spec_string (PROP_LABEL_S,
	                                                     "A label next to the icon",
	                                                     "A label to provide dynamic information.",
	                                                     NULL,
	                                                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	/**
	 * AppIndicator:label-guide:
	 *
	 * An optional string to provide guidance to the panel on how big
	 * the #AppIndicator:label string could get.  If this is set correctly
	 * then the panel should never 'jiggle' as the string adjusts through
	 * out the range of options.  For instance, if you were providing a
	 * percentage like "54% thrust" in #AppIndicator:label you'd want to
	 * set this string to "100% thrust" to ensure space when Scotty can
	 * get you enough power.
	 */
	g_object_class_install_property(object_class,
	                                PROP_LABEL_GUIDE,
	                                g_param_spec_string (PROP_LABEL_GUIDE_S,
	                                                     "A string to size the space available for the label.",
	                                                     "To ensure that the label does not cause the panel to 'jiggle' this string should provide information on how much space it could take.",
	                                                     NULL,
	                                                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	/**
	 * AppIndicator:ordering-index:
	 *
	 * The ordering index is an odd parameter, and if you think you don't need
	 * it you're probably right.  In general, the application indicator try
	 * to place the applications in a recreatable place taking into account
	 * which category they're in to try and group them.  But, there are some
	 * cases where you'd want to ensure indicators are next to each other.
	 * To do that you can override the generated ordering index and replace it
	 * with a new one.  Again, you probably don't want to be doing this, but
	 * in case you do, this is the way.
	 */
	g_object_class_install_property(object_class,
	                                PROP_ORDERING_INDEX,
	                                g_param_spec_uint (PROP_ORDERING_INDEX_S,
	                                                   "The location that this app indicator should be in the list.",
	                                                   "A way to override the default ordering of the applications by providing a very specific idea of where this entry should be placed.",
	                                                   0, G_MAXUINT32, 0,
	                                                   G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/**
	 * AppIndicator:dbus-menu-server:
	 *
	 * A way to get the internal dbusmenu server if it is available.
	 * This should only be used for testing.
	 */
	g_object_class_install_property(object_class,
	                                PROP_DBUS_MENU_SERVER,
	                                g_param_spec_object (PROP_DBUS_MENU_SERVER_S,
	                                                     "The internal DBusmenu Server",
	                                                     "DBusmenu server which is available for testing the application indicators.",
	                                                     DBUSMENU_TYPE_SERVER,
	                                                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	/**
	 * AppIndicator:title:
	 *
	 * Provides a way to refer to this application indicator in a human
	 * readable form.  This is used in the Unity desktop in the HUD as
	 * the first part of the menu entries to distinguish them from the
	 * focused application's entries.
	 */
	g_object_class_install_property(object_class,
	                                PROP_TITLE,
	                                g_param_spec_string (PROP_TITLE_S,
	                                                     "Title of the application indicator",
	                                                     "A human readable way to refer to this application indicator in the UI.",
	                                                     NULL,
	                                                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/* Signals */

	/**
	 * AppIndicator::new-icon:
	 * @arg0: The #AppIndicator object
	 *
	 * when #AppIndicator:icon-name is changed
	 */
	signals[NEW_ICON] = g_signal_new (APP_INDICATOR_SIGNAL_NEW_ICON,
	                                  G_TYPE_FROM_CLASS(klass),
	                                  G_SIGNAL_RUN_LAST,
	                                  G_STRUCT_OFFSET (AppIndicatorClass, new_icon),
	                                  NULL, NULL,
	                                  g_cclosure_marshal_VOID__VOID,
	                                  G_TYPE_NONE, 0, G_TYPE_NONE);

	/**
	 * AppIndicator::new-attention-icon:
	 * @arg0: The #AppIndicator object
	 *
	 * Emitted when #AppIndicator:attention-icon-name is changed
	 */
	signals[NEW_ATTENTION_ICON] = g_signal_new (APP_INDICATOR_SIGNAL_NEW_ATTENTION_ICON,
	                                            G_TYPE_FROM_CLASS(klass),
	                                            G_SIGNAL_RUN_LAST,
	                                            G_STRUCT_OFFSET (AppIndicatorClass, new_attention_icon),
	                                            NULL, NULL,
	                                            g_cclosure_marshal_VOID__VOID,
	                                            G_TYPE_NONE, 0, G_TYPE_NONE);

	/**
	 * AppIndicator::new-status:
	 * @arg0: The #AppIndicator object
	 * @arg1: The string value of the #AppIndicatorStatus enum.
	 *
	 * Emitted when #AppIndicator:status is changed
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
	 * AppIndicator::new-label:
	 * @arg0: The #AppIndicator object
	 * @arg1: The string for the label
	 * @arg1: The string for the guide
	 *
	 * Emitted when either #AppIndicator:label or #AppIndicator:label-guide are
	 * changed.
	*/
	signals[NEW_LABEL] = g_signal_new (APP_INDICATOR_SIGNAL_NEW_LABEL,
	                                    G_TYPE_FROM_CLASS(klass),
	                                    G_SIGNAL_RUN_LAST,
	                                    G_STRUCT_OFFSET (AppIndicatorClass, new_label),
	                                    NULL, NULL,
	                                    _application_service_marshal_VOID__STRING_STRING,
	                                    G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

	/**
	 * AppIndicator::connection-changed:
	 * @arg0: The #AppIndicator object
	 * @arg1: Whether we're connected or not
	 *
	 * Signaled when we connect to a watcher, or when it drops away.
	 */
	signals[CONNECTION_CHANGED] = g_signal_new (APP_INDICATOR_SIGNAL_CONNECTION_CHANGED,
	                                            G_TYPE_FROM_CLASS(klass),
	                                            G_SIGNAL_RUN_LAST,
	                                            G_STRUCT_OFFSET (AppIndicatorClass, connection_changed),
	                                            NULL, NULL,
	                                            g_cclosure_marshal_VOID__BOOLEAN,
	                                            G_TYPE_NONE, 1, G_TYPE_BOOLEAN, G_TYPE_NONE);

	/**
	 * AppIndicator::new-icon-theme-path:
	 * @arg0: The #AppIndicator object
	 *
	 * Signaled when there is a new icon set for the
	 * object.
	 */
	signals[NEW_ICON_THEME_PATH] = g_signal_new (APP_INDICATOR_SIGNAL_NEW_ICON_THEME_PATH,
	                                  G_TYPE_FROM_CLASS(klass),
	                                  G_SIGNAL_RUN_LAST,
	                                  G_STRUCT_OFFSET (AppIndicatorClass, new_icon_theme_path),
	                                  NULL, NULL,
	                                  g_cclosure_marshal_VOID__STRING,
	                                  G_TYPE_NONE, 1, G_TYPE_STRING);

	/**
	 * AppIndicator::scroll-event:
	 * @arg0: The #AppIndicator object
	 * @arg1: How many steps the scroll wheel has taken
	 * @arg2: (type Gdk.ScrollDirection) Which direction the wheel went in
	 *
	 * Signaled when the #AppIndicator receives a scroll event.
	 */
	signals[SCROLL_EVENT] = g_signal_new (APP_INDICATOR_SIGNAL_SCROLL_EVENT,
	                                  G_TYPE_FROM_CLASS(klass),
	                                  G_SIGNAL_RUN_LAST,
	                                  G_STRUCT_OFFSET (AppIndicatorClass, scroll_event),
	                                  NULL, NULL,
	                                  _application_service_marshal_VOID__INT_UINT,
	                                  G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_UINT);

	/* DBus interfaces */
	if (item_node_info == NULL) {
		GError * error = NULL;

		item_node_info = g_dbus_node_info_new_for_xml(_notification_item, &error);
		if (error != NULL) {
			g_error("Unable to parse Notification Item DBus interface: %s", error->message);
			g_error_free(error);
		}
	}

	if (item_interface_info == NULL && item_node_info != NULL) {
		item_interface_info = g_dbus_node_info_lookup_interface(item_node_info, NOTIFICATION_ITEM_DBUS_IFACE);

		if (item_interface_info == NULL) {
			g_error("Unable to find interface '" NOTIFICATION_ITEM_DBUS_IFACE "'");
		}
	}

	if (watcher_node_info == NULL) {
		GError * error = NULL;

		watcher_node_info = g_dbus_node_info_new_for_xml(_notification_watcher, &error);
		if (error != NULL) {
			g_error("Unable to parse Notification Item DBus interface: %s", error->message);
			g_error_free(error);
		}
	}

	if (watcher_interface_info == NULL && watcher_node_info != NULL) {
		watcher_interface_info = g_dbus_node_info_lookup_interface(watcher_node_info, NOTIFICATION_WATCHER_DBUS_IFACE);

		if (watcher_interface_info == NULL) {
			g_error("Unable to find interface '" NOTIFICATION_WATCHER_DBUS_IFACE "'");
		}
	}

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
	priv->title = NULL;
	priv->label = NULL;
	priv->label_guide = NULL;
	priv->label_change_idle = 0;

	priv->watcher_proxy = NULL;
	priv->connection = NULL;
	priv->dbus_registration = 0;
	priv->path = NULL;

	priv->status_icon = NULL;
	priv->fallback_timer = 0;

	priv->shorties = NULL;

	priv->sec_activate_target = NULL;
	priv->sec_activate_enabled = FALSE;

	/* Start getting the session bus */
	g_object_ref(self); /* ref for the bus creation callback */
	g_bus_get(G_BUS_TYPE_SESSION, NULL, bus_creation, self);

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

	if (priv->shorties != NULL) {
		g_object_unref(G_OBJECT(priv->shorties));
		priv->shorties = NULL;
	}

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
		g_object_unref(G_OBJECT(priv->menu));
		priv->menu = NULL;
	}

	if (priv->menuservice != NULL) {
		g_object_unref (priv->menuservice);
	}

	if (priv->watcher_proxy != NULL) {
		g_signal_handlers_disconnect_by_func(G_OBJECT(priv->watcher_proxy), watcher_owner_changed, self);
		g_object_unref(G_OBJECT(priv->watcher_proxy));
		priv->watcher_proxy = NULL;

	    /* Emit the AppIndicator::connection-changed signal*/
        g_signal_emit (self, signals[CONNECTION_CHANGED], 0, FALSE);
	}

	if (priv->dbus_registration != 0) {
		g_dbus_connection_unregister_object(priv->connection, priv->dbus_registration);
		priv->dbus_registration = 0;
	}

	if (priv->connection != NULL) {
		g_object_unref(G_OBJECT(priv->connection));
		priv->connection = NULL;
	}

	if (priv->sec_activate_target != NULL) {
		g_signal_handlers_disconnect_by_func (priv->sec_activate_target, sec_activate_target_parent_changed, self);
		g_object_unref(G_OBJECT(priv->sec_activate_target));
		priv->sec_activate_target = NULL;
	}

	g_signal_handlers_disconnect_by_func(gtk_icon_theme_get_default(), G_CALLBACK(theme_changed_cb), self);

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
	
	if (priv->title != NULL) {
		g_free(priv->title);
		priv->title = NULL;
	}

	if (priv->label != NULL) {
		g_free(priv->label);
		priv->label = NULL;
	}

	if (priv->label_guide != NULL) {
		g_free(priv->label_guide);
		priv->label_guide = NULL;
	}

	if (priv->accessible_desc != NULL) {
		g_free(priv->accessible_desc);
		priv->accessible_desc = NULL;
	}

	if (priv->att_accessible_desc != NULL) {
		g_free(priv->att_accessible_desc);
		priv->att_accessible_desc = NULL;
	}

	if (priv->path != NULL) {
		g_free(priv->path);
		priv->path = NULL;
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
          app_indicator_set_icon_full (APP_INDICATOR (object),
                                       g_value_get_string (value),
                                       priv->accessible_desc);
          check_connect (self);
          break;

        case PROP_ICON_DESC:
          app_indicator_set_icon_full (APP_INDICATOR (object),
                                       priv->icon_name,
                                       g_value_get_string (value));
          check_connect (self);
          break;

        case PROP_ATTENTION_ICON_NAME:
          app_indicator_set_attention_icon_full (APP_INDICATOR (object),
                                                 g_value_get_string (value),
                                                 priv->att_accessible_desc);
          break;

        case PROP_ATTENTION_ICON_DESC:
          app_indicator_set_attention_icon_full (APP_INDICATOR (object),
                                                 priv->attention_icon_name,
                                                 g_value_get_string (value));
          break;

        case PROP_ICON_THEME_PATH:
          app_indicator_set_icon_theme_path (APP_INDICATOR (object),
                                            g_value_get_string (value));
          check_connect (self);
          break;

		case PROP_LABEL: {
		  gchar * oldlabel = priv->label;
		  priv->label = g_value_dup_string(value);

		  if (priv->label != NULL && priv->label[0] == '\0') {
		  	g_free(priv->label);
			priv->label = NULL;
		  }

		  if (g_strcmp0(oldlabel, priv->label) != 0) {
		    signal_label_change(APP_INDICATOR(object));
		  }

		  if (oldlabel != NULL) {
		  	g_free(oldlabel);
		  }
		  break;
		}
		case PROP_TITLE: {
		  gchar * oldtitle = priv->title;
		  priv->title = g_value_dup_string(value);

		  if (priv->title != NULL && priv->title[0] == '\0') {
		  	g_free(priv->title);
			priv->title = NULL;
		  }

		  if (g_strcmp0(oldtitle, priv->title) != 0 && self->priv->connection != NULL) {
			GError * error = NULL;

			g_dbus_connection_emit_signal(self->priv->connection,
										  NULL,
										  self->priv->path,
										  NOTIFICATION_ITEM_DBUS_IFACE,
										  "NewTitle",
										  NULL,
										  &error);

			if (error != NULL) {
				g_warning("Unable to send signal for NewTitle: %s", error->message);
				g_error_free(error);
			}
		  }

		  if (oldtitle != NULL) {
		  	g_free(oldtitle);
		  }

		  if (priv->status_icon != NULL) {
		  	gtk_status_icon_set_title(priv->status_icon, priv->title ? priv->title : "");
		  }
		  break;
		}
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
		case PROP_ORDERING_INDEX:
		  priv->ordering_index = g_value_get_uint(value);
		  break;

		case PROP_DBUS_MENU_SERVER:
			g_clear_object (&priv->menuservice);
			priv->menuservice = DBUSMENU_SERVER (g_value_dup_object(value));
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

        case PROP_ICON_DESC:
          g_value_set_string (value, priv->accessible_desc);
          break;

        case PROP_ATTENTION_ICON_NAME:
          g_value_set_string (value, priv->attention_icon_name);
          break;

        case PROP_ATTENTION_ICON_DESC:
          g_value_set_string (value, priv->att_accessible_desc);
          break;

        case PROP_ICON_THEME_PATH:
          g_value_set_string (value, priv->icon_theme_path);
          break;

		case PROP_CONNECTED: {
			gboolean connected = FALSE;

			if (priv->watcher_proxy != NULL) {
				gchar * name = g_dbus_proxy_get_name_owner(priv->watcher_proxy);
				if (name != NULL) {
					connected = TRUE;
					g_free(name);
				}
			}

			g_value_set_boolean (value, connected);
			break;
		}

        case PROP_LABEL:
          g_value_set_string (value, priv->label);
          break;

        case PROP_LABEL_GUIDE:
          g_value_set_string (value, priv->label_guide);
          break;

		case PROP_ORDERING_INDEX:
		  g_value_set_uint(value, priv->ordering_index);
		  break;

		case PROP_DBUS_MENU_SERVER:
			g_value_set_object(value, priv->menuservice);
			break;

		case PROP_TITLE:
			g_value_set_string(value, priv->title);
			break;

        default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
          break;
        }

	return;
}

/* DBus bus has been created, well maybe, but we got a call
   back about it so we need to check into it. */
static void
bus_creation (GObject * obj, GAsyncResult * res, gpointer user_data)
{
	GError * error = NULL;

	GDBusConnection * connection = g_bus_get_finish(res, &error);
	if (error != NULL) {
		g_warning("Unable to get the session bus: %s", error->message);
		g_error_free(error);
		g_object_unref(G_OBJECT(user_data));
		return;
	}

	AppIndicator * app = APP_INDICATOR(user_data);
	app->priv->connection = connection;

	/* If the connection was blocking the exporting of the
	   object this function will export everything. */
	check_connect(app);

	g_object_unref(G_OBJECT(app));

	return;
}

static void
bus_method_call (GDBusConnection * connection, const gchar * sender,
                 const gchar * path, const gchar * interface,
                 const gchar * method, GVariant * params,
                 GDBusMethodInvocation * invocation, gpointer user_data)
{
	g_return_if_fail(IS_APP_INDICATOR(user_data));

	AppIndicator * app = APP_INDICATOR(user_data);
	AppIndicatorPrivate * priv = app->priv;
	GVariant * retval = NULL;

	if (g_strcmp0(method, "Scroll") == 0) {
		guint direction;
		gint delta;
		const gchar *orientation;

		g_variant_get(params, "(i&s)", &delta, &orientation);

		if (g_strcmp0(orientation, "horizontal") == 0) {
			direction = (delta >= 0) ? GDK_SCROLL_RIGHT : GDK_SCROLL_LEFT;
		} else if (g_strcmp0(orientation, "vertical") == 0) {
			direction = (delta >= 0) ? GDK_SCROLL_DOWN : GDK_SCROLL_UP;
		} else {
			g_dbus_method_invocation_return_value(invocation, retval);
			return;
		}

		delta = ABS(delta);
		g_signal_emit(app, signals[SCROLL_EVENT], 0, delta, direction);

	} else if (g_strcmp0(method, "SecondaryActivate") == 0 ||
	           g_strcmp0(method, "XAyatanaSecondaryActivate") == 0) {
		GtkWidget *menuitem = priv->sec_activate_target;
		
		if (priv->sec_activate_enabled && menuitem &&
		    gtk_widget_get_visible (menuitem) &&
		    gtk_widget_get_sensitive (menuitem))
		{
			gtk_widget_activate (menuitem);
		}
	} else {
		g_warning("Calling method '%s' on the app-indicator and it's unknown", method);
	}

	g_dbus_method_invocation_return_value(invocation, retval);
}

/* DBus is asking for a property so we should figure out what it
   wants and try and deliver. */
static GVariant *
bus_get_prop (GDBusConnection * connection, const gchar * sender, const gchar * path, const gchar * interface, const gchar * property, GError ** error, gpointer user_data)
{
	g_return_val_if_fail(IS_APP_INDICATOR(user_data), NULL);
	AppIndicator * app = APP_INDICATOR(user_data);
	AppIndicatorPrivate *priv = app->priv;

	if (g_strcmp0(property, "Id") == 0) {
		return g_variant_new_string(app->priv->id ? app->priv->id : "");
	} else if (g_strcmp0(property, "Category") == 0) {
        GEnumValue *enum_value;
		enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_CATEGORY), priv->category);
		return g_variant_new_string(enum_value->value_nick ? enum_value->value_nick : "");
	} else if (g_strcmp0(property, "Status") == 0) {
        GEnumValue *enum_value;
		enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_STATUS), priv->status);
		return g_variant_new_string(enum_value->value_nick ? enum_value->value_nick : "");
	} else if (g_strcmp0(property, "IconName") == 0) {
		return g_variant_new_string(priv->icon_name ? priv->icon_name : "");
	} else if (g_strcmp0(property, "AttentionIconName") == 0) {
		return g_variant_new_string(priv->attention_icon_name ? priv->attention_icon_name : "");
	} else if (g_strcmp0(property, "Title") == 0) {
		const gchar * output = NULL;
		if (priv->title == NULL) {
			const gchar * name = g_get_application_name();
			if (name != NULL) {
				output = name;
			} else {
				output = "";
			}
		} else {
			output = priv->title;
		}
		return g_variant_new_string(output);
	} else if (g_strcmp0(property, "IconThemePath") == 0) {
		return g_variant_new_string(priv->icon_theme_path ? priv->icon_theme_path : "");
	} else if (g_strcmp0(property, "Menu") == 0) {
		if (priv->menuservice != NULL) {
			GValue strval = { 0 };
			g_value_init(&strval, G_TYPE_STRING);
			g_object_get_property (G_OBJECT (priv->menuservice), DBUSMENU_SERVER_PROP_DBUS_OBJECT, &strval);
			GVariant * var = g_variant_new("o", g_value_get_string(&strval));
			g_value_unset(&strval);
			return var;
		} else {
			return g_variant_new("o", "/");
		}
	} else if (g_strcmp0(property, "XAyatanaLabel") == 0) {
		return g_variant_new_string(priv->label ? priv->label : "");
	} else if (g_strcmp0(property, "XAyatanaLabelGuide") == 0) {
		return g_variant_new_string(priv->label_guide ? priv->label_guide : "");
	} else if (g_strcmp0(property, "XAyatanaOrderingIndex") == 0) {
		return g_variant_new_uint32(priv->ordering_index);
	} else if (g_strcmp0(property, "IconAccessibleDesc") == 0) {
		return g_variant_new_string(priv->accessible_desc ? priv->accessible_desc : "");
	} else if (g_strcmp0(property, "AttentionAccessibleDesc") == 0) {
		return g_variant_new_string(priv->att_accessible_desc ? priv->att_accessible_desc : "");
	}

	*error = g_error_new(0, 0, "Unknown property: %s", property);
	return NULL;
}

/* Sends the label changed signal and resets the source ID */
static gboolean
signal_label_change_idle (gpointer user_data)
{
	AppIndicator * self = (AppIndicator *)user_data;
	AppIndicatorPrivate *priv = self->priv;

	gchar * label = priv->label != NULL ? priv->label : "";
	gchar * guide = priv->label_guide != NULL ? priv->label_guide : "";

	g_signal_emit(G_OBJECT(self), signals[NEW_LABEL], 0,
	              label, guide, TRUE);
	if (priv->dbus_registration != 0 && priv->connection != NULL) {
		GError * error = NULL;

		g_dbus_connection_emit_signal(priv->connection,
		                              NULL,
		                              priv->path,
		                              NOTIFICATION_ITEM_DBUS_IFACE,
		                              "XAyatanaNewLabel",
		                              g_variant_new("(ss)", label, guide),
		                              &error);

		if (error != NULL) {
			g_warning("Unable to send signal for NewIcon: %s", error->message);
			g_error_free(error);
		}
	}

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

	/* Do we have a connection? */
	if (priv->connection == NULL) return;

	/* If we already have a proxy, let's see if it has someone
	   implementing it.  If not, we can't do much more than to
	   do nothing. */
	if (priv->watcher_proxy != NULL) {
		gchar * name = g_dbus_proxy_get_name_owner(priv->watcher_proxy);
		if (name == NULL) {
			return;
		}
		g_free(name);
	}

	/* Do we have enough information? */
	if (priv->menu == NULL) return;
	if (priv->icon_name == NULL) return;
	if (priv->id == NULL) return;

	if (priv->path == NULL) {
		priv->path = g_strdup_printf(DEFAULT_ITEM_PATH "/%s", priv->clean_id);
	}

	if (priv->dbus_registration == 0) {
		GError * error = NULL;
		priv->dbus_registration = g_dbus_connection_register_object(priv->connection,
		                                                            priv->path,
		                                                            item_interface_info,
		                                                            &item_interface_table,
		                                                            self,
		                                                            NULL,
		                                                            &error);
		if (error != NULL) {
			g_warning("Unable to register object on path '%s': %s", priv->path, error->message);
			g_error_free(error);
			return;
		}
	}

	/* NOTE: It's really important the order here.  We make sure to *publish*
	   the object on the bus and *then* get the proxy.  The reason is that we
	   want to ensure all the filters are setup before talking to the watcher
	   and that's where the order is important. */

	g_object_ref(G_OBJECT(self)); /* Unref in watcher_ready() */
	if (priv->watcher_proxy == NULL) {
		/* Build Watcher Proxy */
		g_dbus_proxy_new(priv->connection,
		                 G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES|
		                 G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS, /* We don't use these, don't bother with them */
		                 watcher_interface_info,
		                 NOTIFICATION_WATCHER_DBUS_ADDR,
		                 NOTIFICATION_WATCHER_DBUS_OBJ,
		                 NOTIFICATION_WATCHER_DBUS_IFACE,
		                 NULL, /* cancellable */
		                 bus_watcher_ready,
		                 self);
	} else {
		bus_watcher_ready(NULL, NULL, self);
	}

	return;
}

/* Callback for when the watcher proxy has been created, or not
   but we got called none-the-less. */
static void
bus_watcher_ready (GObject * obj, GAsyncResult * res, gpointer user_data)
{
	GError * error = NULL;

	GDBusProxy * proxy = NULL;
	if (res != NULL) {
		proxy = g_dbus_proxy_new_finish(res, &error);
	}

	if (error != NULL) {
		/* Unable to get proxy, but we're handling that now so
		   it's not a warning anymore. */
		g_error_free(error);

		if (IS_APP_INDICATOR(user_data)) {
			start_fallback_timer(APP_INDICATOR(user_data), FALSE);
		}

		g_object_unref(G_OBJECT(user_data));
		return;
	}

	AppIndicator * app = APP_INDICATOR(user_data);

	if (res != NULL) {
		app->priv->watcher_proxy = proxy;

		/* Setting up a signal to watch when the unique name
		   changes */
		g_signal_connect(G_OBJECT(app->priv->watcher_proxy), "notify::g-name-owner", G_CALLBACK(watcher_owner_changed), user_data);
	}

	/* Let's insure that someone is on the other side, else we're
	   still in a fallback scenario. */
	gchar * name = g_dbus_proxy_get_name_owner(app->priv->watcher_proxy);
	if (name == NULL) {
		start_fallback_timer(APP_INDICATOR(user_data), FALSE);
		g_object_unref(G_OBJECT(user_data));
		return;
	}
	g_free(name);

	/* g_object_unref(G_OBJECT(user_data)); */
	/* Why is this commented out?  Oh, wait, we don't want to
	   unref in this case because we need to ref again to do the
	   register callback.  Let's not unref to ref again. */

	g_dbus_proxy_call(app->priv->watcher_proxy,
	                  "RegisterStatusNotifierItem",
	                  g_variant_new("(s)", app->priv->path),
	                  G_DBUS_CALL_FLAGS_NONE,
					  -1,
	                  NULL, /* cancelable */
	                  register_service_cb,
	                  user_data);

	return;
}

/* Watching for when the name owner changes on the interface
   to know whether we should be connected or not. */
static void
watcher_owner_changed (GObject * obj, GParamSpec * pspec, gpointer user_data)
{
	AppIndicator * self = APP_INDICATOR(user_data);
	g_return_if_fail(self != NULL);
	g_return_if_fail(self->priv->watcher_proxy != NULL);

	gchar * name = g_dbus_proxy_get_name_owner(self->priv->watcher_proxy);

	if (name == NULL) {
		/* Emit the AppIndicator::connection-changed signal*/
		g_signal_emit (self, signals[CONNECTION_CHANGED], 0, FALSE);
		
		start_fallback_timer(self, FALSE);
	} else {
		if (self->priv->fallback_timer != 0) {
			/* Stop the timer */
			g_source_remove(self->priv->fallback_timer);
			self->priv->fallback_timer = 0;
		}

		check_connect(self);
	}

	return;
}

/* Responce from the DBus command to register a service
   with a NotificationWatcher. */
static void
register_service_cb (GObject * obj, GAsyncResult * res, gpointer user_data)
{
	GError * error = NULL;
	GVariant * returns = g_dbus_proxy_call_finish(G_DBUS_PROXY(obj), res, &error);

	/* We don't care about any return values */
	if (returns != NULL) {
		g_variant_unref(returns);
	}

	if (error != NULL) {
		/* They didn't respond, ewww.  Not sure what they could
		   be doing */
		g_warning("Unable to connect to the Notification Watcher: %s", error->message);
		start_fallback_timer(APP_INDICATOR(user_data), TRUE);
		g_object_unref(G_OBJECT(user_data));
		return;
	}

	g_return_if_fail(IS_APP_INDICATOR(user_data));
	AppIndicator * app = APP_INDICATOR(user_data);
	AppIndicatorPrivate * priv = app->priv;

	/* Emit the AppIndicator::connection-changed signal*/
    g_signal_emit (app, signals[CONNECTION_CHANGED], 0, TRUE);

	if (priv->status_icon) {
		AppIndicatorClass * class = APP_INDICATOR_GET_CLASS(app);
		if (class->unfallback != NULL) {
			class->unfallback(app, priv->status_icon);
			priv->status_icon = NULL;
		} 
	}

	g_object_unref(G_OBJECT(user_data));
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

	AppIndicator * self = (AppIndicator *)user_data;
	AppIndicatorPrivate *priv = self->priv;

	if (priv->dbus_registration != 0 && priv->connection != NULL) {
		GError * error = NULL;

		g_dbus_connection_emit_signal(priv->connection,
		                              NULL,
		                              priv->path,
		                              NOTIFICATION_ITEM_DBUS_IFACE,
		                              "NewIcon",
		                              NULL,
		                              &error);

		if (error != NULL) {
			g_warning("Unable to send signal for NewIcon: %s", error->message);
			g_error_free(error);
		}
	}

	return;
}

/* Creates a StatusIcon that can be used when the application
   indicator area isn't available. */
static GtkStatusIcon *
fallback (AppIndicator * self)
{
	GtkStatusIcon * icon = gtk_status_icon_new();

	gtk_status_icon_set_name(icon, app_indicator_get_id(self));
	const gchar * title = app_indicator_get_title(self);
	if (title != NULL) {
		gtk_status_icon_set_title(icon, title);
	}
	
	g_signal_connect(G_OBJECT(self), APP_INDICATOR_SIGNAL_NEW_STATUS,
		G_CALLBACK(status_icon_status_wrapper), icon);
	g_signal_connect(G_OBJECT(self), APP_INDICATOR_SIGNAL_NEW_ICON,
		G_CALLBACK(status_icon_changes), icon);
	g_signal_connect(G_OBJECT(self), APP_INDICATOR_SIGNAL_NEW_ATTENTION_ICON,
		G_CALLBACK(status_icon_changes), icon);

	status_icon_changes(self, icon);

	g_signal_connect(G_OBJECT(icon), "activate", G_CALLBACK(status_icon_activate), self);
	g_signal_connect(G_OBJECT(icon), "popup-menu", G_CALLBACK(status_icon_menu_activate), self);
	g_signal_connect(G_OBJECT(icon), "scroll-event", G_CALLBACK(scroll_event_wrapper), self);
	g_signal_connect(G_OBJECT(icon), "button-release-event", G_CALLBACK(middle_click_wrapper), self);

	return icon;
}

/* A wrapper as the status update prototype is a little
   bit different, but we want to handle it the same. */
static void
status_icon_status_wrapper (AppIndicator * self, const gchar * status, gpointer data)
{
	return status_icon_changes(self, data);
}

/* A wrapper for redirecting the scroll events to the app-indicator from status
   icon widget. */
static gboolean
scroll_event_wrapper (GtkWidget *status_icon, GdkEventScroll *event, gpointer data)
{
	g_return_val_if_fail(IS_APP_INDICATOR(data), FALSE);
	AppIndicator * app = APP_INDICATOR(data);
	g_signal_emit(app, signals[SCROLL_EVENT], 0, 1, event->direction);

	return TRUE;
}

static gboolean
middle_click_wrapper (GtkWidget *status_icon, GdkEventButton *event, gpointer data)
{
	g_return_val_if_fail(IS_APP_INDICATOR(data), FALSE);
	AppIndicator * app = APP_INDICATOR(data);
	AppIndicatorPrivate *priv = app->priv;

	if (event->button == 2 && event->type == GDK_BUTTON_RELEASE) {
		GtkAllocation alloc;
		gint px = event->x;
		gint py = event->y;
		gtk_widget_get_allocation (status_icon, &alloc);
		GtkWidget *menuitem = priv->sec_activate_target;

		if (px >= 0 && px < alloc.width && py >= 0 && py < alloc.height &&
		    priv->sec_activate_enabled && menuitem &&
		    gtk_widget_get_visible (menuitem) &&
		    gtk_widget_get_sensitive (menuitem))
		{
			gtk_widget_activate (menuitem);
			return TRUE;
		}
	}

	return FALSE;
}

/* This tracks changes to either the status or the icons
   that are associated with the app indicator */
static void
status_icon_changes (AppIndicator * self, gpointer data)
{
	GtkStatusIcon * icon = GTK_STATUS_ICON(data);

	/* add the icon_theme_path once if needed */
	GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
	if (self->priv->icon_theme_path != NULL) {
		gchar **path;
		gint n_elements, i;
		gboolean found=FALSE;
		gtk_icon_theme_get_search_path(icon_theme, &path, &n_elements);
		for (i=0; i< n_elements || path[i] == NULL; i++) {
			if(g_strcmp0(path[i], self->priv->icon_theme_path) == 0) {
				found=TRUE;
				break;
			}
		}
		if(!found) {
			gtk_icon_theme_append_search_path(icon_theme, self->priv->icon_theme_path);
		}
		g_strfreev (path);
	}

	const gchar * icon_name = NULL;
	switch (app_indicator_get_status(self)) {
	case APP_INDICATOR_STATUS_PASSIVE:
		/* hide first to avoid that the change is visible to the user */
		gtk_status_icon_set_visible(icon, FALSE);
		icon_name = app_indicator_get_icon(self);
		break;
	case APP_INDICATOR_STATUS_ACTIVE:
		icon_name = app_indicator_get_icon(self);
		gtk_status_icon_set_visible(icon, TRUE);
		break;
	case APP_INDICATOR_STATUS_ATTENTION:
		/* get the _attention_ icon here */
		icon_name = app_indicator_get_attention_icon(self);
		gtk_status_icon_set_visible(icon, TRUE);
		break;
	};

	if (icon_name != NULL) {
		if (g_file_test(icon_name, G_FILE_TEST_EXISTS)) {
			gtk_status_icon_set_from_file(icon, icon_name);
		} else {
			gchar *longname = append_panel_icon_suffix(icon_name);

			if (longname != NULL && gtk_icon_theme_has_icon (icon_theme, longname)) {
				gtk_status_icon_set_from_icon_name(icon, longname);
			} else {
				gtk_status_icon_set_from_icon_name(icon, icon_name);
			}

			g_free(longname);
		}
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

/* Handles the right-click action by the status icon by showing
   the menu in a popup. */
static void
status_icon_menu_activate (GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data)
{
	status_icon_activate(status_icon, user_data);
}

/* Removes the status icon as the application indicator area
   is now up and running again. */
static void
unfallback (AppIndicator * self, GtkStatusIcon * status_icon)
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(self), status_icon_status_wrapper, status_icon);
	g_signal_handlers_disconnect_by_func(G_OBJECT(self), status_icon_changes, status_icon);
	g_signal_handlers_disconnect_by_func(G_OBJECT(self), scroll_event_wrapper, status_icon);
	g_signal_handlers_disconnect_by_func(G_OBJECT(self), middle_click_wrapper, status_icon);
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

static gboolean
widget_is_menu_child(AppIndicator * self, GtkWidget *child)
{
	g_return_val_if_fail(IS_APP_INDICATOR(self), FALSE);

	if (!self->priv->menu) return FALSE;
	if (!child) return FALSE;

	GtkWidget *parent;

	while ((parent = gtk_widget_get_parent(child))) {
		if (parent == self->priv->menu)
			return TRUE;

		if (GTK_IS_MENU(parent))
			child = gtk_menu_get_attach_widget(GTK_MENU(parent));
		else
			child = parent;
	}

	return FALSE;
}

static void
sec_activate_target_parent_changed(GtkWidget *menuitem, GtkWidget *old_parent,
                                   gpointer data)
{
	g_return_if_fail(IS_APP_INDICATOR(data));
	AppIndicator *self = data;
	self->priv->sec_activate_enabled = widget_is_menu_child(self, menuitem);
}


/* ************************* */
/*    Public Functions       */
/* ************************* */

/**
 * app_indicator_new:
 * @id: The unique id of the indicator to create.
 * @icon_name: The icon name for this indicator
 * @category: The category of indicator.
 *
 * Creates a new #AppIndicator setting the properties:
 * #AppIndicator:id with @id, #AppIndicator:category with @category
 * and #AppIndicator:icon-name with @icon_name.
 * 
 * Return value: A pointer to a new #AppIndicator object.
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
 * app_indicator_new_with_path:
 * @id: The unique id of the indicator to create.
 * @icon_name: The icon name for this indicator
 * @category: The category of indicator.
 * @icon_theme_path: A custom path for finding icons.

 * Creates a new #AppIndicator setting the properties:
 * #AppIndicator:id with @id, #AppIndicator:category with @category,
 * #AppIndicator:icon-name with @icon_name and #AppIndicator:icon-theme-path
 * with @icon_theme_path.
 *
 * Return value: A pointer to a new #AppIndicator object.
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
 * app_indicator_get_type:
 *
 * Generates or returns the unique #GType for #AppIndicator.
 *
 * Return value: A unique #GType for #AppIndicator objects.
 */

/**
 * app_indicator_set_status:
 * @self: The #AppIndicator object to use
 * @status: The status to set for this indicator
 *
 * Wrapper function for property #AppIndicator:status.
 */
void
app_indicator_set_status (AppIndicator *self, AppIndicatorStatus status)
{
	g_return_if_fail (IS_APP_INDICATOR (self));

	if (self->priv->status != status) {
		GEnumValue *value = g_enum_get_value ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_STATUS), status);

		self->priv->status = status;
		g_signal_emit (self, signals[NEW_STATUS], 0, value->value_nick);

		if (self->priv->dbus_registration != 0 && self->priv->connection != NULL) {
			GError * error = NULL;

			g_dbus_connection_emit_signal(self->priv->connection,
										  NULL,
										  self->priv->path,
										  NOTIFICATION_ITEM_DBUS_IFACE,
										  "NewStatus",
										  g_variant_new("(s)", value->value_nick),
										  &error);

			if (error != NULL) {
				g_warning("Unable to send signal for NewStatus: %s", error->message);
				g_error_free(error);
			}
		}
	}

	return;
}

/**
 * app_indicator_set_attention_icon:
 * @self: The #AppIndicator object to use
 * @icon_name: The name of the attention icon to set for this indicator
 *
 * Wrapper for app_indicator_set_attention_icon_full() with a NULL
 * description.
 *
 * Deprecated: Use app_indicator_set_attention_icon_full() instead.
 */
void
app_indicator_set_attention_icon (AppIndicator *self, const gchar *icon_name)
{
	return app_indicator_set_attention_icon_full(self, icon_name, NULL);
}

/**
 * app_indicator_set_attention_icon_full:
 * @self: The #AppIndicator object to use
 * @icon_name: The name of the attention icon to set for this indicator
 * @icon_desc: A textual description of the icon
 *
 * Wrapper function for property #AppIndicator:attention-icon-name.
 */
void
app_indicator_set_attention_icon_full (AppIndicator *self, const gchar *icon_name, const gchar * icon_desc)
{
	g_return_if_fail (IS_APP_INDICATOR (self));
	g_return_if_fail (icon_name != NULL);
	gboolean changed = FALSE;

	if (g_strcmp0 (self->priv->attention_icon_name, icon_name) != 0) {
		if (self->priv->attention_icon_name) {
			g_free (self->priv->attention_icon_name);
		}

		self->priv->attention_icon_name = g_strdup(icon_name);
		changed = TRUE;
	}

	if (g_strcmp0(self->priv->att_accessible_desc, icon_desc) != 0) {
		if (self->priv->att_accessible_desc) {
			g_free (self->priv->att_accessible_desc);
		}

		self->priv->att_accessible_desc = g_strdup(icon_name);
		changed = TRUE;
	}

	if (changed) {
		g_signal_emit (self, signals[NEW_ATTENTION_ICON], 0, TRUE);

		if (self->priv->dbus_registration != 0 && self->priv->connection != NULL) {
			GError * error = NULL;

			g_dbus_connection_emit_signal(self->priv->connection,
										  NULL,
										  self->priv->path,
										  NOTIFICATION_ITEM_DBUS_IFACE,
										  "NewAttentionIcon",
										  NULL,
										  &error);

			if (error != NULL) {
				g_warning("Unable to send signal for NewAttentionIcon: %s", error->message);
				g_error_free(error);
			}
		}
	}

	return;
}

/**
 * app_indicator_set_icon:
 * @self: The #AppIndicator object to use
 * @icon_name: The icon name to set.
 *
 * Wrapper function for app_indicator_set_icon_full() with a NULL
 * description.
 *
 * Deprecated: Use app_indicator_set_icon_full()
 */
void
app_indicator_set_icon (AppIndicator *self, const gchar *icon_name)
{
	return app_indicator_set_icon_full(self, icon_name, NULL);
}

/**
 * app_indicator_set_icon_full:
 * @self: The #AppIndicator object to use
 * @icon_name: The icon name to set.
 * @icon_desc: A textual description of the icon for accessibility
 *
 * Sets the default icon to use when the status is active but
 * not set to attention.  In most cases, this should be the
 * application icon for the program.
 *
 * Wrapper function for property #AppIndicator:icon-name and
 * #AppIndicator::icon-desc.
 */
void
app_indicator_set_icon_full (AppIndicator *self, const gchar *icon_name, const gchar * icon_desc)
{
	g_return_if_fail (IS_APP_INDICATOR (self));
	g_return_if_fail (icon_name != NULL);
	gboolean changed = FALSE;

	if (g_strcmp0 (self->priv->icon_name, icon_name) != 0) {
		if (self->priv->icon_name) {
			g_free (self->priv->icon_name);
		}

		self->priv->icon_name = g_strdup(icon_name);
		changed = TRUE;
	}

	if (g_strcmp0(self->priv->accessible_desc, icon_desc) != 0) {
		if (self->priv->accessible_desc != NULL) {
			g_free(self->priv->accessible_desc);
		}

		self->priv->accessible_desc = g_strdup(icon_desc);
		changed = TRUE;
	}

	if (changed) {
		g_signal_emit (self, signals[NEW_ICON], 0, TRUE);

		if (self->priv->dbus_registration != 0 && self->priv->connection != NULL) {
			GError * error = NULL;

			g_dbus_connection_emit_signal(self->priv->connection,
										  NULL,
										  self->priv->path,
										  NOTIFICATION_ITEM_DBUS_IFACE,
										  "NewIcon",
										  NULL,
										  &error);

			if (error != NULL) {
				g_warning("Unable to send signal for NewIcon: %s", error->message);
				g_error_free(error);
			}
		}
	}

	return;
}

/**
 * app_indicator_set_label:
 * @self: The #AppIndicator object to use
 * @label: The label to show next to the icon.
 * @guide: A guide to size the label correctly.
 *
 * This is a wrapper function for the #AppIndicator:label and
 * #AppIndicator:guide properties.  This function can take #NULL
 * as either @label or @guide and will clear the entries.
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
 * app_indicator_set_icon_theme_path:
 * @self: The #AppIndicator object to use
 * @icon_theme_path: The icon theme path to set.
 *
 * Sets the path to use when searching for icons.
 */
void
app_indicator_set_icon_theme_path (AppIndicator *self, const gchar *icon_theme_path)
{
	g_return_if_fail (IS_APP_INDICATOR (self));

	if (g_strcmp0 (self->priv->icon_theme_path, icon_theme_path) != 0) {
		if (self->priv->icon_theme_path != NULL)
			g_free(self->priv->icon_theme_path);

		self->priv->icon_theme_path = g_strdup(icon_theme_path);

		g_signal_emit (self, signals[NEW_ICON_THEME_PATH], 0, self->priv->icon_theme_path, TRUE);

		if (self->priv->dbus_registration != 0 && self->priv->connection != NULL) {
			GError * error = NULL;

			g_dbus_connection_emit_signal(self->priv->connection,
										  NULL,
										  self->priv->path,
										  NOTIFICATION_ITEM_DBUS_IFACE,
										  "NewIconThemePath",
										  g_variant_new("(s)", self->priv->icon_theme_path),
										  &error);

			if (error != NULL) {
				g_warning("Unable to send signal for NewIconThemePath: %s", error->message);
				g_error_free(error);
			}
		}
	}

	return;
}

/* Does the dbusmenu related work.  If there isn't a server, it builds
   one and if there are menus it runs the parse to put those menus into
   the server. */
static void
setup_dbusmenu (AppIndicator *self)
{
	AppIndicatorPrivate *priv;
	DbusmenuMenuitem *root = NULL;

	priv = self->priv;

	if (priv->menu) {
		root = dbusmenu_gtk_parse_menu_structure(priv->menu);
	}

	if (priv->menuservice == NULL) {
		gchar * path = g_strdup_printf(DEFAULT_ITEM_PATH "/%s/Menu", priv->clean_id);
		priv->menuservice = dbusmenu_server_new (path);
		g_free(path);
	}

	dbusmenu_server_set_root (priv->menuservice, root);

	/* Drop our local ref as set_root should get it's own. */
	if (root != NULL) {
		g_object_unref(root);
	}

	return;
}

/**
 * app_indicator_set_menu:
 * @self: The #AppIndicator
 * @menu: (allow-none): A #GtkMenu to set
 *
 * Sets the menu that should be shown when the Application Indicator
 * is clicked on in the panel.  An application indicator will not
 * be rendered unless it has a menu.
 *
 * Wrapper function for property #AppIndicator:menu.
 */
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
  g_object_ref_sink (priv->menu);

  setup_dbusmenu (self);

  priv->sec_activate_enabled = widget_is_menu_child (self, priv->sec_activate_target);

  check_connect (self);

  return;
}

/**
 * app_indicator_set_ordering_index:
 * @self: The #AppIndicator
 * @ordering_index: A value for the ordering of this app indicator
 *
 * Sets the ordering index for the app indicator which effects the
 * placement of it on the panel.  For almost all app indicator
 * this is not the function you're looking for.
 *
 * Wrapper function for property #AppIndicator:ordering-index.
 */
void
app_indicator_set_ordering_index (AppIndicator *self, guint32 ordering_index)
{
	g_return_if_fail (IS_APP_INDICATOR (self));

	self->priv->ordering_index = ordering_index;

	return;
}

/**
 * app_indicator_set_secondary_activate_target:
 * @self: The #AppIndicator
 * @menuitem: (allow-none): A #GtkWidget to be activated on secondary activation
 *
 * Set the @menuitem to be activated when a secondary activation event (i.e. a
 * middle-click) is emitted over the #AppIndicator icon/label.
 *
 * The @menuitem can be also a complex #GtkWidget, but to get activated when
 * a secondary activation occurs in the #Appindicator, it must be a visible and
 * active child (or inner-child) of the #AppIndicator:menu.
 *
 * Setting @menuitem to %NULL causes to disable this feature.
 */
void
app_indicator_set_secondary_activate_target (AppIndicator *self, GtkWidget *menuitem)
{
	g_return_if_fail (IS_APP_INDICATOR (self));
	AppIndicatorPrivate *priv = self->priv;

	if (priv->sec_activate_target) {
		g_signal_handlers_disconnect_by_func (priv->sec_activate_target,
		                                      sec_activate_target_parent_changed,
		                                      self);
		g_object_unref(G_OBJECT(priv->sec_activate_target));
		priv->sec_activate_target = NULL;
	}

	if (menuitem == NULL) {
		return;
	}

	g_return_if_fail (GTK_IS_WIDGET (menuitem));

	priv->sec_activate_target = g_object_ref(G_OBJECT(menuitem));
	priv->sec_activate_enabled = widget_is_menu_child(self, menuitem);
	g_signal_connect(menuitem, "parent-set", G_CALLBACK(sec_activate_target_parent_changed), self);
}

/**
 * app_indicator_set_title:
 * @self: The #AppIndicator
 * @title: (allow-none): Title of the app indicator
 *
 * Sets the title of the application indicator, or how it should be referred
 * in a human readable form.  This string should be UTF-8 and localized as it
 * expected that users will set it.
 *
 * In the Unity desktop the most prominent place that this is show will be
 * in the HUD.  HUD listings for this application indicator will start with
 * the title as the first part of the line for the menu items.
 *
 * Setting @title to %NULL removes the title.
 *
 * Since: 0.5
 *
 */
void
app_indicator_set_title (AppIndicator *self, const gchar * title)
{
	g_return_if_fail (IS_APP_INDICATOR (self));

	g_object_set(G_OBJECT(self),
	             PROP_TITLE_S, title == NULL ? "": title,
	             NULL);

	return;
}

/**
 * app_indicator_get_id:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:id.
 *
 * Return value: The current ID
 */
const gchar *
app_indicator_get_id (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->id;
}

/**
 * app_indicator_get_category:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:category.
 *
 * Return value: The current category.
 */
AppIndicatorCategory
app_indicator_get_category (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

  return self->priv->category;
}

/**
 * app_indicator_get_status:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:status.
 *
 * Return value: The current status.
 */
AppIndicatorStatus
app_indicator_get_status (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), APP_INDICATOR_STATUS_PASSIVE);

  return self->priv->status;
}

/**
 * app_indicator_get_icon:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:icon-name.
 *
 * Return value: The current icon name.
 */
const gchar *
app_indicator_get_icon (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->icon_name;
}

/**
 * app_indicator_get_icon_desc:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:icon-desc.
 *
 * Return value: The current icon description.
*/
const gchar *
app_indicator_get_icon_desc (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->accessible_desc;
}

/**
 * app_indicator_get_icon_theme_path:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:icon-theme-path.
 *
 * Return value: The current icon theme path.
 */
const gchar *
app_indicator_get_icon_theme_path (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->icon_theme_path;
}

/**
 * app_indicator_get_attention_icon:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:attention-icon-name.
 *
 * Return value: The current attention icon name.
 */
const gchar *
app_indicator_get_attention_icon (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->attention_icon_name;
}

/**
 * app_indicator_get_attention_icon_desc:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:attention-icon-desc.
 *
 * Return value: The current attention icon description.
 */
const gchar *
app_indicator_get_attention_icon_desc (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->att_accessible_desc;
}

/**
 * app_indicator_get_title:
 * @self: The #AppIndicator object to use
 *
 * Gets the title of the application indicator.  See the function
 * app_indicator_set_title() for information on the title.
 *
 * Return value: The current title.
 *
 * Since: 0.5
 *
 */
const gchar *
app_indicator_get_title (AppIndicator *self)
{
	g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

	return self->priv->title;
}


/**
 * app_indicator_get_menu:
 * @self: The #AppIndicator object to use
 *
 * Gets the menu being used for this application indicator.
 * Wrapper function for property #AppIndicator:menu.
 *
 * Returns: (transfer none): A #GtkMenu object or %NULL if one hasn't been set.
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
 * app_indicator_get_label:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:label.
 *
 * Return value: The current label.
 */
const gchar *
app_indicator_get_label (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->label;
}

/**
 * app_indicator_get_label_guide:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:label-guide.
 *
 * Return value: The current label guide.
 */
const gchar *
app_indicator_get_label_guide (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->label_guide;
}

/**
 * app_indicator_get_ordering_index:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:ordering-index.
 *
 * Return value: The current ordering index.
 */
guint32
app_indicator_get_ordering_index (AppIndicator *self)
{
	g_return_val_if_fail (IS_APP_INDICATOR (self), 0);

	if (self->priv->ordering_index == 0) {
		return _generate_id(self->priv->category, self->priv->id);
	} else {
		return self->priv->ordering_index;
	}
}

/**
 * app_indicator_get_secondary_activate_target:
 * @self: The #AppIndicator object to use
 *
 * Gets the menuitem being called on secondary-activate event.
 *
 * Returns: (transfer none): A #GtkWidget object or %NULL if none has been set.
 */
GtkWidget *
app_indicator_get_secondary_activate_target (AppIndicator *self)
{
	g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

	return GTK_WIDGET(self->priv->sec_activate_target);
}

#define APP_INDICATOR_SHORTY_NICK "app-indicator-shorty-nick"

/* Callback when an item from the desktop shortcuts gets
   called. */
static void
shorty_activated_cb (DbusmenuMenuitem * mi, guint timestamp, gpointer user_data)
{
	gchar * nick = g_object_get_data(G_OBJECT(mi), APP_INDICATOR_SHORTY_NICK);
	g_return_if_fail(nick != NULL);

	g_return_if_fail(IS_APP_INDICATOR(user_data));
	AppIndicator * self = APP_INDICATOR(user_data);
	AppIndicatorPrivate *priv = self->priv;

	g_return_if_fail(priv->shorties != NULL);

	indicator_desktop_shortcuts_nick_exec(priv->shorties, nick);

	return;
}

/**
 * app_indicator_build_menu_from_desktop:
 * @self: The #AppIndicator object to use
 * @desktop_file: A path to the desktop file to build the menu from
 * @desktop_profile: Which entries should be used from the desktop file
 *
 * This function allows for building the Application Indicator menu
 * from a static desktop file.
 */
void
app_indicator_build_menu_from_desktop (AppIndicator * self, const gchar * desktop_file, const gchar * desktop_profile)
{
	g_return_if_fail(IS_APP_INDICATOR(self));
	AppIndicatorPrivate *priv = self->priv;

	/* Build a new shortcuts object */
	if (priv->shorties != NULL) {
		g_object_unref(priv->shorties);
		priv->shorties = NULL;
	}
	priv->shorties = indicator_desktop_shortcuts_new(desktop_file, desktop_profile);
	g_return_if_fail(priv->shorties != NULL);

	const gchar ** nicks = indicator_desktop_shortcuts_get_nicks(priv->shorties);
	int nick_num;

	/* Place the items on a dbusmenu */
	DbusmenuMenuitem * root = dbusmenu_menuitem_new();

	for (nick_num = 0; nicks[nick_num] != NULL; nick_num++) {
		DbusmenuMenuitem * item = dbusmenu_menuitem_new();
		g_object_set_data(G_OBJECT(item), APP_INDICATOR_SHORTY_NICK, (gpointer)nicks[nick_num]);

		gchar * name = indicator_desktop_shortcuts_nick_get_name(priv->shorties, nicks[nick_num]);
		dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_LABEL, name);
		g_free(name);

		g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(shorty_activated_cb), self);

		dbusmenu_menuitem_child_append(root, item);
	}

	/* Swap it if needed */
	if (priv->menuservice == NULL) {
		gchar * path = g_strdup_printf(DEFAULT_ITEM_PATH "/%s/Menu", priv->clean_id);
		priv->menuservice = dbusmenu_server_new (path);
		g_free(path);
	}

	dbusmenu_server_set_root (priv->menuservice, root);

	if (priv->menu != NULL) {
		g_object_unref(G_OBJECT(priv->menu));
		priv->menu = NULL;
	}

	return;
}
