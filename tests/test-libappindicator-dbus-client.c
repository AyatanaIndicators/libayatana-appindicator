/*
Tests for the libappindicator library that are over DBus.  This is
the client side of those tests.

Copyright 2009 Canonical Ltd.
Copyright 2023 Robert Tari

Authors:
    Ted Gould <ted@canonical.com>
    Robert Tari <robert@tari.in>

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


#include <glib.h>
#include <app-indicator.h>
#include "test-defines.h"
#include "../src/dbus-shared.h"

static GMainLoop * mainloop = NULL;
static gboolean passed = TRUE;
static int propcount = 0;

static void
check_propcount (void)
{
    if (propcount >= 5) {
        g_main_loop_quit(mainloop);
    }
    return;
}

static void prop_id_cb (GObject *pObject, GAsyncResult *pResult, void *pUserata)
{
    propcount++;
    GError *pError = NULL;
    GVariant *pParams = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObject), pResult, &pError);

    if (pParams)
    {
        gchar *sInterface = NULL;
        gchar *sId = NULL;
        g_variant_get (pParams, "(ss)", &sInterface, &sId);
        gint nComp = g_strcmp0 (TEST_ID, sId);

        if (nComp)
        {
            g_debug ("Property ID Returned: FAILED");
            passed = FALSE;
        }
        else
        {
            g_debug ("Property ID Returned: PASSED");
        }

        g_variant_unref (pParams);
    }

    if (pError)
    {
        g_warning ("Getting ID failed: %s", pError->message);
        g_error_free (pError);
        passed = FALSE;
        check_propcount();

        return;
    }

    check_propcount();

    return;
}

static void prop_category_cb (GObject *pObject, GAsyncResult *pResult, void *pUserata)
{
    propcount++;
    GError *pError = NULL;
    GVariant *pParams = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObject), pResult, &pError);

    if (pParams)
    {
        gchar *sInterface = NULL;
        gchar *sCategory = NULL;
        g_variant_get (pParams, "(ss)", &sInterface, &sCategory);
        gint nComp = g_strcmp0 (TEST_CATEGORY_S, sCategory);

        if (nComp)
        {
            g_debug ("Property category Returned: FAILED");
            passed = FALSE;
        }
        else
        {
            g_debug ("Property category Returned: PASSED");
        }

        g_variant_unref (pParams);
    }

    if (pError)
    {
        g_warning ("Getting category failed: %s", pError->message);
        g_error_free (pError);
        passed = FALSE;
        check_propcount();

        return;
    }

    check_propcount();

    return;
}

static void prop_status_cb (GObject *pObject, GAsyncResult *pResult, void *pUserata)
{
    propcount++;
    GError *pError = NULL;
    GVariant *pParams = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObject), pResult, &pError);

    if (pParams)
    {
        gchar *sInterface = NULL;
        gchar *sStatus = NULL;
        g_variant_get (pParams, "(ss)", &sInterface, &sStatus);
        gint nComp = g_strcmp0 (TEST_STATE_S, sStatus);

        if (nComp)
        {
            g_debug ("Property status Returned: FAILED");
            passed = FALSE;
        }
        else
        {
            g_debug ("Property status Returned: PASSED");
        }

        g_variant_unref (pParams);
    }

    if (pError)
    {
        g_warning ("Getting status failed: %s", pError->message);
        g_error_free (pError);
        passed = FALSE;
        check_propcount();

        return;
    }

    check_propcount();

    return;
}

static void prop_icon_name_cb (GObject *pObject, GAsyncResult *pResult, void *pUserata)
{
    propcount++;
    GError *pError = NULL;
    GVariant *pParams = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObject), pResult, &pError);

    if (pParams)
    {
        gchar *sInterface = NULL;
        gchar *sIcon = NULL;
        g_variant_get (pParams, "(ss)", &sInterface, &sIcon);
        gint nComp = g_strcmp0 (TEST_ICON_NAME, sIcon);

        if (nComp)
        {
            g_debug ("Property icon name Returned: FAILED");
            passed = FALSE;
        }
        else
        {
            g_debug ("Property icon name Returned: PASSED");
        }

        g_variant_unref (pParams);
    }

    if (pError)
    {
        g_warning ("Getting icon name failed: %s", pError->message);
        g_error_free (pError);
        passed = FALSE;
        check_propcount();

        return;
    }

    check_propcount();

    return;
}

static void prop_attention_icon_name_cb (GObject *pObject, GAsyncResult *pResult, void *pUserata)
{
    propcount++;
    GError *pError = NULL;
    GVariant *pParams = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObject), pResult, &pError);

    if (pParams)
    {
        gchar *sInterface = NULL;
        gchar *sAttentionIcon = NULL;
        g_variant_get (pParams, "(ss)", &sInterface, &sAttentionIcon);
        gint nComp = g_strcmp0 (TEST_ATTENTION_ICON_NAME, sAttentionIcon);

        if (nComp)
        {
            g_debug ("Property attention icon name Returned: FAILED");
            passed = FALSE;
        }
        else
        {
            g_debug ("Property attention icon name Returned: PASSED");
        }

        g_variant_unref (pParams);
    }

    if (pError)
    {
        g_warning ("Getting attention icon name failed: %s", pError->message);
        g_error_free (pError);
        passed = FALSE;
        check_propcount();

        return;
    }

    check_propcount();

    return;
}

gboolean
kill_func (gpointer userdata)
{
    g_main_loop_quit(mainloop);
    g_warning("Forced to Kill");
    passed = FALSE;
    return FALSE;
}

static GDBusMessage* dbus_filter (GDBusConnection *pConnection, GDBusMessage *pMessage, gboolean bIncoming, void *pUserData)
{
    GDBusMessageType cType = g_dbus_message_get_message_type (pMessage);
    const gchar *sInterface = g_dbus_message_get_interface (pMessage);
    const gchar *sMember = g_dbus_message_get_member (pMessage);
    int nInterfaceComp = g_strcmp0 (sInterface, NOTIFICATION_WATCHER_DBUS_ADDR);
    int nMemberComp = g_strcmp0 (sMember, "RegisterStatusNotifierItem");

    if (cType == G_DBUS_MESSAGE_TYPE_METHOD_CALL && nInterfaceComp == 0 && nMemberComp == 0)
    {
        GDBusMessage *pReply = g_dbus_message_new_method_reply (pMessage);
        g_dbus_connection_send_message (pConnection, pReply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
        g_object_unref (pReply);
        g_object_unref (pMessage);
        pMessage = NULL;
    }

    return pMessage;
}

gint
main (gint argc, gchar * argv[])
{
    GError *pError = NULL;
    GDBusProxy *pProxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, NULL, &pError);

    if (pError != NULL)
    {
        g_error ("Unable to get session bus: %s", pError->message);
        g_error_free (pError);

        return 1;
    }

    GDBusConnection *pConnection = g_dbus_proxy_get_connection (pProxy);
    guint nName = g_bus_own_name_on_connection (pConnection, NOTIFICATION_WATCHER_DBUS_ADDR, G_BUS_NAME_OWNER_FLAGS_NONE, NULL, NULL, NULL, NULL);

    if (!nName)
    {
        g_error ("Unable to call to request name");

        return 1;
    }

    if (nName != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
    {
        g_error ("Unable to get name");

        return 1;
    }

    g_dbus_connection_add_filter (pConnection, dbus_filter, NULL, NULL);
    g_usleep(500000);
    GDBusProxy *pProps = g_dbus_proxy_new_sync (pConnection, G_DBUS_PROXY_FLAGS_NONE, NULL, ":1.2", "/org/ayatana/NotificationItem/my_id", DBUS_INTERFACE_PROPERTIES, NULL, &pError);

    if (pError != NULL)
    {
        g_error ("Unable to get property proxy: %s", pError->message);
        g_error_free (pError);

        return 1;
    }

    g_dbus_proxy_call (pProps, "Get", g_variant_new ("(ss)", NOTIFICATION_ITEM_DBUS_IFACE, "Id"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback) prop_id_cb, NULL);
    g_dbus_proxy_call (pProps, "Get", g_variant_new ("(ss)", NOTIFICATION_ITEM_DBUS_IFACE, "Category"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback) prop_category_cb, NULL);
    g_dbus_proxy_call (pProps, "Get", g_variant_new ("(ss)", NOTIFICATION_ITEM_DBUS_IFACE, "Status"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback) prop_status_cb, NULL);
    g_dbus_proxy_call (pProps, "Get", g_variant_new ("(ss)", NOTIFICATION_ITEM_DBUS_IFACE, "IconName"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback) prop_icon_name_cb, NULL);
    g_dbus_proxy_call (pProps, "Get", g_variant_new ("(ss)", NOTIFICATION_ITEM_DBUS_IFACE, "AttentionIconName"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback) prop_attention_icon_name_cb, NULL);
    g_timeout_add_seconds(2, kill_func, NULL);

    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    if (passed) {
        g_debug("Quiting");
        return 0;
    } else {
        g_debug("Quiting as we're a failure");
        return 1;
    }
    return 0;
}
