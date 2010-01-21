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
