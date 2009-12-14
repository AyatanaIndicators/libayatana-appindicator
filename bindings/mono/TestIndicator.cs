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

using System;
using GLib;
using Gtk;
using AppIndicator;

using NUnit.Framework;

namespace Ayatana.AppIndicator.Test
{
        [TestFixture]
        public class IndicatorTest
        {
                ApplicationIndicator indicator;

                private void Update ()
                {
                        while (MainContext.Pending ())
                                MainContext.Iteration(true);
                }

                [SetUp]
                public void Init ()
                {
                        Application.Init ();

                        Console.WriteLine ("Init()");

                        indicator = new ApplicationIndicator ("my-id", "my-name", Category.ApplicationStatus);

                        Console.WriteLine ("Created indicator");

                        Update();
                }

                [Test]
                public void TestProperties ()
                {
                        Console.WriteLine ("TestProperties()");

                        Assert.AreNotSame (indicator, null);

                        Assert.AreEqual (indicator.IconName, "my-name");
                        Assert.AreEqual (indicator.ID, "my-id");
                        Assert.AreEqual (indicator.Status, Category.ApplicationStatus);

                        Console.WriteLine ("End..");
                }

                [Test]
                public void TestSetProperties ()
                {
                        Console.WriteLine ("TestSetProperties");

                        indicator.Status = Status.Attention;
                        indicator.AttentionIconName = "my-attention-name";

                        Assert.AreEqual (indicator.Status, Status.Attention);
                        Assert.AreEqual (indicator.AttentionIconName, "my-attention-name");

                        Console.WriteLine ("End..");
                }
        }
}