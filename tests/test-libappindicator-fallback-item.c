#include <glib.h>
#include <glib-object.h>
#include <libappindicator/app-indicator.h>

#define TEST_LIBAPPINDICATOR_FALLBACK_ITEM_TYPE            (test_libappindicator_fallback_item_get_type ())
#define TEST_LIBAPPINDICATOR_FALLBACK_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEST_LIBAPPINDICATOR_FALLBACK_ITEM_TYPE, TestLibappindicatorFallbackItem))
#define TEST_LIBAPPINDICATOR_FALLBACK_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TEST_LIBAPPINDICATOR_FALLBACK_ITEM_TYPE, TestLibappindicatorFallbackItemClass))
#define IS_TEST_LIBAPPINDICATOR_FALLBACK_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEST_LIBAPPINDICATOR_FALLBACK_ITEM_TYPE))
#define IS_TEST_LIBAPPINDICATOR_FALLBACK_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TEST_LIBAPPINDICATOR_FALLBACK_ITEM_TYPE))
#define TEST_LIBAPPINDICATOR_FALLBACK_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TEST_LIBAPPINDICATOR_FALLBACK_ITEM_TYPE, TestLibappindicatorFallbackItemClass))

typedef struct _TestLibappindicatorFallbackItem      TestLibappindicatorFallbackItem;
typedef struct _TestLibappindicatorFallbackItemClass TestLibappindicatorFallbackItemClass;

struct _TestLibappindicatorFallbackItemClass {
	AppIndicatorClass parent_class;

};

struct _TestLibappindicatorFallbackItem {
	AppIndicator parent;

};

GType test_libappindicator_fallback_item_get_type (void);

#define TEST_LIBAPPINDICATOR_FALLBACK_ITEM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), TEST_LIBAPPINDICATOR_FALLBACK_ITEM_TYPE, TestLibappindicatorFallbackItemPrivate))

static void test_libappindicator_fallback_item_class_init (TestLibappindicatorFallbackItemClass *klass);
static void test_libappindicator_fallback_item_init       (TestLibappindicatorFallbackItem *self);
static GtkStatusIcon * fallback (AppIndicator * indicator);
static void unfallback (AppIndicator * indicator, GtkStatusIcon * status_icon);

G_DEFINE_TYPE (TestLibappindicatorFallbackItem, test_libappindicator_fallback_item, APP_INDICATOR_TYPE);

static void
test_libappindicator_fallback_item_class_init (TestLibappindicatorFallbackItemClass *klass)
{
	AppIndicatorClass * aiclass = APP_INDICATOR_CLASS(klass);

	aiclass->fallback = fallback;
	aiclass->unfallback = unfallback;
}

static void
test_libappindicator_fallback_item_init (TestLibappindicatorFallbackItem *self)
{
}

GMainLoop * mainloop = NULL;

static GtkStatusIcon *
fallback (AppIndicator * indicator)
{
	g_debug("Fallback");
	return NULL;
}

static void
unfallback (AppIndicator * indicator, GtkStatusIcon * status_icon)
{
	g_debug("Unfallback");
	return;
}

int
main (int argc, char ** argv)
{
	g_type_init();

	TestLibappindicatorFallbackItem * item = g_object_new(TEST_LIBAPPINDICATOR_FALLBACK_ITEM_TYPE,
		"id", "test-id",
		"category", "other",
		"icon-name", "bob",
		NULL);

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	g_object_unref(G_OBJECT(item));

	return 0;
}
