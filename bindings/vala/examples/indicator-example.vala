/*
 * Copyright 2009 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *   Marco Trevisan (Treviño) <mail@3v1n0.net>
 */

using Gtk;
using AppIndicator;

public class IndicatorExample {
    public static int main(string[] args) {
		Gtk.init(ref args);
	
        var win = new Window();
        win.title = "Indicator Test";
        win.resize(200, 200);
        win.destroy.connect(Gtk.main_quit);

        var label = new Label("Hello, world!");
        win.add(label);

        var indicator = new Indicator(win.title, "indicator-messages",
                                      IndicatorCategory.APPLICATION_STATUS);

		indicator.set_status(IndicatorStatus.ACTIVE);
        indicator.set_attention_icon("indicator-messages-new");

        var menu = new Menu();
        var item = new MenuItem.with_label("Foo");
        item.activate.connect(() => {
        	indicator.set_status(IndicatorStatus.ATTENTION);
        });
        item.show();
        menu.append(item);


        item = new MenuItem.with_label("Bar");
        item.show();
        item.activate.connect(() => {
        	indicator.set_status(IndicatorStatus.ATTENTION);
        });
        menu.append(item);

        indicator.set_menu(menu);

        win.show_all();

		Gtk.main();
		return 0;
    }
}
