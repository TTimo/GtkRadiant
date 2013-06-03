# Makefile for GtkRadiant, requires http://macdylibbundler.sourceforge.net/

INSTALL = ../install
TARGET = target
GTKRADIANT = $(TARGET)/GtkRadiant.app
LIBRARIES = $(GTKRADIANT)/Contents/lib
BINARIES = $(GTKRADIANT)/Contents/MacOS
RESOURCES = $(GTKRADIANT)/Contents/Resources
VERSION = 1.6.4
DMG = $(TARGET)/GtkRadiant-$(VERSION).dmg
VOLUME_NAME = "GtkRadiant $(VERSION)"

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
	install $(INSTALL)/modules/*.so $(BINARIES)/modules
	
	install $(INSTALL)/bitmaps/*.* $(RESOURCES)/bitmaps
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
		`find $(BINARIES) -type f | xargs -I {} echo -x {}` \
	-d $(LIBRARIES) -of -p @executable_path/../lib

image:
	ln -f -s /Applications $(TARGET)/Applications
	hdiutil create $(DMG) -srcfolder $(TARGET) -volname $(VOLUME_NAME)
	rm $(TARGET)/Applications

clean:
	rm -rf $(TARGET)/*
