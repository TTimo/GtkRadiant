# Makefile for GtkRadiant, requires http://macdylibbundler.sourceforge.net/

INSTALL = ../install
TARGET = target
GTKRADIANT = $(TARGET)/GtkRadiant.app
BINARIES = $(GTKRADIANT)/Contents/MacOS
LIBRARIES = $(GTKRADIANT)/Contents/MacOS/lib
RESOURCES = $(GTKRADIANT)/Contents/Resources
VERSION = 1.6.4
DMG = $(TARGET)/GtkRadiant-$(VERSION).dmg
VOLUME_NAME = "GtkRadiant $(VERSION)"

GTK_RUNTIME = $(RESOURCES)/gtk-runtime
GTK_PREFIX_EXPR = 's:/opt/local:@executable_path/../Resources/gtk-runtime:g'

all: install

pre-install:
	install -d $(TARGET)
	cp -r GtkRadiant.app $(TARGET)
	find $(TARGET) -name .turd -delete

install: pre-install
	install $(INSTALL)/radiant.bin $(BINARIES)/radiant.bin
	install $(INSTALL)/q3map2 $(BINARIES)/q3map2
	install $(INSTALL)/q3map2_urt $(BINARIES)/q3map2_urt
	install $(INSTALL)/q3data $(BINARIES)/q3data
		
	install $(INSTALL)/bitmaps/*.* $(RESOURCES)/bitmaps
	
	install $(INSTALL)/modules/*.so $(RESOURCES)/modules
	install $(INSTALL)/modules/bitmaps/*.* $(RESOURCES)/modules/bitmaps
	
	@for i in $(INSTALL)/installs/*; do \
		if [ -d $$i/.svn ]; then \
			svn export --force $$i $(RESOURCES)/installs/`basename $$i` ; \
		else \
			cp -r $$i $(RESOURCES)/installs ; \
		fi \
	done

bundle:
	dylibbundler -b \
		-x $(BINARIES)/radiant.bin \
		-x $(BINARIES)/q3map2 \
		-x $(BINARIES)/q3map2_urt \
		-x $(BINARIES)/q3data \
	-d $(LIBRARIES) -of -p @executable_path/lib

	# The Radiant plugins (modules) are a little funky
	# Some of them are actually linked against the build directory
	ln -s ../build ./build
	
	dylibbundler -b \
		`find $(RESOURCES)/modules -name "*.so" | xargs -I {} echo -x {}` \
	-d $(LIBRARIES) -of -p @executable_path/lib
	
	rm -f build

gtk-runtime:
	gdk-pixbuf-query-loaders | sed $(GTK_PREFIX_EXPR) > \
		$(GTK_RUNTIME)/etc/gtk-2.0/gdk-pixbuf.loaders
		
	pango-querymodules | sed $(GTK_PREFIX_EXPR) > \
		$(GTK_RUNTIME)/etc/pango/pango.modules
	
	cp -r /opt/local/lib/gdk-pixbuf-2.0 $(GTK_RUNTIME)/lib
	cp -r /opt/local/lib/pango $(GTK_RUNTIME)/lib
	
	find $(GTK_RUNTIME)/lib -type f ! -name "*.so" -delete
	
	dylibbundler -b \
		`find $(GTK_RUNTIME)/lib -name "*.so" | xargs -I {} echo -x {}` \
	-d $(LIBRARIES) -of -p @executable_path/lib
	
	cp -r /opt/local/share/themes/Default $(GTK_RUNTIME)/share

image:
	ln -f -s /Applications $(TARGET)/Applications
	hdiutil create $(DMG) -srcfolder $(TARGET) -volname $(VOLUME_NAME)
	rm $(TARGET)/Applications

clean:
	rm -rf $(TARGET)/*
