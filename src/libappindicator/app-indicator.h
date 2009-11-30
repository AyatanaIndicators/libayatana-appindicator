#ifndef __APP_INDICATOR_H__
#define __APP_INDICATOR_H__

#include <glib.h>
#include <glib-object.h>
#include <libdbusmenu-glib/server.h>

G_BEGIN_DECLS

#define APP_INDICATOR_TYPE            (app_indicator_get_type ())
#define APP_INDICATOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), APP_INDICATOR_TYPE, AppIndicator))
#define APP_INDICATOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), APP_INDICATOR_TYPE, AppIndicatorClass))
#define IS_APP_INDICATOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), APP_INDICATOR_TYPE))
#define IS_APP_INDICATOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), APP_INDICATOR_TYPE))
#define APP_INDICATOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), APP_INDICATOR_TYPE, AppIndicatorClass))

#define APP_INDICATOR_SIGNAL_NEW_ICON            "new-icon"
#define APP_INDICATOR_SIGNAL_NEW_ATTENTION_ICON  "new-attention-icon"
#define APP_INDICATOR_SIGNAL_NEW_STATUS          "new-status"
#define APP_INDICATOR_SIGNAL_CONNECTION_CHANGED  "connection-changed"

/**
	AppIndicatorCategory:
	@APP_INDICATOR_CATEGORY_APPLICATION_STATUS: The indicator is used to display the status of the application.
	@APP_INDICATOR_CATEGORY_COMMUNICATIONS: The application is used for communication with other people.
	@APP_INDICATOR_CATEGORY_SYSTEM_SERVICES: A system indicator relating to something in the user's system.
	@APP_INDICATOR_CATEGORY_HARDWARE: An indicator relating to the user's hardware.
	@APP_INDICATOR_CATEGORY_OTHER: Something not defined in this enum, please don't use unless you really need it.

	The category provides grouping for the indicators so that
	users can find indicators that are similar together.
*/
typedef enum { /*< prefix=APP_INDICATOR_CATEGORY >*/
	APP_INDICATOR_CATEGORY_APPLICATION_STATUS,
	APP_INDICATOR_CATEGORY_COMMUNICATIONS,
	APP_INDICATOR_CATEGORY_SYSTEM_SERVICES,
	APP_INDICATOR_CATEGORY_HARDWARE,
	APP_INDICATOR_CATEGORY_OTHER
} AppIndicatorCategory;

/**
	AppIndicatorStatus:
	@APP_INDICATOR_STATUS_PASSIVE: The indicator should not be shown to the user.
	@APP_INDICATOR_STATUS_ACTIVE: The indicator should be shown in it's default state.
	@APP_INDICATOR_STATUS_ATTENTION: The indicator should show it's attention icon.

	These are the states that the indicator can be on in
	the user's panel.  The indicator by default starts
	in the state @APP_INDICATOR_STATUS_OFF and can be
	shown by setting it to @APP_INDICATOR_STATUS_ON.
*/
typedef enum { /*< prefix=APP_INDICATOR_STATUS >*/
	APP_INDICATOR_STATUS_PASSIVE,
	APP_INDICATOR_STATUS_ACTIVE,
	APP_INDICATOR_STATUS_ATTENTION
} AppIndicatorStatus;

typedef struct _AppIndicator      AppIndicator;
typedef struct _AppIndicatorClass AppIndicatorClass;

/**
	AppIndicatorClass:
	@parent_class: Mia familia
	@new_icon: Slot for #AppIndicator::new-icon.
	@new_attention_icon: Slot for #AppIndicator::new-attention-icon.
	@new_status: Slot for #AppIndicator::new-status.
	@connection_changed: Slot for #AppIndicator::connection-changed.
	@app_indicator_reserved_1: Reserved for future use.
	@app_indicator_reserved_2: Reserved for future use.
	@app_indicator_reserved_3: Reserved for future use.
	@app_indicator_reserved_4: Reserved for future use.

	The signals and external functions that make up the #AppIndicator
	class object.
*/
struct _AppIndicatorClass {
	/* Parent */
	GObjectClass parent_class;

	/* DBus Signals */
	void (* new_icon)               (AppIndicator * indicator,
	                                 gpointer          user_data);
	void (* new_attention_icon)     (AppIndicator * indicator,
	                                 gpointer          user_data);
	void (* new_status)             (AppIndicator * indicator,
	                                 gchar *           status_string,
	                                 gpointer          user_data);

	/* Local Signals */
	void (* connection_changed)     (AppIndicator * indicator,
	                                 gboolean          connected,
	                                 gpointer          user_data);

	/* Reserved */
	void (*app_indicator_reserved_1)(void);
	void (*app_indicator_reserved_2)(void);
	void (*app_indicator_reserved_3)(void);
	void (*app_indicator_reserved_4)(void);
};

/**
	AppIndicator:
	@parent: Parent object.

	A application indicator represents the values that are needed to show a
	unique status in the panel for an application.  In general, applications
	should try to fit in the other indicators that are available on the
	panel before using this.  But, sometimes it is necissary.
*/
struct _AppIndicator {
	GObject parent;
	/* None.  We're a very private object. */
};

/* GObject Stuff */
GType                           app_indicator_get_type           (void);

/* Set properties */
void                            app_indicator_set_id             (AppIndicator * ci,
                                                                  const gchar * id);
void                            app_indicator_set_category       (AppIndicator * ci,
                                                                  AppIndicatorCategory category);
void                            app_indicator_set_status         (AppIndicator * ci,
                                                                  AppIndicatorStatus status);
void                            app_indicator_set_icon           (AppIndicator * ci,
                                                                  const gchar * icon_name);
void                            app_indicator_set_attention_icon (AppIndicator * ci,
                                                                  const gchar * icon_name);
void                            app_indicator_set_menu           (AppIndicator * ci,
                                                                  DbusmenuServer * menu);

/* Get properties */
const gchar *                   app_indicator_get_id             (AppIndicator * ci);
AppIndicatorCategory            app_indicator_get_category       (AppIndicator * ci);
AppIndicatorStatus              app_indicator_get_status         (AppIndicator * ci);
const gchar *                   app_indicator_get_icon           (AppIndicator * ci);
const gchar *                   app_indicator_get_attention_icon (AppIndicator * ci);
DbusmenuServer *                app_indicator_get_menu           (AppIndicator * ci);

G_END_DECLS

#endif
