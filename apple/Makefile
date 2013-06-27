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

all: install gtk-runtime bundle

pre-install:
	install -d $(TARGET)
	cp -r GtkRadiant.app $(TARGET)
	find $(TARGET) -name .turd -delete

install: pre-install
	cp -r $(INSTALL) $(RESOURCES)
	rm -rf `find $(INSTDIR)/installs -type d -name .svn`

gtk-runtime:
	gdk-pixbuf-query-loaders | sed $(PREFIX_SED_EXPR) > \
		$(CONFDIR)/gtk-2.0/gdk-pixbuf.loaders
		
	gtk-query-immodules-2.0 | sed $(PREFIX_SED_EXPR) > \
		$(CONFDIR)/gtk-2.0/gtk.immodules
		
	pango-querymodules | sed $(PREFIX_SED_EXPR) > \
		$(CONFDIR)/pango/pango.modules
	
	cp -r /opt/local/lib/gdk-pixbuf-2.0 $(LIBDIR)
	cp -r /opt/local/lib/gtk-2.0 $(LIBDIR)
	cp -r /opt/local/lib/pango $(LIBDIR)
	
	find $(LIBDIR)/gdk-pixbuf-2.0 -type f ! -name "*.so" -delete
	find $(LIBDIR)/gtk-2.0 -type f ! -name "*.so" -delete	
	find $(LIBDIR)/pango -type f ! -name "*.so" -delete
	
	rm -rf $(LIBDIR)/gtk-2.0/{includes,modules}
	rm -rf $(LIBDIR)/gtk-2.0/*/printbackends
	
	dylibbundler -b \
		`find $(LIBDIR)/gdk-pixbuf-2.0 -type f | xargs -I {} echo -x {}` \
		`find $(LIBDIR)/gtk-2.0 -type f | xargs -I {} echo -x {}` \
		`find $(LIBDIR)/pango -type f | xargs -I {} echo -x {}` \
	-d $(LIBDIR) -of -p @executable_path/../lib
	
	cp -r /opt/local/share/themes/Default $(RESOURCES)/share

bundle:
	dylibbundler -b \
		-x $(INSTDIR)/radiant.bin \
		-x $(INSTDIR)/q3map2 \
		-x $(INSTDIR)/q3map2_urt \
		-x $(INSTDIR)/q3data \
	-d $(LIBDIR) -of -p @executable_path/../lib

	# The Radiant plugins (modules) are a little funky
	# Some of them are actually linked against the build directory
	ln -s ../build ./build
	
	dylibbundler -b \
		`find $(INSTDIR)/modules -name "*.so" | xargs -I {} echo -x {}` \
	-d $(LIBDIR) -of -p @executable_path/../lib
	
	rm -f build

image:
	ln -f -s /Applications $(TARGET)/Applications
	hdiutil create $(DMG) -srcfolder $(TARGET) -volname $(VOLUME_NAME)
	rm $(TARGET)/Applications

clean:
	rm -rf $(TARGET)/*
