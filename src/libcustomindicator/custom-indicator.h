#ifndef __CUSTOM_INDICATOR_H__
#define __CUSTOM_INDICATOR_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define CUSTOM_INDICATOR_TYPE            (custom_indicator_get_type ())
#define CUSTOM_INDICATOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CUSTOM_INDICATOR_TYPE, CustomIndicator))
#define CUSTOM_INDICATOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CUSTOM_INDICATOR_TYPE, CustomIndicatorClass))
#define IS_CUSTOM_INDICATOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CUSTOM_INDICATOR_TYPE))
#define IS_CUSTOM_INDICATOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CUSTOM_INDICATOR_TYPE))
#define CUSTOM_INDICATOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CUSTOM_INDICATOR_TYPE, CustomIndicatorClass))

typedef struct _CustomIndicator      CustomIndicator;
typedef struct _CustomIndicatorClass CustomIndicatorClass;

struct _CustomIndicatorClass {
	GObjectClass parent_class;
};

struct _CustomIndicator {
	GObject parent;
};

GType custom_indicator_get_type (void);

G_END_DECLS

#endif
