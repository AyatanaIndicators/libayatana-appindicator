#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "libcustomindicator/custom-indicator.h"
#include "libcustomindicator/custom-indicator-enum-types.h"

typedef struct _CustomIndicatorPrivate CustomIndicatorPrivate;
struct _CustomIndicatorPrivate {
	int placeholder;
};

enum properties {
	PROP_0,
	PROP_STATUS
};

#define CUSTOM_INDICATOR_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), CUSTOM_INDICATOR_TYPE, CustomIndicatorPrivate))

static void custom_indicator_class_init (CustomIndicatorClass *klass);
static void custom_indicator_init       (CustomIndicator *self);
static void custom_indicator_dispose    (GObject *object);
static void custom_indicator_finalize   (GObject *object);
static void custom_indicator_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void custom_indicator_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);

G_DEFINE_TYPE (CustomIndicator, custom_indicator, G_TYPE_OBJECT);

static void
custom_indicator_class_init (CustomIndicatorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (CustomIndicatorPrivate));

	object_class->dispose = custom_indicator_dispose;
	object_class->finalize = custom_indicator_finalize;

	object_class->set_property = custom_indicator_set_property;
	object_class->get_property = custom_indicator_get_property;

	g_object_class_install_property(object_class, PROP_STATUS,
	                                g_param_spec_enum("status",
	                                                  "Indicator Status",
	                                                  "Whether the indicator is shown or requests attention.",
	                                                  CUSTOM_INDICATOR_TYPE_CUSTOM_INDICATOR_STATUS_T,
	                                                  CUSTOM_INDICATOR_STATUS_OFF,
	                                                  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	return;
}

static void
custom_indicator_init (CustomIndicator *self)
{

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


	return;
}

static void
custom_indicator_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec)
{


	return;
}
