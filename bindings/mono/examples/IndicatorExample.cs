using Gtk;
using AppIndicator;

public class IndicatorExample
{
        public static void Main ()
        {
                Application.Init ();

                Window win = new Window ("Test");
                win.Resize (200, 200);

                Label label = new Label ();
                label.Text = "Hello, world!";

                win.Add (label);

                ApplicationIndicator indicator = new ApplicationIndicator ("my-id",
                                                                           "my-name",
                                                                           Category.ApplicationStatus);

                indicator.Status = Status.Attention;

                /*
                Menu menu = new Menu ();
                menu.Append (new MenuItem ("Foo"));
                menu.Append (new MenuItem ("Bar"));

                indicator.Menu = menu;
                */

                win.ShowAll ();

                Application.Run ();
        }
}