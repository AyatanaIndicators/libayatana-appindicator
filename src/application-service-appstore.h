#ifndef __APPLICATION_SERVICE_APPSTORE_H__
#define __APPLICATION_SERVICE_APPSTORE_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define APPLICATION_SERVICE_APPSTORE_TYPE            (application_service_appstore_get_type ())
#define APPLICATION_SERVICE_APPSTORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), APPLICATION_SERVICE_APPSTORE_TYPE, ApplicationServiceAppstore))
#define APPLICATION_SERVICE_APPSTORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), APPLICATION_SERVICE_APPSTORE_TYPE, ApplicationServiceAppstoreClass))
#define IS_APPLICATION_SERVICE_APPSTORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), APPLICATION_SERVICE_APPSTORE_TYPE))
#define IS_APPLICATION_SERVICE_APPSTORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), APPLICATION_SERVICE_APPSTORE_TYPE))
#define APPLICATION_SERVICE_APPSTORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), APPLICATION_SERVICE_APPSTORE_TYPE, ApplicationServiceAppstoreClass))

typedef struct _ApplicationServiceAppstore      ApplicationServiceAppstore;
typedef struct _ApplicationServiceAppstoreClass ApplicationServiceAppstoreClass;

struct _ApplicationServiceAppstoreClass {
	GObjectClass parent_class;
};

struct _ApplicationServiceAppstore {
	GObject parent;

	void (*application_added) (ApplicationServiceAppstore * appstore, gchar *, gint, gchar *, gchar *, gpointer);
	void (*application_removed) (ApplicationServiceAppstore * appstore, gint, gpointer);
};

GType application_service_appstore_get_type               (void);
void  application_service_appstore_application_add        (ApplicationServiceAppstore *   appstore,
                                                      const gchar *             dbus_name,
                                                      const gchar *             dbus_object);
void  application_service_appstore_application_remove     (ApplicationServiceAppstore *   appstore,
                                                      const gchar *             dbus_name,
                                                      const gchar *             dbus_object);

G_END_DECLS

#endif
