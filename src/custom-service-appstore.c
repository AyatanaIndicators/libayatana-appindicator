#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "custom-service-appstore.h"

typedef struct _CustomServiceAppstorePrivate CustomServiceAppstorePrivate;

struct _CustomServiceAppstorePrivate {
	int demo;
};

#define CUSTOM_SERVICE_APPSTORE_GET_PRIVATE(o) \
			(G_TYPE_INSTANCE_GET_PRIVATE ((o), CUSTOM_SERVICE_APPSTORE_TYPE, CustomServiceAppstorePrivate))

static void custom_service_appstore_class_init (CustomServiceAppstoreClass *klass);
static void custom_service_appstore_init       (CustomServiceAppstore *self);
static void custom_service_appstore_dispose    (GObject *object);
static void custom_service_appstore_finalize   (GObject *object);

G_DEFINE_TYPE (CustomServiceAppstore, custom_service_appstore, G_TYPE_OBJECT);

static void
custom_service_appstore_class_init (CustomServiceAppstoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (CustomServiceAppstorePrivate));

	object_class->dispose = custom_service_appstore_dispose;
	object_class->finalize = custom_service_appstore_finalize;

	return;
}

static void
custom_service_appstore_init (CustomServiceAppstore *self)
{

	return;
}

static void
custom_service_appstore_dispose (GObject *object)
{

	G_OBJECT_CLASS (custom_service_appstore_parent_class)->dispose (object);
	return;
}

static void
custom_service_appstore_finalize (GObject *object)
{

	G_OBJECT_CLASS (custom_service_appstore_parent_class)->finalize (object);
	return;
}
