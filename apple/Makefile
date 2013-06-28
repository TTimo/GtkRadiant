# Makefile for GtkRadiant.app, requires http://macdylibbundler.sourceforge.net/

INSTALL = ../install
TARGET = target
RESOURCES = $(TARGET)/GtkRadiant.app/Contents/Resources
CONFDIR = $(RESOURCES)/etc
DATADIR = $(RESOURCES)/share
LIBDIR = $(RESOURCES)/lib
INSTDIR = $(RESOURCES)/install
PREFIX_SED_EXPR = 's:/opt/local:@executable_path/..:g'
VERSION = 1.6.4
DMG = $(TARGET)/GtkRadiant-$(VERSION).dmg
VOLUME_NAME = "GtkRadiant $(VERSION)"

all: install bundle

-pre-install:
	install -d $(TARGET)
	cp -r GtkRadiant.app $(TARGET)
	find $(TARGET) -name .turd -delete
	
-gtk-runtime-gdk-pixbuf-2.0:
	cp -r /opt/local/lib/gdk-pixbuf-2.0 $(LIBDIR)
	find $(LIBDIR)/gdk-pixbuf-2.0 -type f ! -name "*.so" -delete
	
	gdk-pixbuf-query-loaders | sed $(PREFIX_SED_EXPR) > \
		$(CONFDIR)/gtk-2.0/gdk-pixbuf.loaders

-gtk-runtime-pango:
	cp -r /opt/local/lib/pango $(LIBDIR)
	find $(LIBDIR)/pango -type f ! -name "*.so" -delete
	
	pango-querymodules | sed $(PREFIX_SED_EXPR) > \
		$(CONFDIR)/pango/pango.modules
	
-gtk-runtime: -gtk-runtime-gdk-pixbuf-2.0 -gtk-runtime-pango
	cp -r /opt/local/lib/gtk-2.0 $(LIBDIR)
	find $(LIBDIR)/gtk-2.0 -type f ! -name "*.so" -delete	
	
	rm -rf $(LIBDIR)/gtk-2.0/{includes,modules}
	rm -rf $(LIBDIR)/gtk-2.0/*/printbackends
	
	cp -r /opt/local/share/themes/Default $(RESOURCES)/share
	
	gtk-query-immodules-2.0 | sed $(PREFIX_SED_EXPR) > \
		$(CONFDIR)/gtk-2.0/gtk.immodules

install: -pre-install -gtk-runtime
	cp -r $(INSTALL) $(RESOURCES)
	rm -rf `find $(INSTDIR)/installs -type d -name .svn`

bundle:

	# The Radiant plugins (modules) are a little funky
	# Some of them are actually linked against the build directory
	
	ln -s ../build ./build
	
	dylibbundler -b \
		-x $(INSTDIR)/radiant.bin \
		-x $(INSTDIR)/q3map2 \
		-x $(INSTDIR)/q3map2_urt \
		-x $(INSTDIR)/q3data \
		`find $(INSTDIR)/modules -name "*.so" | xargs -I {} echo -x {}` \
		`find $(LIBDIR) -name "*.so" | xargs -I {} echo -x {}` \
	-d $(LIBDIR) -of -p @executable_path/../lib

	rm -f build

image:
	find $(TARGET) -name .DS_Store -delete
	ln -f -s /Applications $(TARGET)/Applications
	hdiutil create -ov $(DMG) -srcfolder $(TARGET) -volname $(VOLUME_NAME)
	rm $(TARGET)/Applications

clean:
	rm -rf $(TARGET)/*
