#ifndef __APPLICATION_INDICATOR_H__
#define __APPLICATION_INDICATOR_H__

#include <glib.h>
#include <glib-object.h>
#include <libdbusmenu-glib/server.h>

G_BEGIN_DECLS

#define APPLICATION_INDICATOR_TYPE            (application_indicator_get_type ())
#define APPLICATION_INDICATOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), APPLICATION_INDICATOR_TYPE, ApplicationIndicator))
#define APPLICATION_INDICATOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), APPLICATION_INDICATOR_TYPE, ApplicationIndicatorClass))
#define IS_APPLICATION_INDICATOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), APPLICATION_INDICATOR_TYPE))
#define IS_APPLICATION_INDICATOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), APPLICATION_INDICATOR_TYPE))
#define APPLICATION_INDICATOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), APPLICATION_INDICATOR_TYPE, ApplicationIndicatorClass))

#define APPLICATION_INDICATOR_SIGNAL_NEW_ICON            "new-icon"
#define APPLICATION_INDICATOR_SIGNAL_NEW_ATTENTION_ICON  "new-attention-icon"
#define APPLICATION_INDICATOR_SIGNAL_NEW_STATUS          "new-status"
#define APPLICATION_INDICATOR_SIGNAL_CONNECTION_CHANGED  "connection-changed"

/**
	ApplicationIndicatorCategory:
	@APPLICATION_INDICATOR_CATEGORY_APPLICATION_STATUS: The indicator is used to display the status of the application.
	@APPLICATION_INDICATOR_CATEGORY_COMMUNICATIONS: The application is used for communication with other people.
	@APPLICATION_INDICATOR_CATEGORY_SYSTEM_SERVICES: A system indicator relating to something in the user's system.
	@APPLICATION_INDICATOR_CATEGORY_HARDWARE: An indicator relating to the user's hardware.
	@APPLICATION_INDICATOR_CATEGORY_OTHER: Something not defined in this enum, please don't use unless you really need it.

	The category provides grouping for the indicators so that
	users can find indicators that are similar together.
*/
typedef enum { /*< prefix=APPLICATION_INDICATOR_CATEGORY >*/
	APPLICATION_INDICATOR_CATEGORY_APPLICATION_STATUS,
	APPLICATION_INDICATOR_CATEGORY_COMMUNICATIONS,
	APPLICATION_INDICATOR_CATEGORY_SYSTEM_SERVICES,
	APPLICATION_INDICATOR_CATEGORY_HARDWARE,
	APPLICATION_INDICATOR_CATEGORY_OTHER
} ApplicationIndicatorCategory;

/**
	ApplicationIndicatorStatus:
	@APPLICATION_INDICATOR_STATUS_PASSIVE: The indicator should not be shown to the user.
	@APPLICATION_INDICATOR_STATUS_ACTIVE: The indicator should be shown in it's default state.
	@APPLICATION_INDICATOR_STATUS_ATTENTION: The indicator should show it's attention icon.

	These are the states that the indicator can be on in
	the user's panel.  The indicator by default starts
	in the state @APPLICATION_INDICATOR_STATUS_OFF and can be
	shown by setting it to @APPLICATION_INDICATOR_STATUS_ON.
*/
typedef enum { /*< prefix=APPLICATION_INDICATOR_STATUS >*/
	APPLICATION_INDICATOR_STATUS_PASSIVE,
	APPLICATION_INDICATOR_STATUS_ACTIVE,
	APPLICATION_INDICATOR_STATUS_ATTENTION
} ApplicationIndicatorStatus;

typedef struct _ApplicationIndicator      ApplicationIndicator;
typedef struct _ApplicationIndicatorClass ApplicationIndicatorClass;

/**
	ApplicationIndicatorClass:
	@parent_class: Mia familia
	@new_icon: Slot for #ApplicationIndicator::new-icon.
	@new_attention_icon: Slot for #ApplicationIndicator::new-attention-icon.
	@new_status: Slot for #ApplicationIndicator::new-status.
	@connection_changed: Slot for #ApplicationIndicator::connection-changed.
	@application_indicator_reserved_1: Reserved for future use.
	@application_indicator_reserved_2: Reserved for future use.
	@application_indicator_reserved_3: Reserved for future use.
	@application_indicator_reserved_4: Reserved for future use.

	The signals and external functions that make up the #ApplicationIndicator
	class object.
*/
struct _ApplicationIndicatorClass {
	/* Parent */
	GObjectClass parent_class;

	/* DBus Signals */
	void (* new_icon)               (ApplicationIndicator * indicator,
	                                 gpointer          user_data);
	void (* new_attention_icon)     (ApplicationIndicator * indicator,
	                                 gpointer          user_data);
	void (* new_status)             (ApplicationIndicator * indicator,
	                                 gchar *           status_string,
	                                 gpointer          user_data);

	/* Local Signals */
	void (* connection_changed)     (ApplicationIndicator * indicator,
	                                 gboolean          connected,
	                                 gpointer          user_data);

	/* Reserved */
	void (*application_indicator_reserved_1)(void);
	void (*application_indicator_reserved_2)(void);
	void (*application_indicator_reserved_3)(void);
	void (*application_indicator_reserved_4)(void);
};

/**
	ApplicationIndicator:
	@parent: Parent object.

	A application indicator represents the values that are needed to show a
	unique status in the panel for an application.  In general, applications
	should try to fit in the other indicators that are available on the
	panel before using this.  But, sometimes it is necissary.
*/
struct _ApplicationIndicator {
	GObject parent;
	/* None.  We're a very private object. */
};

/* GObject Stuff */
GType                           application_indicator_get_type           (void);

/* Set properties */
void                            application_indicator_set_id             (ApplicationIndicator * ci,
                                                                          const gchar * id);
void                            application_indicator_set_category       (ApplicationIndicator * ci,
                                                                          ApplicationIndicatorCategory category);
void                            application_indicator_set_status         (ApplicationIndicator * ci,
                                                                          ApplicationIndicatorStatus status);
void                            application_indicator_set_icon           (ApplicationIndicator * ci,
                                                                          const gchar * icon_name);
void                            application_indicator_set_attention_icon (ApplicationIndicator * ci,
                                                                          const gchar * icon_name);
void                            application_indicator_set_menu           (ApplicationIndicator * ci,
                                                                          DbusmenuServer * menu);

/* Get properties */
const gchar *                   application_indicator_get_id             (ApplicationIndicator * ci);
ApplicationIndicatorCategory    application_indicator_get_category       (ApplicationIndicator * ci);
ApplicationIndicatorStatus      application_indicator_get_status         (ApplicationIndicator * ci);
const gchar *                   application_indicator_get_icon           (ApplicationIndicator * ci);
const gchar *                   application_indicator_get_attention_icon (ApplicationIndicator * ci);
DbusmenuServer *                application_indicator_get_menu           (ApplicationIndicator * ci);

G_END_DECLS

#endif
