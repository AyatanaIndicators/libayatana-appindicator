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
