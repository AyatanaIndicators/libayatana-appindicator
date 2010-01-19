#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "application-service-lru-file.h"

typedef struct _AppLruFilePrivate AppLruFilePrivate;
struct _AppLruFilePrivate {
	gint temp;
};

#define APP_LRU_FILE_GET_PRIVATE(o) \
		(G_TYPE_INSTANCE_GET_PRIVATE ((o), APP_LRU_FILE_TYPE, AppLruFilePrivate))

static void app_lru_file_class_init (AppLruFileClass *klass);
static void app_lru_file_init       (AppLruFile *self);
static void app_lru_file_dispose    (GObject *object);
static void app_lru_file_finalize   (GObject *object);

G_DEFINE_TYPE (AppLruFile, app_lru_file, G_TYPE_OBJECT);

static void
app_lru_file_class_init (AppLruFileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (AppLruFilePrivate));

	object_class->dispose = app_lru_file_dispose;
	object_class->finalize = app_lru_file_finalize;

	return;
}

static void
app_lru_file_init (AppLruFile *self)
{

	return;
}

static void
app_lru_file_dispose (GObject *object)
{

	G_OBJECT_CLASS (app_lru_file_parent_class)->dispose (object);
	return;
}

static void
app_lru_file_finalize (GObject *object)
{

	G_OBJECT_CLASS (app_lru_file_parent_class)->finalize (object);
	return;
}
