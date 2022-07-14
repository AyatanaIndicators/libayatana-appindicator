/*
A small piece of sample code demonstrating a very simple application
with an indicator.

Copyright 2011 Canonical Ltd.
Copyright 2022 Robert Tari

Authors:
    Marco Trevisan <mail@3v1n0.net>
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

using Gtk;
using AppIndicator;

static int main(string[] args) {
    var sc = new SimpleClient(args);
    sc.run();
    return 0;
}

class SimpleClient {
    Gtk.Menu menu;
    Indicator ci;
    int percentage;
    bool active;
    bool can_haz_label;

    public SimpleClient(string[] args) {
        Gtk.init(ref args);

        ci = new Indicator("example-simple-client",
                           "indicator-messages",
                           IndicatorCategory.APPLICATION_STATUS);

        ci.set_status(IndicatorStatus.ACTIVE);
        ci.set_attention_icon_full("indicator-messages-new", null);
        ci.set_label("1%", "100%");
        ci.set_title("Test Indicator (vala)");

        active = true;
        can_haz_label = true;
    }

    private void toggle_sensitivity(Widget widget) {
        widget.set_sensitive(!widget.is_sensitive());
    }

    private void append_submenu(Gtk.MenuItem item) {
        var menu = new Gtk.Menu();
        Gtk.MenuItem mi;

        mi = new Gtk.MenuItem.with_label("Sub 1");
        menu.append(mi);
        mi.activate.connect(() => { print("Sub1\n"); });

        Gtk.MenuItem prev_mi = mi;
        mi = new Gtk.MenuItem.with_label("Sub 2");
        menu.append(mi);
        mi.activate.connect(() => { toggle_sensitivity(prev_mi); });

        mi = new Gtk.MenuItem.with_label("Sub 3");
        menu.append(mi);
        mi.activate.connect(() => { print("Sub3\n"); });

        mi = new Gtk.MenuItem.with_label("Toggle Attention");
        menu.append(mi);
        mi.activate.connect(() => {
            if (ci.get_status() == IndicatorStatus.ATTENTION)
                ci.set_status(IndicatorStatus.ACTIVE);
            else
                ci.set_status(IndicatorStatus.ATTENTION);
        });

        ci.set_secondary_activate_target(mi);

        menu.show_all();
        item.set_submenu(menu);
    }

    private void label_toggle(Gtk.MenuItem item) {
        can_haz_label = !can_haz_label;

        if (can_haz_label) {
            item.set_label("Hide label");
        } else {
            item.set_label("Show label");
        }
    }

    public void run() {

        ci.scroll_event.connect((delta, direction) => {
            print(@"Got scroll event! delta: $delta, direction: $direction\n");
        });

        GLib.Timeout.add_seconds(1, () => {
            percentage = (percentage + 1) % 100;
            if (can_haz_label) {
                ci.set_label(@"$(percentage+1)%", "");
            } else {
                ci.set_label("", "");
            }
            return true;
        });

        menu = new Gtk.Menu();
        var chk = new CheckMenuItem.with_label("1");
        chk.activate.connect(() => { print("1\n"); });
        menu.append(chk);
        chk.show();

        var radio = new Gtk.RadioMenuItem.with_label(new SList<RadioMenuItem>(), "2");
        radio.activate.connect(() => { print("2\n"); });
        menu.append(radio);
        radio.show();

        var submenu = new Gtk.MenuItem.with_label("3");
        menu.append(submenu);
        append_submenu(submenu);
        submenu.show();

        var toggle_item = new Gtk.MenuItem.with_label("Toggle 3");
        toggle_item.activate.connect(() => { toggle_sensitivity(submenu); });
        menu.append(toggle_item);
        toggle_item.show();

        var pImage = new Gtk.Image.from_icon_name("document-new", IconSize.MENU);
        var pLabel = new Gtk.Label("New");
        var pGrid = new Gtk.Grid();
        pGrid.add(pImage);
        pGrid.add(pLabel);
        var imgitem = new Gtk.MenuItem();
        imgitem.add(pGrid);
        imgitem.activate.connect(() => {
            Grid pGridLocal = (Grid)imgitem.get_children().nth_data(0);
            Image img = (Image) pGridLocal.get_children().nth_data(1);
            img.set_from_icon_name("document-open", IconSize.MENU);
        });
        menu.append(imgitem);
        imgitem.show();

        var att = new Gtk.MenuItem.with_label("Get Attention");
        att.activate.connect(() => {
            if (active) {
                ci.set_status(IndicatorStatus.ATTENTION);
                att.set_label("I'm okay now");
                active = false;
            } else {
                ci.set_status(IndicatorStatus.ACTIVE);
                att.set_label("Get Attention");
                active = false;
            }
        });
        menu.append(att);
        att.show();

        var show = new Gtk.MenuItem.with_label("Show Label");
        label_toggle(show);
        show.activate.connect(() => { label_toggle(show); });
        menu.append(show);
        show.show();

        var icon = new Gtk.CheckMenuItem.with_label("Set Local Icon");
        icon.activate.connect(() => {
            if (icon.get_active()) {
                ci.set_icon_full("simple-client-test-icon.png", null);
            } else {
                ci.set_icon_full("indicator-messages", null);
            }
        });
        menu.append(icon);
        icon.show();

        ci.set_menu(menu);

        Gtk.main();
    }
}
