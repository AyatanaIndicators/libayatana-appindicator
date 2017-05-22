#!/bin/sh

PKG_NAME="libayatana-appindicator"

which mate-autogen || {
	echo "You need mate-common from https://git.mate-desktop.org/mate-common"
	exit 1
}

gtkdocize || exit 1
USE_COMMON_DOC_BUILD=yes \
mate-autogen --enable-gtk-doc $@
