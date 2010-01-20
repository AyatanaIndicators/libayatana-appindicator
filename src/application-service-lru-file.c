/*
This object manages the database of when the entires were touched
and loved.  And writes that out to disk occationally as well.

Copyright 2010 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>
#include "application-service-lru-file.h"

#define ENTRY_CATEGORY   "category"
#define ENTRY_FIRST_TIME "first-time"
#define ENTRY_LAST_TIME  "last-time"
#define ENTRY_VERSION    "version"

#define CONFIG_DIR       ("indicators" G_DIR_SEPARATOR_S "application")
#define CONFIG_FILE      "lru-file.json"

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
static void get_dirty (AppLruFile * lrufile);
static gboolean load_from_file      (gpointer data);
static void load_file_object_cb (JsonObject * obj, const gchar * key, JsonNode * value, gpointer data);
static void clean_off_hash_cb (gpointer key, gpointer value, gpointer data);
static void clean_off_write_end_cb (GObject * obj, GAsyncResult * res, gpointer data);

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
	priv->filename = g_build_filename(g_get_user_config_dir(), CONFIG_DIR, CONFIG_FILE, NULL);

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
	AppLruFile * lrufile = (AppLruFile *)data;
	AppLruFilePrivate * priv = APP_LRU_FILE_GET_PRIVATE(lrufile);

	if (!g_file_test(priv->filename, G_FILE_TEST_EXISTS)) {
		return FALSE;
	}

	JsonParser * parser = json_parser_new();

	if (!json_parser_load_from_file(parser, priv->filename, NULL)) {
		g_warning("Unable to parse JSON file '%s'", priv->filename);
		g_object_unref(parser);
		return FALSE;
	}

	JsonNode * root = json_parser_get_root(parser);
	JsonObject * rootobj = json_node_get_object(root);
	if (rootobj == NULL) {
		g_warning("Malformed LRU file.  The root node is not an object.");
		g_object_unref(parser);
		return FALSE;
	}

	json_object_foreach_member(rootobj, load_file_object_cb, lrufile);

	g_object_unref(parser);
	return FALSE;
}

/* Looks at the various things that we need on a node, makes
   sure that we have them, and then copies them into the
   application hash table of love. */
static void
load_file_object_cb (JsonObject * rootobj, const gchar * key, JsonNode * value, gpointer data)
{
	AppLruFile * lrufile = (AppLruFile *)data;
	AppLruFilePrivate * priv = APP_LRU_FILE_GET_PRIVATE(lrufile);

	/* We're not looking at this today. */
	if (!g_strcmp0(key, ENTRY_VERSION)) {
		return;
	}

	JsonObject * obj = json_node_get_object(value);
	if (obj == NULL) {
		g_warning("Data for node '%s' is not an object.", key);
		return;
	}

	const gchar * obj_category = json_object_get_string_member(obj, ENTRY_CATEGORY);
	const gchar * obj_first    = json_object_get_string_member(obj, ENTRY_FIRST_TIME);
	const gchar * obj_last     = json_object_get_string_member(obj, ENTRY_LAST_TIME);

	if (obj_category == NULL || obj_first == NULL || obj_last == NULL) {
		g_warning("Node '%s' is missing data.  Got: ('%s', '%s', '%s')", key, obj_category, obj_first, obj_last);
		get_dirty(lrufile);
		return;
	}

	/* Check to see how old this entry is.  If it hasn't been
	   used in the last year, we remove the cruft. */
	GTimeVal currenttime;
	g_get_current_time(&currenttime);
	GDate * currentdate = g_date_new();
	g_date_set_time_val(currentdate, &currenttime);

	GTimeVal lasttouch;
	g_time_val_from_iso8601(obj_last, &lasttouch);
	GDate * lastdate = g_date_new();
	g_date_set_time_val(lastdate, &lasttouch);

	gint spread = g_date_days_between(lastdate, currentdate);

	g_date_free(currentdate);
	g_date_free(lastdate);

	if (spread > 365) {
		g_debug("Removing node '%s' as it's %d days old.", key, spread);
		get_dirty(lrufile);
		return;
	}

	/* See if we already have one of these.  It's a little bit
	   unlikely, but since we're async, we need to check */
	gpointer datapntr = g_hash_table_lookup(priv->apps, key);
	if (datapntr == NULL) {
		/* Build a new node */
		AppData * appdata = g_new0(AppData, 1);
		appdata->category = g_strdup(obj_category);
		g_time_val_from_iso8601(obj_first, &appdata->first_touched);
		g_time_val_from_iso8601(obj_last, &appdata->last_touched);

		g_hash_table_insert(priv->apps, g_strdup(key), appdata);
	} else {
		/* Merge nodes */
		AppData * appdata = (AppData *)datapntr;
		GTimeVal temptime;
		g_time_val_from_iso8601(obj_first, &temptime);

		if (temptime.tv_sec < appdata->first_touched.tv_sec) {
			g_time_val_from_iso8601(obj_first, &appdata->first_touched);
		}
	}

	return;
}

/* Write out our cache to a file so that we can unmark the dirty
   bit and be happy. */
static gboolean
clean_off (gpointer data)
{
	AppLruFile * lrufile = (AppLruFile *)data;
	AppLruFilePrivate * priv = APP_LRU_FILE_GET_PRIVATE(lrufile);
	priv->timer = 0;

	GError * error = NULL;
	
	/* Check to see if our directory exists.  Build it if not. */
	gchar * dirname = g_build_filename(g_get_user_config_dir(), CONFIG_DIR, NULL);
	if (!g_file_test(dirname, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) {
		GFile * dirfile = g_file_new_for_path(dirname);
		g_file_make_directory_with_parents(dirfile, NULL, NULL);
		g_object_unref(dirfile);
	}
	g_free(dirname);

	GFile * file = g_file_new_for_path(priv->filename);
	GFileOutputStream * ostream = g_file_replace(file,
	                                             NULL, /* etag */
	                                             TRUE, /* backup */
	                                             G_FILE_CREATE_NONE, /* flags */
	                                             NULL, /* cancelable */
	                                             &error);
	if (error != NULL) {
		g_warning("Unable to open a file to store LRU file: %s", error->message);
		return FALSE;
	}

	/* This is how the file will start */
	GString * filestring = g_string_new("{\n  \"" ENTRY_VERSION "\": 1");

	/* Put the middle in. */
	g_hash_table_foreach (priv->apps, clean_off_hash_cb, filestring);

	/* And then tack on the end. */
	g_string_append(filestring, "\n}\n");
	gchar * filedata = g_string_free(filestring, FALSE);
	g_output_stream_write_async(G_OUTPUT_STREAM(ostream),
	                            filedata,
	                            strlen(filedata),
	                            G_PRIORITY_DEFAULT_IDLE,
	                            NULL,
	                            clean_off_write_end_cb,
	                            filedata);

	return FALSE; /* drop the timer */
}

/* Looks at every value in the applications hash table and
   turns it into a string for writing out. */
static void
clean_off_hash_cb (gpointer key, gpointer value, gpointer data)
{
	/* Mega-cast */
	gchar * id = (gchar *)key;
	AppData * appdata = (AppData *)value;
	GString * string = (GString *)data;

	gchar * firsttime = g_time_val_to_iso8601(&appdata->first_touched);
	gchar * lasttime = g_time_val_to_iso8601(&appdata->last_touched);

	g_string_append_printf(string, ",\n  \"%s\": { \"" ENTRY_FIRST_TIME "\": \"%s\", \"" ENTRY_LAST_TIME "\": \"%s\", \"" ENTRY_CATEGORY "\": \"%s\"}", id, firsttime, lasttime, appdata->category);

	g_free(lasttime);
	g_free(firsttime);

	return;
}

/* Very much like clean_off_write_cb except that it is the
   last actor on this Output Stream so it closes it. */
static void
clean_off_write_end_cb (GObject * obj, GAsyncResult * res, gpointer data)
{
	g_free(data);

	GError * error = NULL;
	g_output_stream_close(G_OUTPUT_STREAM(obj), NULL, &error);

	if (error != NULL) {
		g_warning("Unable to close LRU File: %s", error->message);
		g_error_free(error);
	}

	return;
}

/* Sets the dirty bit if not already set and makes sure that
   we have a timer to fix that at some point. */
static void
get_dirty (AppLruFile * lrufile)
{
	AppLruFilePrivate * priv = APP_LRU_FILE_GET_PRIVATE(lrufile);

	priv->dirty = TRUE;

	if (priv->timer == 0) {
		priv->timer = g_timeout_add_seconds(60, clean_off, lrufile);
	}

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
	get_dirty(lrufile);
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
