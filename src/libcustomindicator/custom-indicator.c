#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dbus/dbus-glib.h>
#include <libdbusmenu-glib/server.h>

#include "libcustomindicator/custom-indicator.h"
#include "libcustomindicator/custom-indicator-enum-types.h"

/**
	CustomIndicatorPrivate:
	@id: The ID of the indicator.  Maps to CustomIndicator::id.
	@category: Which category the indicator is.  Maps to CustomIndicator::category.
	@status: The status of the indicator.  Maps to CustomIndicator::status.
	@icon_name: The name of the icon to use.  Maps to CustomIndicator::icon-name.
	@attention_icon_name: The name of the attention icon to use.  Maps to CustomIndicator::attention-icon-name.
	@menu: The menu for this indicator.  Maps to CustomIndicator::menu
	@watcher_proxy: The proxy connection to the watcher we're connected to.  If we're not connected to one this will be #NULL.

	All of the private data in an instance of a
	custom indicator.
*/
typedef struct _CustomIndicatorPrivate CustomIndicatorPrivate;
struct _CustomIndicatorPrivate {
	/* Properties */
	gchar * id;
	CustomIndicatorCategory category;
	CustomIndicatorStatus status;
	gchar * icon_name;
	gchar * attention_icon_name;
	DbusmenuServer * menu;

	/* Fun stuff */
	DBusGProxy * watcher_proxy;
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
#define CUSTOM_INDICATOR_GET_PRIVATE(o) \
                             (G_TYPE_INSTANCE_GET_PRIVATE ((o), CUSTOM_INDICATOR_TYPE, CustomIndicatorPrivate))

/* Boiler plate */
static void custom_indicator_class_init (CustomIndicatorClass *klass);
static void custom_indicator_init       (CustomIndicator *self);
static void custom_indicator_dispose    (GObject *object);
static void custom_indicator_finalize   (GObject *object);
/* Property functions */
static void custom_indicator_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void custom_indicator_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);

/* GObject type */
G_DEFINE_TYPE (CustomIndicator, custom_indicator, G_TYPE_OBJECT);

static void
custom_indicator_class_init (CustomIndicatorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (CustomIndicatorPrivate));

	/* Clean up */
	object_class->dispose = custom_indicator_dispose;
	object_class->finalize = custom_indicator_finalize;

	/* Property funcs */
	object_class->set_property = custom_indicator_set_property;
	object_class->get_property = custom_indicator_get_property;

	/* Properties */
	g_object_class_install_property(object_class, PROP_ID,
	                                g_param_spec_string(PROP_ID_S,
	                                                    "The ID for this indicator",
	                                                    "An ID that should be unique, but used consistently by this program and it's indicator.",
	                                                    NULL,
	                                                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_CATEGORY,
	                                g_param_spec_enum(PROP_CATEGORY_S,
	                                                  "Indicator Category",
	                                                  "The type of indicator that this represents.  Please don't use 'other'.  Defaults to 'Application Status'.",
	                                                  CUSTOM_INDICATOR_TYPE_INDICATOR_CATEGORY,
	                                                  CUSTOM_INDICATOR_CATEGORY_APPLICATION_STATUS,
	                                                  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_STATUS,
	                                g_param_spec_enum(PROP_STATUS_S,
	                                                  "Indicator Status",
	                                                  "Whether the indicator is shown or requests attention.  Defaults to 'off'.",
	                                                  CUSTOM_INDICATOR_TYPE_INDICATOR_STATUS,
	                                                  CUSTOM_INDICATOR_STATUS_OFF,
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
	                                g_param_spec_object(PROP_MENU_S,
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
		CustomIndicator::new-icon:
		@arg0: The #CustomIndicator object
		
		Signaled when there is a new icon set for the
		object.
	*/
	signals[NEW_ICON] = g_signal_new (CUSTOM_INDICATOR_SIGNAL_NEW_ICON,
	                                  G_TYPE_FROM_CLASS(klass),
	                                  G_SIGNAL_RUN_LAST,
	                                  G_STRUCT_OFFSET (CustomIndicatorClass, new_icon),
	                                  NULL, NULL,
	                                  g_cclosure_marshal_VOID__VOID,
	                                  G_TYPE_NONE, 0, G_TYPE_NONE);

	/**
		CustomIndicator::new-attention-icon:
		@arg0: The #CustomIndicator object
		
		Signaled when there is a new attention icon set for the
		object.
	*/
	signals[NEW_ATTENTION_ICON] = g_signal_new (CUSTOM_INDICATOR_SIGNAL_NEW_ATTENTION_ICON,
	                                            G_TYPE_FROM_CLASS(klass),
	                                            G_SIGNAL_RUN_LAST,
	                                            G_STRUCT_OFFSET (CustomIndicatorClass, new_attention_icon),
	                                            NULL, NULL,
	                                            g_cclosure_marshal_VOID__VOID,
	                                            G_TYPE_NONE, 0, G_TYPE_NONE);

	/**
		CustomIndicator::new-status:
		@arg0: The #CustomIndicator object
		@arg1: The string value of the #CustomIndicatorStatus enum.
		
		Signaled when the status of the indicator changes.
	*/
	signals[NEW_STATUS] = g_signal_new (CUSTOM_INDICATOR_SIGNAL_NEW_STATUS,
	                                    G_TYPE_FROM_CLASS(klass),
	                                    G_SIGNAL_RUN_LAST,
	                                    G_STRUCT_OFFSET (CustomIndicatorClass, new_status),
	                                    NULL, NULL,
	                                    g_cclosure_marshal_VOID__STRING,
	                                    G_TYPE_NONE, 1, G_TYPE_STRING, G_TYPE_NONE);

	/**
		CustomIndicator::connection-changed:
		@arg0: The #CustomIndicator object
		@arg1: Whether we're connected or not
		
		Signaled when we connect to a watcher, or when it drops
		away.
	*/
	signals[CONNECTION_CHANGED] = g_signal_new (CUSTOM_INDICATOR_SIGNAL_CONNECTION_CHANGED,
	                                            G_TYPE_FROM_CLASS(klass),
	                                            G_SIGNAL_RUN_LAST,
	                                            G_STRUCT_OFFSET (CustomIndicatorClass, connection_changed),
	                                            NULL, NULL,
	                                            g_cclosure_marshal_VOID__BOOLEAN,
	                                            G_TYPE_NONE, 1, G_TYPE_BOOLEAN, G_TYPE_NONE);

	return;
}

static void
custom_indicator_init (CustomIndicator *self)
{
	CustomIndicatorPrivate * priv = CUSTOM_INDICATOR_GET_PRIVATE(self);
	g_return_if_fail(priv != NULL);

	priv->id = NULL;
	priv->category = CUSTOM_INDICATOR_CATEGORY_OTHER;
	priv->status = CUSTOM_INDICATOR_STATUS_OFF;
	priv->icon_name = NULL;
	priv->attention_icon_name = NULL;
	priv->menu = NULL;

	priv->watcher_proxy = NULL;

	return;
}

static void
custom_indicator_dispose (GObject *object)
{

	G_OBJECT_CLASS (custom_indicator_parent_class)->dispose (object);
	return;
}

static void
custom_indicator_finalize (GObject *object)
{

	G_OBJECT_CLASS (custom_indicator_parent_class)->finalize (object);
	return;
}

static void
custom_indicator_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec)
{
	CustomIndicator * self = CUSTOM_INDICATOR(object);
	g_return_if_fail(self != NULL);

	CustomIndicatorPrivate * priv = CUSTOM_INDICATOR_GET_PRIVATE(self);
	g_return_if_fail(priv != NULL);

	switch (prop_id) {
	case PROP_ID:
		break;
	case PROP_CATEGORY:
		break;
	case PROP_STATUS:
		break;
	case PROP_ICON_NAME:
		break;
	case PROP_ATTENTION_ICON_NAME:
		break;
	case PROP_MENU:
		break;
	case PROP_CONNECTED:
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}

	return;
}

static void
custom_indicator_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec)
{
	CustomIndicator * self = CUSTOM_INDICATOR(object);
	g_return_if_fail(self != NULL);

	CustomIndicatorPrivate * priv = CUSTOM_INDICATOR_GET_PRIVATE(self);
	g_return_if_fail(priv != NULL);

	switch (prop_id) {
	case PROP_ID:
		break;
	case PROP_CATEGORY:
		break;
	case PROP_STATUS:
		break;
	case PROP_ICON_NAME:
		break;
	case PROP_ATTENTION_ICON_NAME:
		break;
	case PROP_MENU:
		break;
	case PROP_CONNECTED:
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}

	return;
}
