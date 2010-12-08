/*
Quick litte lack to generate the ordering ID.

Copyright 2010 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

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

#ifndef __GENERATE_ID_H__
#define __GENERATE_ID_H__

#include <glib.h>
#include "app-indicator.h"

guint32 _generate_id (const AppIndicatorCategory category, const gchar * id);

#endif /* __GENERATE_ID_H__ */
