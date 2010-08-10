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

struct ordering_id_struct {
	gchar category;
	gchar first;
	gchar second;
	gchar third;
};

union ordering_id_union_t {
	guint32 id;
	struct ordering_id_struct str;
};

guint32
generate_id (const gchar category, const gchar * id)
{
	union ordering_id_union_t u;

	u.str.category = category;
	
	u.str.first = 0;
	u.str.second = 0;
	u.str.third = 0;

	if (id != NULL) {
		if (id[0] != '\0') {
			u.str.first = id[0];
			if (id[1] != '\0') {
				u.str.second = id[1];
				if (id[2] != '\0') {
					u.str.third = id[2];
				}
			}
		}
	}

	return u.id;
}
