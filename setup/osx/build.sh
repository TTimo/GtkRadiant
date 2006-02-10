#!/bin/sh
(cd loki_setup && patch -N -p0 < ../setup.patch)
(cd loki_setupdb ; make distclean ; ./autogen.sh && ./configure && make)
(
cd loki_setup
make distclean
./autogen.sh && ./configure && make
mkdir -p image/setup.data/bin/Darwin/ppc/glibc-2.1
make install
find image -name setup -exec rm {} \;
)

(cd GtkRadiant ; scons BUILD=release SETUP=1)
