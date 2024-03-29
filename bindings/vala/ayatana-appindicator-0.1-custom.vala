/*
 Copyright 2011 Canonical, Ltd.
 Copyright 2022 Robert Tari

  This program is free software: you can redistribute it and/or modify it
  under the terms of either or both of the following licenses:

  1) the GNU Lesser General Public License version 3, as published by the
     Free Software Foundation; and/or
  2) the GNU Lesser General Public License version 2.1, as published by
     the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranties of
  MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the applicable version of the GNU Lesser General Public
  License for more details.

  You should have received a copy of both the GNU Lesser General Public
  License version 3 and version 2.1 along with this program.  If not, see
  <http://www.gnu.org/licenses/>

  Authors:
      Michal Hruby <michal.hruby@canonical.com>
      Robert Tari <robert@tari.in>
*/

namespace AppIndicator {
  [CCode (type_check_function = "APP_IS_INDICATOR", type_id = "app_indicator_get_type ()")]
  public class Indicator : GLib.Object {
  }
}

// vim:et:ai:cindent:ts=2 sts=2 sw=2:
