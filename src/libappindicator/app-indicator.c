#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dbus/dbus-glib.h>
#include <libdbusmenu-glib/server.h>

#include "libappindicator/app-indicator.h"
#include "libappindicator/app-indicator-enum-types.h"

#include "notification-item-server.h"
#include "notification-watcher-client.h"

#include "dbus-shared.h"

/**
	ApplicationIndicatorPrivate:
	@id: The ID of the indicator.  Maps to ApplicationIndicator::id.
	@category: Which category the indicator is.  Maps to ApplicationIndicator::category.
	@status: The status of the indicator.  Maps to ApplicationIndicator::status.
	@icon_name: The name of the icon to use.  Maps to ApplicationIndicator::icon-name.
	@attention_icon_name: The name of the attention icon to use.  Maps to ApplicationIndicator::attention-icon-name.
	@menu: The menu for this indicator.  Maps to ApplicationIndicator::menu
	@watcher_proxy: The proxy connection to the watcher we're connected to.  If we're not connected to one this will be #NULL.

	All of the private data in an instance of a
	application indicator.
*/
typedef struct _ApplicationIndicatorPrivate ApplicationIndicatorPrivate;
struct _ApplicationIndicatorPrivate {
	/* Properties */
	gchar * id;
	ApplicationIndicatorCategory category;
	ApplicationIndicatorStatus status;
	gchar * icon_name;
	gchar * attention_icon_name;
	DbusmenuServer * menu;

	/* Fun stuff */
	DBusGProxy * watcher_proxy;
	DBusGConnection * connection;
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
	PROP_CATEGORY_ENUM,
	PROP_STATUS,
	PROP_STATUS_ENUM,
	PROP_ICON_NAME,
	PROP_ATTENTION_ICON_NAME,
	PROP_MENU,
	PROP_MENU_OBJECT,
	PROP_CONNECTED
};

/* The strings so that they can be slowly looked up. */
#define PROP_ID_S                    "id"
#define PROP_CATEGORY_S              "category"
#define PROP_CATEGORY_ENUM_S         "category-enum"
#define PROP_STATUS_S                "status"
#define PROP_STATUS_ENUM_S           "status-enum"
#define PROP_ICON_NAME_S             "icon-name"
#define PROP_ATTENTION_ICON_NAME_S   "attention-icon-name"
#define PROP_MENU_S                  "menu"
#define PROP_MENU_OBJECT_S           "menu-object"
#define PROP_CONNECTED_S             "connected"

/* Private macro, shhhh! */
#define APPLICATION_INDICATOR_GET_PRIVATE(o) \
                             (G_TYPE_INSTANCE_GET_PRIVATE ((o), APPLICATION_INDICATOR_TYPE, ApplicationIndicatorPrivate))

/* Boiler plate */
static void application_indicator_class_init (ApplicationIndicatorClass *klass);
static void application_indicator_init       (ApplicationIndicator *self);
static void application_indicator_dispose    (GObject *object);
static void application_indicator_finalize   (GObject *object);
/* Property functions */
static void application_indicator_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void application_indicator_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);
/* Other stuff */
static void check_connect (ApplicationIndicator * self);
static void register_service_cb (DBusGProxy * proxy, GError * error, gpointer data);

/* GObject type */
G_DEFINE_TYPE (ApplicationIndicator, application_indicator, G_TYPE_OBJECT);

static void
application_indicator_class_init (ApplicationIndicatorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (ApplicationIndicatorPrivate));

	/* Clean up */
	object_class->dispose = application_indicator_dispose;
	object_class->finalize = application_indicator_finalize;

	/* Property funcs */
	object_class->set_property = application_indicator_set_property;
	object_class->get_property = application_indicator_get_property;

	/* Properties */
	g_object_class_install_property(object_class, PROP_ID,
	                                g_param_spec_string(PROP_ID_S,
	                                                    "The ID for this indicator",
	                                                    "An ID that should be unique, but used consistently by this program and it's indicator.",
	                                                    NULL,
	                                                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_CATEGORY,
	                                g_param_spec_string(PROP_CATEGORY_S,
	                                                    "Indicator Category as a string",
	                                                    "The type of indicator that this represents as a string.  For DBus.",
	                                                    NULL,
	                                                    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_CATEGORY_ENUM,
	                                g_param_spec_enum(PROP_CATEGORY_ENUM_S,
	                                                  "Indicator Category",
	                                                  "The type of indicator that this represents.  Please don't use 'other'.  Defaults to 'Application Status'.",
	                                                  APPLICATION_INDICATOR_TYPE_INDICATOR_CATEGORY,
	                                                  APPLICATION_INDICATOR_CATEGORY_APPLICATION_STATUS,
	                                                  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_STATUS,
	                                g_param_spec_string(PROP_STATUS_S,
	                                                    "Indicator Status as a string",
	                                                    "The status of the indicator represented as a string.  For DBus.",
	                                                    NULL,
	                                                    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_STATUS_ENUM,
	                                g_param_spec_enum(PROP_STATUS_ENUM_S,
	                                                  "Indicator Status",
	                                                  "Whether the indicator is shown or requests attention.  Defaults to 'off'.",
	                                                  APPLICATION_INDICATOR_TYPE_INDICATOR_STATUS,
	                                                  APPLICATION_INDICATOR_STATUS_PASSIVE,
	                                                  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_ICON_NAME,
	                                g_param_spec_string(PROP_ICON_NAME_S,
	                                                    "An icon for the indicator",
	                                                    "The default icon that is shown for the indicator.",
	                                                    NULL,
	                                                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_ATTENTION_ICON_NAME,
	                                g_param_spec_string(PROP_ATTENTION_ICON_NAME_S,
	                                                    "An icon to show when the indicator request attention.",
	                                                    "If the indicator sets it's status to 'attention' then this icon is shown.",
	                                                    NULL,
	                                                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_MENU,
	                                g_param_spec_string(PROP_MENU_S,
	                                                    "The object path of the menu on DBus.",
	                                                    "A method for getting the menu path as a string for DBus.",
	                                                    NULL,
	                                                    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_MENU_OBJECT,
	                                g_param_spec_object(PROP_MENU_OBJECT_S,
	                                                    "The Menu for the indicator",
	                                                    "A DBus Menu Server object that can have a menu attached to it.  The object from this menu will be sent across the bus for the client to connect to and signal.",
	                                                    DBUSMENU_TYPE_SERVER,
	                                                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_CONNECTED,
	                                g_param_spec_boolean(PROP_CONNECTED_S,
	                                                     "Whether we're conneced to a watcher",
	                                                     "Pretty simple, true if we have a reasonable expectation of being displayed through this object.  You should hide your TrayIcon if so.",
	                                                     FALSE,
	                                                     G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));


	/* Signals */

	/**
		ApplicationIndicator::new-icon:
		@arg0: The #ApplicationIndicator object
		
		Signaled when there is a new icon set for the
		object.
	*/
	signals[NEW_ICON] = g_signal_new (APPLICATION_INDICATOR_SIGNAL_NEW_ICON,
	                                  G_TYPE_FROM_CLASS(klass),
	                                  G_SIGNAL_RUN_LAST,
	                                  G_STRUCT_OFFSET (ApplicationIndicatorClass, new_icon),
	                                  NULL, NULL,
	                                  g_cclosure_marshal_VOID__VOID,
	                                  G_TYPE_NONE, 0, G_TYPE_NONE);

	/**
		ApplicationIndicator::new-attention-icon:
		@arg0: The #ApplicationIndicator object
		
		Signaled when there is a new attention icon set for the
		object.
	*/
	signals[NEW_ATTENTION_ICON] = g_signal_new (APPLICATION_INDICATOR_SIGNAL_NEW_ATTENTION_ICON,
	                                            G_TYPE_FROM_CLASS(klass),
	                                            G_SIGNAL_RUN_LAST,
	                                            G_STRUCT_OFFSET (ApplicationIndicatorClass, new_attention_icon),
	                                            NULL, NULL,
	                                            g_cclosure_marshal_VOID__VOID,
	                                            G_TYPE_NONE, 0, G_TYPE_NONE);

	/**
		ApplicationIndicator::new-status:
		@arg0: The #ApplicationIndicator object
		@arg1: The string value of the #ApplicationIndicatorStatus enum.
		
		Signaled when the status of the indicator changes.
	*/
	signals[NEW_STATUS] = g_signal_new (APPLICATION_INDICATOR_SIGNAL_NEW_STATUS,
	                                    G_TYPE_FROM_CLASS(klass),
	                                    G_SIGNAL_RUN_LAST,
	                                    G_STRUCT_OFFSET (ApplicationIndicatorClass, new_status),
	                                    NULL, NULL,
	                                    g_cclosure_marshal_VOID__STRING,
	                                    G_TYPE_NONE, 1, G_TYPE_STRING, G_TYPE_NONE);

	/**
		ApplicationIndicator::connection-changed:
		@arg0: The #ApplicationIndicator object
		@arg1: Whether we're connected or not
		
		Signaled when we connect to a watcher, or when it drops
		away.
	*/
	signals[CONNECTION_CHANGED] = g_signal_new (APPLICATION_INDICATOR_SIGNAL_CONNECTION_CHANGED,
	                                            G_TYPE_FROM_CLASS(klass),
	                                            G_SIGNAL_RUN_LAST,
	                                            G_STRUCT_OFFSET (ApplicationIndicatorClass, connection_changed),
	                                            NULL, NULL,
	                                            g_cclosure_marshal_VOID__BOOLEAN,
	                                            G_TYPE_NONE, 1, G_TYPE_BOOLEAN, G_TYPE_NONE);
	
	/* Initialize the object as a DBus type */
	dbus_g_object_type_install_info(APPLICATION_INDICATOR_TYPE,
	                                &dbus_glib__notification_item_server_object_info);

	return;
}

static void
application_indicator_init (ApplicationIndicator *self)
{
	ApplicationIndicatorPrivate * priv = APPLICATION_INDICATOR_GET_PRIVATE(self);

	priv->id = NULL;
	priv->category = APPLICATION_INDICATOR_CATEGORY_OTHER;
	priv->status = APPLICATION_INDICATOR_STATUS_PASSIVE;
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

	return;
}

/* Free all objects, make sure that all the dbus
   signals are sent out before we shut this down. */
static void
application_indicator_dispose (GObject *object)
{
	ApplicationIndicator * self = APPLICATION_INDICATOR(object);
	g_return_if_fail(self != NULL);

	ApplicationIndicatorPrivate * priv = APPLICATION_INDICATOR_GET_PRIVATE(self);

	if (priv->status != APPLICATION_INDICATOR_STATUS_PASSIVE) {
		application_indicator_set_status(self, APPLICATION_INDICATOR_STATUS_PASSIVE);
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

	G_OBJECT_CLASS (application_indicator_parent_class)->dispose (object);
	return;
}

/* Free all of the memory that we could be using in
   the object. */
static void
application_indicator_finalize (GObject *object)
{
	ApplicationIndicator * self = APPLICATION_INDICATOR(object);
	g_return_if_fail(self != NULL);

	ApplicationIndicatorPrivate * priv = APPLICATION_INDICATOR_GET_PRIVATE(self);

	if (priv->status != APPLICATION_INDICATOR_STATUS_PASSIVE) {
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

	G_OBJECT_CLASS (application_indicator_parent_class)->finalize (object);
	return;
}

#define WARN_BAD_TYPE(prop, value)  g_warning("Can not work with property '%s' with value of type '%s'.", prop, G_VALUE_TYPE_NAME(value))

/* Set some properties */
static void
application_indicator_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec)
{
	ApplicationIndicator * self = APPLICATION_INDICATOR(object);
	g_return_if_fail(self != NULL);

	ApplicationIndicatorPrivate * priv = APPLICATION_INDICATOR_GET_PRIVATE(self);

	switch (prop_id) {
	/* *********************** */
	case PROP_ID:
		if (G_VALUE_HOLDS_STRING(value)) {
			if (priv->id != NULL) {
				g_warning("Resetting ID value when I already had a value of: %s", priv->id);
				g_free(priv->id);
				priv->id = NULL;
			}
			priv->id = g_strdup(g_value_get_string(value));
		} else {
			WARN_BAD_TYPE(PROP_ID_S, value);
		}
		check_connect(self);
		break;
	/* *********************** */
	case PROP_CATEGORY_ENUM:
		if (G_VALUE_HOLDS_ENUM(value)) {
			priv->category = g_value_get_enum(value);
		} else {
			WARN_BAD_TYPE(PROP_CATEGORY_ENUM_S, value);
		}
		break;
	/* *********************** */
	case PROP_STATUS_ENUM: {
		gboolean changed = FALSE;
		if (G_VALUE_HOLDS_ENUM(value)) {
			if (priv->status != g_value_get_enum(value)) {
				changed = TRUE;
			}
			priv->status = g_value_get_enum(value);
		} else {
			WARN_BAD_TYPE(PROP_STATUS_ENUM_S, value);
		}
		if (changed) {
			GParamSpecEnum * enumspec = G_PARAM_SPEC_ENUM(pspec);
			if (enumspec != NULL) {
				GEnumValue * enumval = g_enum_get_value(enumspec->enum_class, priv->status);
				g_signal_emit(object, signals[NEW_STATUS], 0, enumval->value_nick, TRUE);
			}
		}
		break;
	}
	/* *********************** */
	case PROP_ICON_NAME:
		if (G_VALUE_HOLDS_STRING(value)) {
			const gchar * instr = g_value_get_string(value);
			gboolean changed = FALSE;
			if (priv->icon_name == NULL) {
				priv->icon_name = g_strdup(instr);
				changed = TRUE;
			} else if (!g_strcmp0(instr, priv->icon_name)) {
				changed = FALSE;
			} else {
				g_free(priv->icon_name);
				priv->icon_name = g_strdup(instr);
				changed = TRUE;
			}
			if (changed) {
				g_signal_emit(object, signals[NEW_ICON], 0, TRUE);
			}
		} else {
			WARN_BAD_TYPE(PROP_ICON_NAME_S, value);
		}
		check_connect(self);
		break;
	/* *********************** */
	case PROP_ATTENTION_ICON_NAME:
		if (G_VALUE_HOLDS_STRING(value)) {
			const gchar * instr = g_value_get_string(value);
			gboolean changed = FALSE;
			if (priv->attention_icon_name == NULL) {
				priv->attention_icon_name = g_strdup(instr);
				changed = TRUE;
			} else if (!g_strcmp0(instr, priv->attention_icon_name)) {
				changed = FALSE;
			} else {
				g_free(priv->attention_icon_name);
				priv->attention_icon_name = g_strdup(instr);
				changed = TRUE;
			}
			if (changed) {
				g_signal_emit(object, signals[NEW_ATTENTION_ICON], 0, TRUE);
			}
		} else {
			WARN_BAD_TYPE(PROP_ATTENTION_ICON_NAME_S, value);
		}
		break;
	/* *********************** */
	case PROP_MENU_OBJECT:
		if (G_VALUE_HOLDS_OBJECT(value)) {
			if (priv->menu != NULL) {
				g_object_unref(G_OBJECT(priv->menu));
			}
			priv->menu = DBUSMENU_SERVER(g_value_get_object(value));
			g_object_ref(G_OBJECT(priv->menu));
		} else {
			WARN_BAD_TYPE(PROP_MENU_S, value);
		}
		check_connect(self);
		break;
	/* *********************** */
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}

	return;
}

/* Function to fill our value with the property it's requesting. */
static void
application_indicator_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec)
{
	ApplicationIndicator * self = APPLICATION_INDICATOR(object);
	g_return_if_fail(self != NULL);

	ApplicationIndicatorPrivate * priv = APPLICATION_INDICATOR_GET_PRIVATE(self);

	switch (prop_id) {
	/* *********************** */
	case PROP_ID:
		if (G_VALUE_HOLDS_STRING(value)) {
			g_value_set_string(value, priv->id);
		} else {
			WARN_BAD_TYPE(PROP_ID_S, value);
		}
		break;
	/* *********************** */
	case PROP_CATEGORY:
		if (G_VALUE_HOLDS_STRING(value)) {
			GParamSpec * spec_for_enum = g_object_class_find_property(G_OBJECT_GET_CLASS(object), PROP_CATEGORY_ENUM_S);
			GParamSpecEnum * enumspec = G_PARAM_SPEC_ENUM(spec_for_enum);
			if (enumspec != NULL) {
				GEnumValue * enumval = g_enum_get_value(enumspec->enum_class, priv->category);
				g_value_set_string(value, enumval->value_nick);
			} else {
				g_assert_not_reached();
			}
		} else {
			WARN_BAD_TYPE(PROP_CATEGORY_S, value);
		}
		break;
	/* *********************** */
	case PROP_CATEGORY_ENUM:
		if (G_VALUE_HOLDS_ENUM(value)) {
			/* We want the enum value */
			g_value_set_enum(value, priv->category);
		} else {
			WARN_BAD_TYPE(PROP_CATEGORY_ENUM_S, value);
		}
		break;
	/* *********************** */
	case PROP_STATUS:
		if (G_VALUE_HOLDS_STRING(value)) {
			GParamSpec * spec_for_enum = g_object_class_find_property(G_OBJECT_GET_CLASS(object), PROP_STATUS_ENUM_S);
			GParamSpecEnum * enumspec = G_PARAM_SPEC_ENUM(spec_for_enum);
			if (enumspec != NULL) {
				GEnumValue * enumval = g_enum_get_value(enumspec->enum_class, priv->status);
				g_value_set_string(value, enumval->value_nick);
			} else {
				g_assert_not_reached();
			}
		} else {
			WARN_BAD_TYPE(PROP_STATUS_S, value);
		}
		break;
	/* *********************** */
	case PROP_STATUS_ENUM:
		if (G_VALUE_HOLDS_ENUM(value)) {
			/* We want the enum value */
			g_value_set_enum(value, priv->status);
		} else {
			WARN_BAD_TYPE(PROP_STATUS_ENUM_S, value);
		}
		break;
	/* *********************** */
	case PROP_ICON_NAME:
		if (G_VALUE_HOLDS_STRING(value)) {
			g_value_set_string(value, priv->icon_name);
		} else {
			WARN_BAD_TYPE(PROP_ICON_NAME_S, value);
		}
		break;
	/* *********************** */
	case PROP_ATTENTION_ICON_NAME:
		if (G_VALUE_HOLDS_STRING(value)) {
			g_value_set_string(value, priv->attention_icon_name);
		} else {
			WARN_BAD_TYPE(PROP_ATTENTION_ICON_NAME_S, value);
		}
		break;
	/* *********************** */
	case PROP_MENU:
		if (G_VALUE_HOLDS_STRING(value)) {
			if (priv->menu != NULL) {
				g_object_get_property(G_OBJECT(priv->menu), DBUSMENU_SERVER_PROP_DBUS_OBJECT, value);
			}
		} else {
			WARN_BAD_TYPE(PROP_MENU_S, value);
		}
		break;
	/* *********************** */
	case PROP_MENU_OBJECT:
		if (G_VALUE_HOLDS_OBJECT(value)) {
			g_value_set_object(value, priv->menu);
		} else {
			WARN_BAD_TYPE(PROP_MENU_OBJECT_S, value);
		}
		break;
	/* *********************** */
	case PROP_CONNECTED:
		if (G_VALUE_HOLDS_BOOLEAN(value)) {
			g_value_set_boolean(value, priv->watcher_proxy != NULL ? TRUE : FALSE);
		} else {
			WARN_BAD_TYPE(PROP_CONNECTED_S, value);
		}
		break;
	/* *********************** */
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
check_connect (ApplicationIndicator * self)
{
	ApplicationIndicatorPrivate * priv = APPLICATION_INDICATOR_GET_PRIVATE(self);

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
	ApplicationIndicatorPrivate * priv = APPLICATION_INDICATOR_GET_PRIVATE(data);

	if (error != NULL) {
		g_warning("Unable to connect to the Notification Watcher: %s", error->message);
		g_object_unref(G_OBJECT(priv->watcher_proxy));
		priv->watcher_proxy = NULL;
	}
	return;
}


/* ************************* */
/*    Public Functions       */
/* ************************* */

/**
	application_indicator_set_id:
	@ci: The #ApplicationIndicator object to use
	@id: ID to set for this indicator

	Wrapper function for property #ApplicationIndicator::id.
*/
void
application_indicator_set_id (ApplicationIndicator * ci, const gchar * id)
{
	GValue value = {0};
	g_value_init(&value, G_TYPE_STRING);
	g_value_set_string(&value, id);
	g_object_set_property(G_OBJECT(ci), PROP_ID_S, &value);
	return;
}

/**
	application_indicator_set_category:
	@ci: The #ApplicationIndicator object to use
	@category: The category to set for this indicator

	Wrapper function for property #ApplicationIndicator::category.
*/
void
application_indicator_set_category (ApplicationIndicator * ci, ApplicationIndicatorCategory category)
{
	GValue value = {0};
	g_value_init(&value, APPLICATION_INDICATOR_TYPE_INDICATOR_CATEGORY);
	g_value_set_enum(&value, category);
	g_object_set_property(G_OBJECT(ci), PROP_CATEGORY_ENUM_S, &value);
	return;
}

/**
	application_indicator_set_status:
	@ci: The #ApplicationIndicator object to use
	@status: The status to set for this indicator

	Wrapper function for property #ApplicationIndicator::status.
*/
void
application_indicator_set_status (ApplicationIndicator * ci, ApplicationIndicatorStatus status)
{
	GValue value = {0};
	g_value_init(&value, APPLICATION_INDICATOR_TYPE_INDICATOR_STATUS);
	g_value_set_enum(&value, status);
	g_object_set_property(G_OBJECT(ci), PROP_STATUS_ENUM_S, &value);
	return;
}

/**
	application_indicator_set_icon:
	@ci: The #ApplicationIndicator object to use
	@icon_name: The name of the icon to set for this indicator

	Wrapper function for property #ApplicationIndicator::icon.
*/
void application_indicator_set_icon (ApplicationIndicator * ci, const gchar * icon_name)
{
	GValue value = {0};
	g_value_init(&value, G_TYPE_STRING);
	g_value_set_string(&value, icon_name);
	g_object_set_property(G_OBJECT(ci), PROP_ICON_NAME_S, &value);
	return;
}

/**
	application_indicator_set_attention_icon:
	@ci: The #ApplicationIndicator object to use
	@icon_name: The name of the attention icon to set for this indicator

	Wrapper function for property #ApplicationIndicator::attention-icon.
*/
void
application_indicator_set_attention_icon (ApplicationIndicator * ci, const gchar * icon_name)
{
	GValue value = {0};
	g_value_init(&value, G_TYPE_STRING);
	g_value_set_string(&value, icon_name);
	g_object_set_property(G_OBJECT(ci), PROP_ATTENTION_ICON_NAME_S, &value);
	return;
}

/**
	application_indicator_set_menu:
	@ci: The #ApplicationIndicator object to use
	@menu: The object with the menu for the indicator

	Wrapper function for property #ApplicationIndicator::menu.
*/
void
application_indicator_set_menu (ApplicationIndicator * ci, DbusmenuServer * menu)
{
	GValue value = {0};
	g_value_init(&value, G_TYPE_OBJECT);
	g_value_set_object(&value, G_OBJECT(menu));
	g_object_set_property(G_OBJECT(ci), PROP_MENU_OBJECT_S, &value);
	return;
}

/**
	application_indicator_get_id:
	@ci: The #ApplicationIndicator object to use

	Wrapper function for property #ApplicationIndicator::id.

	Return value: The current ID
*/
const gchar *
application_indicator_get_id (ApplicationIndicator * ci)
{
	GValue value = {0};
	g_value_init(&value, G_TYPE_STRING);
	g_object_get_property(G_OBJECT(ci), PROP_ID_S, &value);
	return g_value_get_string(&value);
}

/**
	application_indicator_get_category:
	@ci: The #ApplicationIndicator object to use

	Wrapper function for property #ApplicationIndicator::category.

	Return value: The current category.
*/
ApplicationIndicatorCategory
application_indicator_get_category (ApplicationIndicator * ci)
{
	GValue value = {0};
	g_value_init(&value, APPLICATION_INDICATOR_TYPE_INDICATOR_CATEGORY);
	g_object_get_property(G_OBJECT(ci), PROP_CATEGORY_ENUM_S, &value);
	return g_value_get_enum(&value);
}

/**
	application_indicator_get_status:
	@ci: The #ApplicationIndicator object to use

	Wrapper function for property #ApplicationIndicator::status.

	Return value: The current status.
*/
ApplicationIndicatorStatus
application_indicator_get_status (ApplicationIndicator * ci)
{
	GValue value = {0};
	g_value_init(&value, APPLICATION_INDICATOR_TYPE_INDICATOR_STATUS);
	g_object_get_property(G_OBJECT(ci), PROP_STATUS_ENUM_S, &value);
	return g_value_get_enum(&value);
}

/**
	application_indicator_get_icon:
	@ci: The #ApplicationIndicator object to use

	Wrapper function for property #ApplicationIndicator::icon-name.

	Return value: The current icon name.
*/
const gchar *
application_indicator_get_icon (ApplicationIndicator * ci)
{
	GValue value = {0};
	g_value_init(&value, G_TYPE_STRING);
	g_object_get_property(G_OBJECT(ci), PROP_ICON_NAME_S, &value);
	return g_value_get_string(&value);
}

/**
	application_indicator_get_attention_icon:
	@ci: The #ApplicationIndicator object to use

	Wrapper function for property #ApplicationIndicator::attention-icon-name.

	Return value: The current attention icon name.
*/
const gchar *
application_indicator_get_attention_icon (ApplicationIndicator * ci)
{
	GValue value = {0};
	g_value_init(&value, G_TYPE_STRING);
	g_object_get_property(G_OBJECT(ci), PROP_ATTENTION_ICON_NAME_S, &value);
	return g_value_get_string(&value);
}

/**
	application_indicator_get_menu:
	@ci: The #ApplicationIndicator object to use

	Wrapper function for property #ApplicationIndicator::menu.

	Return value: The current menu being used.
*/
DbusmenuServer *
application_indicator_get_menu (ApplicationIndicator * ci)
{
	GValue value = {0};
	g_value_init(&value, G_TYPE_OBJECT);
	g_object_get_property(G_OBJECT(ci), PROP_MENU_OBJECT_S, &value);
	return DBUSMENU_SERVER(g_value_get_object(&value));
}


