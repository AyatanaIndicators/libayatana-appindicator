/*
This puts the NotificationWatcher on the bus, kinda.  Enough to
trick the Item into unfalling back.

Copyright 2010 Canonical Ltd.
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

gboolean kill_func (gpointer userdata);

static GMainLoop * mainloop = NULL;

static GDBusMessage* dbus_filter (GDBusConnection *pConnection, GDBusMessage *pMessage, gboolean bIncoming, void *pUserData)
{
    GDBusMessageType cType = g_dbus_message_get_message_type (pMessage);
    const gchar *sMember = g_dbus_message_get_member (pMessage);
    int nMemberComp = g_strcmp0 (sMember, "RegisterStatusNotifierItem");

    if (cType == G_DBUS_MESSAGE_TYPE_METHOD_CALL && nMemberComp == 0)
    {
        GDBusMessage *pReply = g_dbus_message_new_method_reply (pMessage);
        g_dbus_connection_send_message (pConnection, pReply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
        g_object_unref (pReply);
        g_timeout_add (50, kill_func, NULL);
        g_object_unref (pMessage);
        pMessage = NULL;
    }

    return pMessage;
}

gboolean
kill_func (gpointer userdata)
{
    g_main_loop_quit(mainloop);
    return FALSE;
}

int
main (int argv, char ** argc)
{
    g_debug("Waiting to init.");

    GError *pError = NULL;
    GDBusProxy * pProxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, NULL, &pError);

    if (pError != NULL)
    {
        g_error("Unable to get session bus: %s", pError->message);
        g_error_free (pError);

        return 1;
    }

    g_usleep (500000);
    gboolean bOwner = FALSE;
    GVariant *pResult = g_dbus_proxy_call_sync (pProxy, "NameHasOwner", g_variant_new ("(s)", "org.test"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &pError);

    if (pResult)
    {
        g_variant_get (pResult, "(b)", &bOwner);
        g_variant_unref (pResult);
    }
    else
    {
        g_error ("Error getting name owner: %s", pError->message);
        g_error_free (pError);

        return 1;
    }

    if (!bOwner)
    {
        g_error ("Unable to get name owner");

        return 1;
    }

    g_usleep(500000);
    g_debug("Initing");

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

    /* This is the final kill function.  It really shouldn't happen
       unless we get an error. */
    g_timeout_add_seconds(20, kill_func, NULL);

    g_debug("Entering Mainloop");

    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    g_debug("Exiting");

    return 0;
}
