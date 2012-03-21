/*
 * Copyright 2011 Canonical Ltd.
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
		if (!(indicator is Indicator)) return -1;

		indicator.set_status(IndicatorStatus.ACTIVE);
		indicator.set_attention_icon("indicator-messages-new");

		var menu = new Gtk.Menu();

		var item = new Gtk.MenuItem.with_label("Foo");
		item.activate.connect(() => {
			indicator.set_status(IndicatorStatus.ATTENTION);
		});
		item.show();
		menu.append(item);

		var bar = item = new Gtk.MenuItem.with_label("Bar");
		item.show();
		item.activate.connect(() => {
			indicator.set_status(IndicatorStatus.ACTIVE);
		});
		menu.append(item);

		indicator.set_menu(menu);
		indicator.set_secondary_activate_target(bar);

		win.show_all();

		Gtk.main();
		return 0;
	}
}
