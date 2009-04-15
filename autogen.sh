#!/bin/sh
aclocal -I m4
autoconf
automake --foreign -a
./configure
