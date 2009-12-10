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

#include "libappindicator/app-indicator.h"
#include "libappindicator/app-indicator-enum-types.h"

#include "notification-item-server.h"
#include "notification-watcher-client.h"

#include "dbus-shared.h"

/**
	AppIndicatorPrivate:
	@id: The ID of the indicator.  Maps to AppIndicator::id.
	@category: Which category the indicator is.  Maps to AppIndicator::category.
	@status: The status of the indicator.  Maps to AppIndicator::status.
	@icon_name: The name of the icon to use.  Maps to AppIndicator::icon-name.
	@attention_icon_name: The name of the attention icon to use.  Maps to AppIndicator::attention-icon-name.
	@menu: The menu for this indicator.  Maps to AppIndicator::menu
	@watcher_proxy: The proxy connection to the watcher we're connected to.  If we're not connected to one this will be #NULL.

	All of the private data in an instance of a
	application indicator.
*/
struct _AppIndicatorPrivate {
	/* Properties */
	gchar                *id;
	AppIndicatorCategory  category;
	AppIndicatorStatus    status;
	gchar                *icon_name;
	gchar                *attention_icon_name;
        DbusmenuServer       *menuservice;
        GtkWidget            *menu;

	/* Fun stuff */
	DBusGProxy           *watcher_proxy;
	DBusGConnection      *connection;
};

/* Signals Stuff */
enum {
	NEW_ICON,
	NEW_ATTENTION_ICON,
	NEW_STATUS,
	CONNECTION_CHANGED,
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
	PROP_MENU,
	PROP_CONNECTED
};

/* The strings so that they can be slowly looked up. */
#define PROP_ID_S                    "id"
#define PROP_CATEGORY_S              "category"
#define PROP_STATUS_S                "status"
#define PROP_ICON_NAME_S             "icon-name"
#define PROP_ATTENTION_ICON_NAME_S   "attention-icon-name"
#define PROP_MENU_S                  "menu"
#define PROP_CONNECTED_S             "connected"

/* Private macro, shhhh! */
#define APP_INDICATOR_GET_PRIVATE(o) \
                             (G_TYPE_INSTANCE_GET_PRIVATE ((o), APP_INDICATOR_TYPE, AppIndicatorPrivate))

/* Boiler plate */
static void app_indicator_class_init (AppIndicatorClass *klass);
static void app_indicator_init       (AppIndicator *self);
static void app_indicator_dispose    (GObject *object);
static void app_indicator_finalize   (GObject *object);
/* Property functions */
static void app_indicator_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void app_indicator_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);
/* Other stuff */
static void check_connect (AppIndicator * self);
static void register_service_cb (DBusGProxy * proxy, GError * error, gpointer data);

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

	/* Properties */
	g_object_class_install_property (object_class,
                                         PROP_ID,
                                         g_param_spec_string(PROP_ID_S,
                                                             "The ID for this indicator",
                                                             "An ID that should be unique, but used consistently by this program and it's indicator.",
                                                             NULL,
                                                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
                                         PROP_CATEGORY,
                                         g_param_spec_string (PROP_CATEGORY_S,
                                                              "Indicator Category",
                                                              "The type of indicator that this represents.  Please don't use 'other'.  Defaults to 'Application Status'.",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
                                         PROP_STATUS,
                                         g_param_spec_string (PROP_STATUS_S,
                                                              "Indicator Status",
                                                              "Whether the indicator is shown or requests attention.  Defaults to 'off'.",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,
                                        PROP_ICON_NAME,
	                                g_param_spec_string (PROP_ICON_NAME_S,
                                                             "An icon for the indicator",
                                                             "The default icon that is shown for the indicator.",
                                                             NULL,
                                                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class,
                                         PROP_ATTENTION_ICON_NAME,
                                         g_param_spec_string (PROP_ATTENTION_ICON_NAME_S,
                                                              "An icon to show when the indicator request attention.",
                                                              "If the indicator sets it's status to 'attention' then this icon is shown.",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

        g_object_class_install_property(object_class,
                                        PROP_MENU,
                                        g_param_spec_string (PROP_MENU_S,
                                                             "The object path of the menu on DBus.",
                                                             "A method for getting the menu path as a string for DBus.",
                                                             NULL,
                                                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class,
                                         PROP_CONNECTED,
                                         g_param_spec_boolean (PROP_CONNECTED_S,
                                                               "Whether we're conneced to a watcher",
                                                               "Pretty simple, true if we have a reasonable expectation of being displayed through this object.  You should hide your TrayIcon if so.",
                                                               FALSE,
                                                               G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));


	/* Signals */

	/**
		AppIndicator::new-icon:
		@arg0: The #AppIndicator object

		Signaled when there is a new icon set for the
		object.
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

		Signaled when there is a new attention icon set for the
		object.
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

		Signaled when the status of the indicator changes.
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
		AppIndicator::connection-changed:
		@arg0: The #AppIndicator object
		@arg1: Whether we're connected or not

		Signaled when we connect to a watcher, or when it drops
		away.
	*/
	signals[CONNECTION_CHANGED] = g_signal_new (APP_INDICATOR_SIGNAL_CONNECTION_CHANGED,
	                                            G_TYPE_FROM_CLASS(klass),
	                                            G_SIGNAL_RUN_LAST,
	                                            G_STRUCT_OFFSET (AppIndicatorClass, connection_changed),
	                                            NULL, NULL,
	                                            g_cclosure_marshal_VOID__BOOLEAN,
	                                            G_TYPE_NONE, 1, G_TYPE_BOOLEAN, G_TYPE_NONE);

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
	priv->category = APP_INDICATOR_CATEGORY_OTHER;
	priv->status = APP_INDICATOR_STATUS_PASSIVE;
	priv->icon_name = NULL;
	priv->attention_icon_name = NULL;
	priv->menu = NULL;

	priv->watcher_proxy = NULL;
	priv->connection = NULL;

	/* Put the object on DBus */
	GError * error = NULL;
	priv->connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_error("Unable to connect to the session bus when creating application indicator: %s", error->message);
		g_error_free(error);
		return;
	}

	dbus_g_connection_register_g_object(priv->connection,
	                                    "/need/a/path",
	                                    G_OBJECT(self));

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

	if (priv->menu != NULL) {
		g_object_unref(G_OBJECT(priv->menu));
		priv->menu = NULL;
	}

	if (priv->watcher_proxy != NULL) {
		dbus_g_connection_flush(priv->connection);
		g_object_unref(G_OBJECT(priv->watcher_proxy));
		priv->watcher_proxy = NULL;
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

	if (priv->icon_name != NULL) {
		g_free(priv->icon_name);
		priv->icon_name = NULL;
	}

	if (priv->attention_icon_name != NULL) {
		g_free(priv->attention_icon_name);
		priv->attention_icon_name = NULL;
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
        const gchar *instr;
        GEnumValue *enum_val;

        switch (prop_id) {
        case PROP_ID:
          if (priv->id != NULL) {
            g_warning ("Resetting ID value when I already had a value of: %s", priv->id);
            g_free (priv->id);
            priv->id = NULL;
          }

          priv->id = g_strdup (g_value_get_string (value));

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
          instr = g_value_get_string (value);

          if (g_strcmp0 (priv->icon_name, instr) != 0)
            {
              if (priv->icon_name)
                g_free (priv->icon_name);

              priv->icon_name = g_strdup (instr);

              g_signal_emit (self, signals[NEW_ICON], 0, TRUE);
            }

          check_connect (self);
          break;

        case PROP_ATTENTION_ICON_NAME:
          app_indicator_set_attention_icon (APP_INDICATOR (object),
                                            g_value_get_string (value));
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

        case PROP_MENU:
          if (G_VALUE_HOLDS_STRING(value)) {
            if (priv->menuservice != NULL) {
              g_object_get_property (G_OBJECT (priv->menuservice), DBUSMENU_SERVER_PROP_DBUS_OBJECT, value);
            }
          } else {
            WARN_BAD_TYPE(PROP_MENU_S, value);
          }
          break;

        case PROP_CONNECTED:
          g_value_set_boolean (value, priv->watcher_proxy != NULL ? TRUE : FALSE);
          break;

        default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
          break;
        }

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

	GError * error = NULL;
	priv->watcher_proxy = dbus_g_proxy_new_for_name_owner(priv->connection,
	                                                      INDICATOR_APPLICATION_DBUS_ADDR,
	                                                      NOTIFICATION_WATCHER_DBUS_OBJ,
	                                                      NOTIFICATION_WATCHER_DBUS_IFACE,
	                                                      &error);
	if (error != NULL) {
		g_warning("Unable to create Ayatana Watcher proxy!  %s", error->message);
		/* TODO: This is where we should start looking at fallbacks */
		g_error_free(error);
		return;
	}

	org_ayatana_indicator_application_NotificationWatcher_register_service_async(priv->watcher_proxy, "/need/a/path", register_service_cb, self);

	return;
}

static void
register_service_cb (DBusGProxy * proxy, GError * error, gpointer data)
{
	AppIndicatorPrivate * priv = APP_INDICATOR_GET_PRIVATE(data);

	if (error != NULL) {
		g_warning("Unable to connect to the Notification Watcher: %s", error->message);
		g_object_unref(G_OBJECT(priv->watcher_proxy));
		priv->watcher_proxy = NULL;
	}
	return;
}

static const gchar *
category_from_enum (AppIndicatorCategory category)
{
  GEnumValue *value;

  value = g_enum_get_value ((GEnumClass *)g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_CATEGORY), category);
  return value->value_nick;
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
		#AppIndicator::id with @id, #AppIndicator::category
		with @category and #AppIndicator::icon-name with
		@icon_name.

        Return value: A pointer to a new #AppIndicator object.
 */
AppIndicator *
app_indicator_new (const gchar          *id,
                   const gchar          *icon_name,
                   AppIndicatorCategory  category)
{
  AppIndicator *indicator = g_object_new (APP_INDICATOR_TYPE,
                                          "id", id,
                                          "category", category_from_enum (category),
                                          "icon-name", icon_name,
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

	Wrapper function for property #AppIndicator::status.
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

	Wrapper function for property #AppIndicator::attention-icon.
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

      self->priv->attention_icon_name = g_strdup (icon_name);

      g_signal_emit (self, signals[NEW_ATTENTION_ICON], 0, TRUE);
    }
}

/**
        app_indicator_set_icon:
        @self: The #AppIndicator object to use
        @icon_name: The icon name to set.

		Sets the default icon to use when the status is active but
		not set to attention.  In most cases, this should be the
		application icon for the program.
**/
void
app_indicator_set_icon (AppIndicator *self, const gchar *icon_name)
{
  g_return_if_fail (IS_APP_INDICATOR (self));
  g_return_if_fail (icon_name != NULL);

  if (g_strcmp0 (self->priv->attention_icon_name, icon_name) != 0)
    {
      if (self->priv->attention_icon_name)
        g_free (self->priv->attention_icon_name);

      self->priv->attention_icon_name = g_strdup (icon_name);

      g_signal_emit (self, signals[NEW_ATTENTION_ICON], 0, TRUE);
    }
}

static void
activate_menuitem (DbusmenuMenuitem *mi, gpointer user_data)
{
  GtkWidget *widget = (GtkWidget *)user_data;

  gtk_menu_item_activate (GTK_MENU_ITEM (widget));
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

static void
container_iterate (GtkWidget *widget,
                   gpointer   data)
{
  DbusmenuMenuitem *root = (DbusmenuMenuitem *)data;
  DbusmenuMenuitem *child;
  const gchar *label = NULL;
  gboolean label_set = FALSE;

  child = dbusmenu_menuitem_new ();

  if (GTK_IS_SEPARATOR_MENU_ITEM (widget))
    {
      dbusmenu_menuitem_property_set (child,
                                      "type",
                                      DBUSMENU_CLIENT_TYPES_SEPARATOR);
    }
  else
    {
      label = gtk_menu_item_get_label (GTK_MENU_ITEM (widget));

      if (GTK_IS_IMAGE_MENU_ITEM (widget))
        {
          GtkWidget *image = gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM (widget));

          if (gtk_image_get_storage_type (GTK_IMAGE (image)) == GTK_IMAGE_STOCK)
            {
              GtkStockItem stock;

              gtk_stock_lookup (GTK_IMAGE (image)->data.stock.stock_id, &stock);

              dbusmenu_menuitem_property_set (child,
                                              DBUSMENU_MENUITEM_PROP_ICON,
                                              GTK_IMAGE (image)->data.stock.stock_id);

              if (stock.label != NULL)
                {
                  dbusmenu_menuitem_property_set (child,
                                                  DBUSMENU_MENUITEM_PROP_LABEL,
                                                  stock.label);
                  label_set = TRUE;
                }
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

  g_signal_connect (G_OBJECT (child),
                    DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                    G_CALLBACK (activate_menuitem), widget);
  dbusmenu_menuitem_child_append (root, child);
}

static void
setup_dbusmenu (AppIndicator *self)
{
  AppIndicatorPrivate *priv;
  DbusmenuMenuitem *root;

  priv = self->priv;
  root = dbusmenu_menuitem_new ();

  gtk_container_forall (GTK_CONTAINER (priv->menu),
                        container_iterate,
                        root);

  priv->menuservice = dbusmenu_server_new ("/need/a/menu/path");
  dbusmenu_server_set_root (priv->menuservice, root);
}

/**
        app_indicator_set_menu:
        @self: The #AppIndicator
        @menu: A #GtkMenu to set

		Sets the menu that should be shown when the Application Indicator
		is clicked on in the panel.  An application indicator will not
		be rendered unless it has a menu.
**/
void
app_indicator_set_menu (AppIndicator *self, GtkMenu *menu)
{
  AppIndicatorPrivate *priv;

  g_return_if_fail (IS_APP_INDICATOR (self));
  g_return_if_fail (GTK_IS_MENU (menu));

  priv = self->priv;

  if (priv->menu != NULL)
    {
      g_object_unref (priv->menu);
    }

  priv->menu = GTK_WIDGET (menu);
  g_object_ref (priv->menu);

  setup_dbusmenu (self);

  check_connect (self);
}

/**
	app_indicator_get_id:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator::id.

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

	Wrapper function for property #AppIndicator::category.

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

	Wrapper function for property #AppIndicator::status.

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

	Wrapper function for property #AppIndicator::icon-name.

	Return value: The current icon name.
*/
const gchar *
app_indicator_get_icon (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->icon_name;
}

/**
	app_indicator_get_attention_icon:
	@self: The #AppIndicator object to use

	Wrapper function for property #AppIndicator::attention-icon-name.

	Return value: The current attention icon name.
*/
const gchar *
app_indicator_get_attention_icon (AppIndicator *self)
{
  g_return_val_if_fail (IS_APP_INDICATOR (self), NULL);

  return self->priv->attention_icon_name;
}
