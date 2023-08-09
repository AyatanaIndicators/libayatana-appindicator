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
#include <gio/gio.h>
#include "../src/dbus-shared.h"

static GMainLoop * mainloop = NULL;
static gboolean passed = TRUE;
static gboolean watchdog_hit = TRUE;
static gboolean active = FALSE;
static guint toggle_count = 0;

#define PASSIVE_STR  "Passive"
#define ACTIVE_STR   "Active"
#define ATTN_STR     "NeedsAttention"

static GDBusMessage* dbus_reg_filter (GDBusConnection *pConnection, GDBusMessage *pMessage, gboolean bIncoming, void *pUserData)
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

void onNewStatus (GDBusConnection* pConnection, const gchar *sSender, const gchar *sPath, const gchar *sInterface, const gchar *sSignal, GVariant *pParameters, gpointer pUserData)
{
    const gchar *sParam = NULL;
    g_variant_get (pParameters, "(s)", &sParam);

    if (!sParam)
    {
        g_warning ("Couldn't get parameter");

        return;
    }

    watchdog_hit = TRUE;
    int nComp = g_strcmp0 (sParam, ACTIVE_STR);

    if (!nComp)
    {
        if (active)
        {
            g_warning ("Got active when already active");
            passed = FALSE;

            return;
        }

        active = TRUE;
    }
    else
    {
        active = FALSE;
    }

    toggle_count++;

    if (toggle_count == 100)
    {
        g_main_loop_quit (mainloop);
    }
}

gboolean
kill_func (gpointer userdata)
{
    if (watchdog_hit == FALSE) {
        g_main_loop_quit(mainloop);
        g_warning("Forced to Kill");
        g_warning("Toggle count: %d", toggle_count);
        passed = FALSE;
        return FALSE;
    }
    watchdog_hit = FALSE;
    return TRUE;
}

gint
main (gint argc, gchar * argv[])
{
    GError *pError = NULL;
    GDBusProxy *pProxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, NULL, &pError);

    if (pError != NULL)
    {
        g_error("Unable to get session bus: %s", pError->message);
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

    g_dbus_connection_add_filter (pConnection, dbus_reg_filter, NULL, NULL);
    g_dbus_connection_signal_subscribe (pConnection, NULL, NOTIFICATION_ITEM_DBUS_IFACE, "NewStatus", NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE, onNewStatus, NULL, NULL);

    watchdog_hit = TRUE;
    g_timeout_add_seconds(20, kill_func, NULL);

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
