/*
Python bindings for libappindicator.

Copyright 2009 Canonical Ltd.

Authors:
    Eitan Isaacson <eitan@ascender.com>
    Neil Jagdish Patel <neil.patel@canonical.com>

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
*/
#include <pygobject.h>
 
void pyappindicator_register_classes (PyObject *d); 
extern PyMethodDef pyappindicator_functions[];

DL_EXPORT(void)
init_appindicator(void)
{
		PyObject *m, *d;
		
		init_pygobject ();
		
		m = Py_InitModule ("_appindicator", pyappindicator_functions);
		d = PyModule_GetDict (m);
		
		pyappindicator_register_classes (d);

		_appindicator_add_constants (m, "APP_INDICATOR_");
		if (PyErr_Occurred ()) {
				Py_FatalError ("can't initialise module appindicator");
		}
}
