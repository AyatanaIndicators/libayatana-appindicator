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

/* API */

/* Simple helper to create a new object */
AppLruFile *
app_lru_file_new (void)
{
	return APP_LRU_FILE(g_object_new(APP_LRU_FILE_TYPE, NULL));
}

/* This updates the timestamp for a particular
   entry in the database.  It also queues up a 
   write out of the database.  But that'll happen
   later. */
void
app_lru_file_touch (AppLruFile * lrufile, const gchar * id, const gchar * category)
{
	g_return_if_fail(IS_APP_LRU_FILE(lrufile));
	g_return_if_fail(id != NULL && id[0] != '\0');
	g_return_if_fail(category != NULL && category[0] != '\0');

	return;
}

/* Used to sort or compare different applications. */
gint
app_lru_file_sort (AppLruFile * lrufile, const gchar * id_a, const gchar * id_b)
{
	g_return_val_if_fail(IS_APP_LRU_FILE(lrufile), -1);


	return 0;
}
