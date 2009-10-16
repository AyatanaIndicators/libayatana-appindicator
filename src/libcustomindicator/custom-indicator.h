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

typedef int custom_indicator_category_t;
typedef int custom_indicator_status_t;

typedef struct _CustomIndicator      CustomIndicator;
typedef struct _CustomIndicatorClass CustomIndicatorClass;

struct _CustomIndicatorClass {
	GObjectClass parent_class;
};

struct _CustomIndicator {
	GObject parent;
};

/* GObject Stuff */
GType                           custom_indicator_get_type           (void);

/* Set properties */
void                            custom_indicator_set_id             (CustomIndicator * ci,
                                                                     const gchar * id);
void                            custom_indicator_set_category       (CustomIndicator * ci,
                                                                     custom_indicator_category_t category);
void                            custom_indicator_set_status         (CustomIndicator * ci,
                                                                     custom_indicator_status_t status);
void                            custom_indicator_set_icon           (CustomIndicator * ci,
                                                                     const gchar * icon_name);
void                            custom_indicator_set_attention_icon (CustomIndicator * ci,
                                                                     const gchar * icon_name);
void                            custom_indicator_set_menu           (CustomIndicator * ci,
                                                                     void * menu);

/* Get properties */
const gchar *                   custom_indicator_get_id             (CustomIndicator * ci);
custom_indicator_category_t     custom_indicator_get_category       (CustomIndicator * ci);
custom_indicator_status_t       custom_indicator_get_status         (CustomIndicator * ci);
const gchar *                   custom_indicator_get_icon           (CustomIndicator * ci);
const gchar *                   custom_indicator_get_attention_icon (CustomIndicator * ci);
void *                          custom_indicator_get_menu           (CustomIndicator * ci);

G_END_DECLS

#endif
