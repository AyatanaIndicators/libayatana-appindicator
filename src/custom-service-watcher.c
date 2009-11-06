#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "custom-service-watcher.h"

typedef struct _CustomServiceWatcherPrivate CustomServiceWatcherPrivate;
struct _CustomServiceWatcherPrivate {
	int dummy;
};

#define CUSTOM_SERVICE_WATCHER_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), CUSTOM_SERVICE_WATCHER_TYPE, CustomServiceWatcherPrivate))

static void custom_service_watcher_class_init (CustomServiceWatcherClass *klass);
static void custom_service_watcher_init       (CustomServiceWatcher *self);
static void custom_service_watcher_dispose    (GObject *object);
static void custom_service_watcher_finalize   (GObject *object);

G_DEFINE_TYPE (CustomServiceWatcher, custom_service_watcher, G_TYPE_OBJECT);

static void
custom_service_watcher_class_init (CustomServiceWatcherClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (CustomServiceWatcherPrivate));

	object_class->dispose = custom_service_watcher_dispose;
	object_class->finalize = custom_service_watcher_finalize;

	return;
}

static void
custom_service_watcher_init (CustomServiceWatcher *self)
{

	return;
}

static void
custom_service_watcher_dispose (GObject *object)
{

	G_OBJECT_CLASS (custom_service_watcher_parent_class)->dispose (object);
	return;
}

static void
custom_service_watcher_finalize (GObject *object)
{

	G_OBJECT_CLASS (custom_service_watcher_parent_class)->finalize (object);
	return;
}
