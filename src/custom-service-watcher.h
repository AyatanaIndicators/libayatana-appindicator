#ifndef __CUSTOM_SERVICE_WATCHER_H__
#define __CUSTOM_SERVICE_WATCHER_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define CUSTOM_SERVICE_WATCHER_TYPE            (custom_service_watcher_get_type ())
#define CUSTOM_SERVICE_WATCHER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CUSTOM_SERVICE_WATCHER_TYPE, CustomServiceWatcher))
#define CUSTOM_SERVICE_WATCHER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CUSTOM_SERVICE_WATCHER_TYPE, CustomServiceWatcherClass))
#define IS_CUSTOM_SERVICE_WATCHER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CUSTOM_SERVICE_WATCHER_TYPE))
#define IS_CUSTOM_SERVICE_WATCHER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CUSTOM_SERVICE_WATCHER_TYPE))
#define CUSTOM_SERVICE_WATCHER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CUSTOM_SERVICE_WATCHER_TYPE, CustomServiceWatcherClass))

typedef struct _CustomServiceWatcher      CustomServiceWatcher;
typedef struct _CustomServiceWatcherClass CustomServiceWatcherClass;

struct _CustomServiceWatcherClass {
	GObjectClass parent_class;
};

struct _CustomServiceWatcher {
	GObject parent;
};

GType custom_service_watcher_get_type (void);

G_END_DECLS

#endif
