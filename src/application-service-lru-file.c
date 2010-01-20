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
	AppData * appdata = (AppData *)data;

	if (appdata->category != NULL) {
		g_free(appdata->category);
	}

	g_free(appdata);

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

	AppData * appdata = NULL;
	AppLruFilePrivate * priv = APP_LRU_FILE_GET_PRIVATE(lrufile);
	gpointer data = g_hash_table_lookup(priv->apps, id);

	if (data == NULL) {
		/* Oh, we don't have one, let's build it and put it
		   into the hash table ourselves */
		appdata = g_new0(AppData, 1);

		appdata->category = g_strdup(category);
		g_get_current_time(&(appdata->first_touched));
		/* NOTE: last touched set below */

		g_hash_table_insert(priv->apps, g_strdup(id), appdata);
	} else {
		/* Boring, we've got this one already */
		appdata = (AppData *)data;
	}

	/* Touch it and mark the DB as dirty */
	g_get_current_time(&(appdata->last_touched));
	/* TODO: Make dirty */
	return;
}

/* Used to sort or compare different applications. */
gint
app_lru_file_sort (AppLruFile * lrufile, const gchar * id_a, const gchar * id_b)
{
	g_return_val_if_fail(IS_APP_LRU_FILE(lrufile), -1);

	/* Let's first look to see if the IDs are the same, this
	   really shouldn't happen, but it'll be confusing if we
	   don't get it out of the way to start. */
	if (g_strcmp0(id_a, id_b) == 0) {
		return 0;
	}

	AppLruFilePrivate * priv = APP_LRU_FILE_GET_PRIVATE(lrufile);

	/* Now make sure we have app data for both of these.  If
	   not we'll assume that the one without is newer? */
	gpointer data_a = g_hash_table_lookup(priv->apps, id_a);
	if (data_a == NULL) {
		return -1;
	}

	gpointer data_b = g_hash_table_lookup(priv->apps, id_b);
	if (data_b == NULL) {
		return 1;
	}

	/* Ugly casting */
	AppData * appdata_a = (AppData *)data_a;
	AppData * appdata_b = (AppData *)data_b;

	/* Look at categories, we'll put the categories in alpha
	   order if they're not the same. */
	gint catcompare = g_strcmp0(appdata_a->category, appdata_b->category);
	if (catcompare != 0) {
		return catcompare;
	}

	/* Now we're looking at the first time that these two were
	   seen in this account.  Only using seconds.  I mean, seriously. */
	if (appdata_a->first_touched.tv_sec < appdata_b->first_touched.tv_sec) {
		return -1;
	}
	if (appdata_b->first_touched.tv_sec < appdata_a->first_touched.tv_sec) {
		return 1;
	}

	/* Eh, this seems roughly impossible.  But if we have to choose,
	   I like A better. */
	return 1;
}
