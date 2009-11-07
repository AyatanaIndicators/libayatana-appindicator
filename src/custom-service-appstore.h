#ifndef __CUSTOM_SERVICE_APPSTORE_H__
#define __CUSTOM_SERVICE_APPSTORE_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define CUSTOM_SERVICE_APPSTORE_TYPE            (custom_service_appstore_get_type ())
#define CUSTOM_SERVICE_APPSTORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CUSTOM_SERVICE_APPSTORE_TYPE, CustomServiceAppstore))
#define CUSTOM_SERVICE_APPSTORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CUSTOM_SERVICE_APPSTORE_TYPE, CustomServiceAppstoreClass))
#define IS_CUSTOM_SERVICE_APPSTORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CUSTOM_SERVICE_APPSTORE_TYPE))
#define IS_CUSTOM_SERVICE_APPSTORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CUSTOM_SERVICE_APPSTORE_TYPE))
#define CUSTOM_SERVICE_APPSTORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CUSTOM_SERVICE_APPSTORE_TYPE, CustomServiceAppstoreClass))

typedef struct _CustomServiceAppstore      CustomServiceAppstore;
typedef struct _CustomServiceAppstoreClass CustomServiceAppstoreClass;

struct _CustomServiceAppstoreClass {
	GObjectClass parent_class;
};

struct _CustomServiceAppstore {
	GObject parent;

	void (*application_added) (CustomServiceAppstore * appstore, gchar *, gint, gchar *, gchar *, gpointer);
	void (*application_removed) (CustomServiceAppstore * appstore, gint, gpointer);
};

GType custom_service_appstore_get_type               (void);
void  custom_service_appstore_application_add        (CustomServiceAppstore *   appstore,
                                                      const gchar *             dbus_name,
                                                      const gchar *             dbus_object);
void  custom_service_appstore_application_remove     (CustomServiceAppstore *   appstore,
                                                      const gchar *             dbus_name,
                                                      const gchar *             dbus_object);

G_END_DECLS

#endif
