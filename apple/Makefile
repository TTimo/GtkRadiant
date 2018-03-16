# Makefile for GtkRadiant.app, requires http://macdylibbundler.sourceforge.net/

BUILD = ../build
INSTALL = ../install
TARGET = target
RESOURCES = $(TARGET)/GtkRadiant.app/Contents/Resources
BINDIR = $(RESOURCES)/install
CONFDIR = $(RESOURCES)/etc
DATADIR = $(RESOURCES)/share
LIBDIR = $(RESOURCES)/lib
VERSION = 1.6.6
DMG = $(TARGET)/GtkRadiant-$(VERSION).dmg
VOLUME_NAME = "GtkRadiant $(VERSION)"

# We must rewrite some Gtk configuration files, which vary from environment to
# environment depending on where you've installed MacPorts. Fun!

MACPORTS_PREFIX := $(shell which port | sed 's:/bin/port::')
PREFIX_SED_EXPR = "s:$(MACPORTS_PREFIX):@executable_path/..:g"

all: install bundle

-pre-install:
	install -d $(TARGET)
	cp -r GtkRadiant.app $(TARGET)
	find $(TARGET) -name .turd -delete
	
-gtk-runtime-gdk-pixbuf-2.0:
	cp -r $(MACPORTS_PREFIX)/lib/gdk-pixbuf-2.0 $(LIBDIR)
	find $(LIBDIR)/gdk-pixbuf-2.0 -type f ! -name "*.so" -delete
	
	gdk-pixbuf-query-loaders | sed $(PREFIX_SED_EXPR) > \
		$(CONFDIR)/gtk-2.0/gdk-pixbuf.loaders
	
-gtk-runtime: -gtk-runtime-gdk-pixbuf-2.0
	cp -r $(MACPORTS_PREFIX)/lib/gtk-2.0 $(LIBDIR)
	find $(LIBDIR)/gtk-2.0 -type f ! -name "*.so" -delete	
	
	rm -rf $(LIBDIR)/gtk-2.0/{includes,modules}
	rm -rf $(LIBDIR)/gtk-2.0/*/printbackends
	
	cp -r $(MACPORTS_PREFIX)/share/themes/Default $(RESOURCES)/share
	
	gtk-query-immodules-2.0 | sed $(PREFIX_SED_EXPR) > \
		$(CONFDIR)/gtk-2.0/gtk.immodules

install: -pre-install -gtk-runtime
	cp -r $(INSTALL) $(RESOURCES)
	rm -rf `find $(RESOURCES)/install -type d -name .svn`

bundle:
	
	# The Radiant plugins (modules) are a little funky
	# Some of them are actually linked against the build directory
	
	test -L build || ln -s $(BUILD)
	
	dylibbundler -b \
		-x $(BINDIR)/radiant.bin \
		-x $(BINDIR)/q3map2 \
		-x $(BINDIR)/q3map2_urt \
		-x $(BINDIR)/q3data \
		`find $(BINDIR)/modules -name "*.so" | xargs -I {} echo -x {}` \
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
