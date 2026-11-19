/*
Regression test for the start-up race between the name watcher and the
session bus request in app_indicator_init().

The indicator starts two asynchronous operations: g_bus_watch_name() for
the StatusNotifierWatcher and g_bus_get() for its own connection. Their
order of completion is not fixed. This test makes the order fixed: the
watcher is on the bus before the indicator starts, and the test holds the
g_bus_get() request of the indicator until its name-appeared handler has
run. The indicator must then still register with the watcher.

To hold the request, this program defines g_bus_get() and
g_bus_watch_name(). The dynamic linker uses these definitions for the
calls from libayatana-appindicator. Calls from other libraries go to GIO
unchanged.

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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>
#include <app-indicator.h>
#include "../src/dbus-shared.h"

typedef void (*BusGetFunc) (GBusType, GCancellable *, GAsyncReadyCallback, gpointer);
typedef guint (*BusWatchNameFunc) (GBusType, const gchar *, GBusNameWatcherFlags, GBusNameAppearedCallback, GBusNameVanishedCallback, gpointer, GDestroyNotify);

static const gchar *sWatcherXml =
    "<node>"
    "  <interface name='" NOTIFICATION_WATCHER_DBUS_IFACE "'>"
    "    <property name='IsStatusNotifierHostRegistered' type='b' access='read'/>"
    "    <method name='RegisterStatusNotifierItem'>"
    "      <arg type='s' name='service' direction='in'/>"
    "    </method>"
    "  </interface>"
    "</node>";

static GMainLoop *pMainLoop = NULL;

/* The name-appeared handler of the indicator, and its user data */
static GBusNameAppearedCallback pIndicatorAppeared = NULL;

/* The g_bus_get() request of the indicator, while the test holds it */
static gboolean bBusGetHeld = FALSE;
static GBusType cHeldBusType;
static GCancellable *pHeldCancellable = NULL;
static GAsyncReadyCallback pHeldCallback = NULL;
static gpointer pHeldUserData = NULL;

/* Results */
static gboolean bHeldWhenAppeared = FALSE;
static gchar *sRegisteredPath = NULL;
static gboolean bConnected = FALSE;

static gboolean is_from_appindicator (gpointer pFunction)
{
    Dl_info cInfo;

    if (pFunction == NULL || !dladdr (pFunction, &cInfo) || cInfo.dli_fname == NULL)
    {
        return FALSE;
    }

    return strstr (cInfo.dli_fname, "libayatana-appindicator") != NULL;
}

static BusGetFunc real_bus_get (void)
{
    static BusGetFunc pRealBusGet = NULL;

    if (pRealBusGet == NULL)
    {
        pRealBusGet = (BusGetFunc) dlsym (RTLD_NEXT, "g_bus_get");
    }

    return pRealBusGet;
}

void g_bus_get (GBusType cBusType, GCancellable *pCancellable, GAsyncReadyCallback pCallback, gpointer pUserData)
{
    if (is_from_appindicator (pCallback))
    {
        /* Hold the request. on_name_appeared() sends it. */
        cHeldBusType = cBusType;
        pHeldCancellable = pCancellable;
        pHeldCallback = pCallback;
        pHeldUserData = pUserData;
        bBusGetHeld = TRUE;

        return;
    }

    real_bus_get () (cBusType, pCancellable, pCallback, pUserData);
}

static void on_name_appeared (GDBusConnection *pConnection, const gchar *sName, const gchar *sOwner, gpointer pUserData)
{
    bHeldWhenAppeared = bBusGetHeld;

    pIndicatorAppeared (pConnection, sName, sOwner, pUserData);

    /* Send the held request only after the handler of the indicator ran */
    if (bBusGetHeld)
    {
        bBusGetHeld = FALSE;
        real_bus_get () (cHeldBusType, pHeldCancellable, pHeldCallback, pHeldUserData);
    }
}

guint g_bus_watch_name (GBusType cBusType, const gchar *sName, GBusNameWatcherFlags nFlags, GBusNameAppearedCallback pAppeared, GBusNameVanishedCallback pVanished, gpointer pUserData, GDestroyNotify pDestroy)
{
    static BusWatchNameFunc pRealWatchName = NULL;

    if (pRealWatchName == NULL)
    {
        pRealWatchName = (BusWatchNameFunc) dlsym (RTLD_NEXT, "g_bus_watch_name");
    }

    if (is_from_appindicator (pAppeared) && g_strcmp0 (sName, NOTIFICATION_WATCHER_DBUS_ADDR) == 0)
    {
        pIndicatorAppeared = pAppeared;
        pAppeared = on_name_appeared;
    }

    return pRealWatchName (cBusType, sName, nFlags, pAppeared, pVanished, pUserData, pDestroy);
}

static void check_done (void)
{
    if (sRegisteredPath != NULL && bConnected)
    {
        g_main_loop_quit (pMainLoop);
    }
}

static void watcher_method_call (GDBusConnection *pConnection, const gchar *sSender, const gchar *sPath, const gchar *sInterface, const gchar *sMethod, GVariant *pParams, GDBusMethodInvocation *pInvocation, gpointer pUserData)
{
    if (g_strcmp0 (sMethod, "RegisterStatusNotifierItem") == 0)
    {
        g_free (sRegisteredPath);
        g_variant_get (pParams, "(s)", &sRegisteredPath);
        g_dbus_method_invocation_return_value (pInvocation, NULL);
        check_done ();

        return;
    }

    g_dbus_method_invocation_return_error (pInvocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD, "Unknown method %s", sMethod);
}

static GVariant* watcher_get_property (GDBusConnection *pConnection, const gchar *sSender, const gchar *sPath, const gchar *sInterface, const gchar *sProperty, GError **pError, gpointer pUserData)
{
    if (g_strcmp0 (sProperty, "IsStatusNotifierHostRegistered") == 0)
    {
        return g_variant_new_boolean (TRUE);
    }

    return NULL;
}

static const GDBusInterfaceVTable cWatcherVTable = {
    .method_call = watcher_method_call,
    .get_property = watcher_get_property,
    .set_property = NULL
};

static void on_connection_changed (AppIndicator *pIndicator, gboolean bNowConnected, gpointer pUserData)
{
    bConnected = bNowConnected;
    check_done ();
}

static gboolean on_timeout (gpointer pUserData)
{
    g_main_loop_quit (pMainLoop);

    return G_SOURCE_REMOVE;
}

static void test_watcher_before_bus (void)
{
    GError *pError = NULL;

    /* Put a fake StatusNotifierWatcher on the bus before the indicator starts */
    GDBusConnection *pConnection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &pError);
    g_assert_no_error (pError);

    GDBusNodeInfo *pNodeInfo = g_dbus_node_info_new_for_xml (sWatcherXml, &pError);
    g_assert_no_error (pError);

    guint nRegistration = g_dbus_connection_register_object (pConnection, NOTIFICATION_WATCHER_DBUS_OBJ, pNodeInfo->interfaces[0], &cWatcherVTable, NULL, NULL, &pError);
    g_assert_no_error (pError);

    GVariant *pResult = g_dbus_connection_call_sync (pConnection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "RequestName", g_variant_new ("(su)", NOTIFICATION_WATCHER_DBUS_ADDR, 0), G_VARIANT_TYPE ("(u)"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &pError);
    g_assert_no_error (pError);
    guint nReply = 0;
    g_variant_get (pResult, "(u)", &nReply);
    g_variant_unref (pResult);
    g_assert_cmpuint (nReply, ==, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);

    pMainLoop = g_main_loop_new (NULL, FALSE);

    AppIndicator *pIndicator = g_object_new (APP_INDICATOR_TYPE, "id", "watcher-race", "category", "Other", "icon-name", "bob", NULL);
    g_signal_connect (pIndicator, APP_INDICATOR_SIGNAL_CONNECTION_CHANGED, G_CALLBACK (on_connection_changed), NULL);

    GtkWidget *pMenu = gtk_menu_new ();
    app_indicator_set_menu (pIndicator, GTK_MENU (pMenu));

    guint nTimeout = g_timeout_add_seconds (10, on_timeout, NULL);
    g_main_loop_run (pMainLoop);
    g_source_remove (nTimeout);

    /* The test controlled the order: the name appeared while the bus request of the indicator was held */
    g_assert_nonnull (pIndicatorAppeared);
    g_assert_true (bHeldWhenAppeared);

    /* The indicator registered with the watcher */
    g_assert_cmpstr (sRegisteredPath, ==, "/org/ayatana/NotificationItem/watcher_race");
    g_assert_true (bConnected);

    g_object_unref (pIndicator);
    g_main_loop_unref (pMainLoop);
    g_dbus_connection_unregister_object (pConnection, nRegistration);
    g_dbus_node_info_unref (pNodeInfo);
    g_object_unref (pConnection);
    g_free (sRegisteredPath);
}

int main (int argc, char **argv)
{
    gtk_init (&argc, &argv);
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/indicator/watcher-race/watcher-before-bus", test_watcher_before_bus);

    return g_test_run ();
}
