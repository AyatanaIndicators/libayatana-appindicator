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
 *   Cody Russell <cody.russell@canonical.com>
 */

using Gtk;
using AyatanaAppIndicator;

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

                ApplicationIndicator indicator = new ApplicationIndicator ("Example",
                                                                           "applications-microblogging-panel",
                                                                           AppIndicatorCategory.ApplicationStatus);

                indicator.AppIndicatorStatus = AppIndicatorStatus.Attention;

                Menu menu = new Menu ();
                var foo = new MenuItem ("Foo");
                menu.Append (foo);
                foo.Activated += delegate {
                        System.Console.WriteLine ("Foo item has been activated");
                };

                menu.Append (new MenuItem ("Bar"));

                indicator.Menu = menu;
                indicator.Menu.ShowAll ();

                indicator.SecondaryActivateTarget = foo;

                win.ShowAll ();

                Application.Run ();
        }
}
