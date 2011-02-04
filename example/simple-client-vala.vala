/*
A small piece of sample code demonstrating a very simple application
with an indicator.

Copyright 2011 Canonical Ltd.

Authors:
    Marco Trevisan <mail@3v1n0.net>

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
	Menu menu;
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
		ci.set_attention_icon("indicator-messages-new");
		ci.set_label("1%", "100%");

		active = true;
		can_haz_label = true;
	}

	private void toggle_sensitivity(Widget widget) {
		widget.set_sensitive(!widget.is_sensitive());
	}

	private void append_submenu(MenuItem item) {
		var menu = new Menu();
		MenuItem mi;

		mi = new MenuItem.with_label("Sub 1");
		menu.append(mi);
		mi.activate.connect(() => { print("Sub1\n"); });

		MenuItem prev_mi = mi;
		mi = new MenuItem.with_label("Sub 2");
		menu.append(mi);
		mi.activate.connect(() => { toggle_sensitivity(prev_mi); });

		mi = new MenuItem.with_label("Sub 3");
		menu.append(mi);
		mi.activate.connect(() => { print("Sub3\n"); });

		menu.show_all();
		item.set_submenu(menu);
	}

	private void label_toggle(MenuItem item) {
		can_haz_label = !can_haz_label;

		if (can_haz_label) {
			item.set_label("Hide label");
		} else {
			item.set_label("Show label");
		}
	}

	public void run() {
		Timeout.add_seconds(1, () => {
			percentage = (percentage + 1) % 100;
			if (can_haz_label) {
				ci.set_label(@"$(percentage+1)%", "");
			} else {
				ci.set_label("", "");
			}
			return true;
		});

		menu = new Menu();
		var chk = new CheckMenuItem.with_label("1");
		chk.activate.connect(() => { print("1\n"); });
		menu.append(chk);
		chk.show();

		var radio = new RadioMenuItem.with_label(new SList<RadioMenuItem>(), "2");
		radio.activate.connect(() => { print("2\n");	});
		menu.append(radio);
		radio.show();

		var submenu = new MenuItem.with_label("3");
		menu.append(submenu);
		append_submenu(submenu);
		submenu.show();

		var toggle_item = new MenuItem.with_label("Toggle 3");
		toggle_item.activate.connect(() => { toggle_sensitivity(submenu); });
		menu.append(toggle_item);
		toggle_item.show();

		var imgitem = new ImageMenuItem.from_stock(Stock.NEW, null);
		imgitem.activate.connect(() => {
			Image img = (Image) imgitem.get_image();
			img.set_from_stock(Stock.OPEN, IconSize.MENU);
		});
		menu.append(imgitem);
		imgitem.show();

		var att = new MenuItem.with_label("Get Attention");
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

		var show = new MenuItem.with_label("Show Label");
		label_toggle(show);
		show.activate.connect(() => { label_toggle(show); });
		menu.append(show);
		show.show();

		var icon = new CheckMenuItem.with_label("Set Local Icon");
		icon.activate.connect(() => {
			if (icon.get_active()) {
				ci.set_icon("simple-client-test-icon.png");
			} else {
				ci.set_icon("indicator-messages");
			}
		});
		menu.append(icon);
		icon.show();

		ci.set_menu(menu);

		Gtk.main();
	}
}
