
/* Generated data (by glib-mkenums) */

/*
An object to represent the application as an application indicator
in the system panel.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

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

#include "app-indicator-enum-types.h"

#include "app-indicator.h"
/**
	app_indicator_category_get_type:

	Builds a GLib type for the #AppIndicatorCategory enumeration.

	Return value: A unique #GType for the #AppIndicatorCategory enum.
*/
GType
app_indicator_category_get_type (void)
{
	static GType etype = 0;
	if (G_UNLIKELY(etype == 0)) {
		static const GEnumValue values[] = {
			{ APP_INDICATOR_CATEGORY_APPLICATION_STATUS,  "APP_INDICATOR_CATEGORY_APPLICATION_STATUS", "ApplicationStatus" },
			{ APP_INDICATOR_CATEGORY_COMMUNICATIONS,  "APP_INDICATOR_CATEGORY_COMMUNICATIONS", "Communications" },
			{ APP_INDICATOR_CATEGORY_SYSTEM_SERVICES,  "APP_INDICATOR_CATEGORY_SYSTEM_SERVICES", "SystemServices" },
			{ APP_INDICATOR_CATEGORY_HARDWARE,  "APP_INDICATOR_CATEGORY_HARDWARE", "Hardware" },
			{ APP_INDICATOR_CATEGORY_OTHER,  "APP_INDICATOR_CATEGORY_OTHER", "Other" },
			{ 0, NULL, NULL}
		};
		
		etype = g_enum_register_static (g_intern_static_string("AppIndicatorCategory"), values);
	}

	return etype;
}

/**
	app_indicator_status_get_type:

	Builds a GLib type for the #AppIndicatorStatus enumeration.

	Return value: A unique #GType for the #AppIndicatorStatus enum.
*/
GType
app_indicator_status_get_type (void)
{
	static GType etype = 0;
	if (G_UNLIKELY(etype == 0)) {
		static const GEnumValue values[] = {
			{ APP_INDICATOR_STATUS_PASSIVE,  "APP_INDICATOR_STATUS_PASSIVE", "Passive" },
			{ APP_INDICATOR_STATUS_ACTIVE,  "APP_INDICATOR_STATUS_ACTIVE", "Active" },
			{ APP_INDICATOR_STATUS_ATTENTION,  "APP_INDICATOR_STATUS_ATTENTION", "NeedsAttention" },
			{ 0, NULL, NULL}
		};
		
		etype = g_enum_register_static (g_intern_static_string("AppIndicatorStatus"), values);
	}

	return etype;
}


/* Generated data ends here */

