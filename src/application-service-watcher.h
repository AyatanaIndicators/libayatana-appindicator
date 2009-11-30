#ifndef __APPLICATION_SERVICE_WATCHER_H__
#define __APPLICATION_SERVICE_WATCHER_H__

#include <glib.h>
#include <glib-object.h>

#include "application-service-appstore.h"

G_BEGIN_DECLS

#define APPLICATION_SERVICE_WATCHER_TYPE            (application_service_watcher_get_type ())
#define APPLICATION_SERVICE_WATCHER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), APPLICATION_SERVICE_WATCHER_TYPE, ApplicationServiceWatcher))
#define APPLICATION_SERVICE_WATCHER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), APPLICATION_SERVICE_WATCHER_TYPE, ApplicationServiceWatcherClass))
#define IS_APPLICATION_SERVICE_WATCHER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), APPLICATION_SERVICE_WATCHER_TYPE))
#define IS_APPLICATION_SERVICE_WATCHER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), APPLICATION_SERVICE_WATCHER_TYPE))
#define APPLICATION_SERVICE_WATCHER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), APPLICATION_SERVICE_WATCHER_TYPE, ApplicationServiceWatcherClass))

typedef struct _ApplicationServiceWatcher      ApplicationServiceWatcher;
typedef struct _ApplicationServiceWatcherClass ApplicationServiceWatcherClass;

struct _ApplicationServiceWatcherClass {
	GObjectClass parent_class;

	/* Signals */
	void (*service_registered) (ApplicationServiceWatcher * watcher, gchar * object, gpointer data);
	void (*service_unregistered) (ApplicationServiceWatcher * watcher, gchar * object, gpointer data);
	void (*notification_host_registered) (ApplicationServiceWatcher * watcher, gpointer data);
	void (*notification_host_unregistered) (ApplicationServiceWatcher * watcher, gpointer data);
};

struct _ApplicationServiceWatcher {
	GObject parent;
};

GType application_service_watcher_get_type (void);
ApplicationServiceWatcher * application_service_watcher_new (ApplicationServiceAppstore * appstore);

G_END_DECLS

#endif
