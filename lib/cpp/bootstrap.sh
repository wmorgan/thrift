#!/bin/sh

./cleanup.sh
autoscan
autoheader
aclocal -I ./aclocal
libtoolize --automake
touch NEWS README AUTHORS ChangeLog
autoconf
automake -ac
