#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "application-service-lru-file.h"

typedef struct _AppLruFilePrivate AppLruFilePrivate;
struct _AppLruFilePrivate {
	GHashTable * apps;
	gboolean dirty;
	guint timer;
	gchar * filename;
};

typedef struct _AppData AppData;
struct _AppData {
	gchar * id;
	gchar * category;
	GTimeVal last_touched;
	GTimeVal first_touched;
};

#define APP_LRU_FILE_GET_PRIVATE(o) \
		(G_TYPE_INSTANCE_GET_PRIVATE ((o), APP_LRU_FILE_TYPE, AppLruFilePrivate))

static void app_lru_file_class_init (AppLruFileClass *klass);
static void app_lru_file_init       (AppLruFile *self);
static void app_lru_file_dispose    (GObject *object);
static void app_lru_file_finalize   (GObject *object);
static void app_data_free           (gpointer data);
static gboolean load_from_file      (gpointer data);

G_DEFINE_TYPE (AppLruFile, app_lru_file, G_TYPE_OBJECT);

/* Set up the class variable stuff */
static void
app_lru_file_class_init (AppLruFileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (AppLruFilePrivate));

	object_class->dispose = app_lru_file_dispose;
	object_class->finalize = app_lru_file_finalize;

	return;
}

/* Set all the data of the per-instance variables */
static void
app_lru_file_init (AppLruFile *self)
{
	AppLruFilePrivate * priv = APP_LRU_FILE_GET_PRIVATE(self);

	/* Default values */
	priv->apps = NULL;
	priv->dirty = FALSE;
	priv->timer = 0;
	priv->filename = NULL;

	/* Now let's build some stuff */
	priv->apps = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, app_data_free);
	priv->filename = g_build_filename(g_get_user_config_dir(), "indicators", "application", "lru-file.json", NULL);

	/* No reason to delay other stuff for this, we'll
	   merge any values that get touched. */
	g_idle_add(load_from_file, self);

	return;
}

/* Get rid of objects and other big things */
static void
app_lru_file_dispose (GObject *object)
{
	AppLruFilePrivate * priv = APP_LRU_FILE_GET_PRIVATE(object);

	if (priv->timer != 0) {
		g_source_remove(priv->timer);
		priv->timer = 0;
	}

	if (priv->apps != NULL) {
		/* Cleans up the keys and entries itself */
		g_hash_table_destroy(priv->apps);
		priv->apps = NULL;
	}

	G_OBJECT_CLASS (app_lru_file_parent_class)->dispose (object);
	return;
}

/* Deallocate memory */
static void
app_lru_file_finalize (GObject *object)
{
	AppLruFilePrivate * priv = APP_LRU_FILE_GET_PRIVATE(object);
	
	if (priv->filename != NULL) {
		g_free(priv->filename);
		priv->filename = NULL;
	}

	G_OBJECT_CLASS (app_lru_file_parent_class)->finalize (object);
	return;
}

/* Takes an AppData structure and free's the
   memory from it. */
static void
app_data_free (gpointer data)
{

	return;
}

/* Loads all of the data out of a json file */
static gboolean
load_from_file (gpointer data)
{

	return FALSE;
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
