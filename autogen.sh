#!/bin/sh

PKG_NAME="libappindicator"

which gnome-autogen.sh || {
	echo "You need gnome-common from GNOME SVN"
	exit 1
}

gtkdocize || exit 1
USE_GNOME2_MACROS=1 \
USE_COMMON_DOC_BUILD=yes \
gnome-autogen.sh --enable-gtk-doc $@
