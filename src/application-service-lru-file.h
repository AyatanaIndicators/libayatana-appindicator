#ifndef __APP_LRU_FILE_H__
#define __APP_LRU_FILE_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define APP_LRU_FILE_TYPE            (app_lru_file_get_type ())
#define APP_LRU_FILE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), APP_LRU_FILE_TYPE, AppLruFile))
#define APP_LRU_FILE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), APP_LRU_FILE_TYPE, AppLruFileClass))
#define IS_APP_LRU_FILE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), APP_LRU_FILE_TYPE))
#define IS_APP_LRU_FILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), APP_LRU_FILE_TYPE))
#define APP_LRU_FILE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), APP_LRU_FILE_TYPE, AppLruFileClass))

typedef struct _AppLruFile      AppLruFile;
typedef struct _AppLruFileClass AppLruFileClass;

struct _AppLruFileClass {
	GObjectClass parent_class;

};

struct _AppLruFile {
	GObject parent;

};

GType app_lru_file_get_type (void);

AppLruFile * app_lru_file_new (void);
void app_lru_file_touch (AppLruFile * lrufile, const gchar * id, const gchar * category);
gint app_lru_file_sort  (AppLruFile * lrufile, const gchar * id_a, const gchar * id_b);

G_END_DECLS

#endif
