# Makefile for GtkRadiant.app, bundling the Homebrew GTK3 runtime.
# Requires dylibbundler (brew install dylibbundler). Run scons at the top level first.

INSTALL = ../install
TARGET = target
APP = $(TARGET)/GtkRadiant.app
RESOURCES = $(APP)/Contents/Resources
BINDIR = $(RESOURCES)/install
LIBDIR = $(RESOURCES)/lib
SHAREDIR = $(RESOURCES)/share
VERSION := $(shell sed -n 's/.*RADIANT_VERSION "\(.*\)".*/\1/p' ../include/version.h)
DMG = $(TARGET)/GtkRadiant-$(VERSION).dmg
VOLUME_NAME = "GtkRadiant $(VERSION)"

# Homebrew bottles are built for the running macOS release, so the bundle can't go lower
MIN_MACOS := $(shell sw_vers -productVersion | cut -d. -f1).0

BREW := $(shell brew --prefix)
PIXBUF_LOADERS = lib/gdk-pixbuf-2.0/2.10.0/loaders

all: install bundle

-pre-install:
	rm -rf $(APP)
	install -d $(APP)
	# rsync doesn't copy extended attributes, so a quarantine flag from downloading the
	# source doesn't make Gatekeeper treat the built app as downloaded (cp -X keeps it)
	rsync -a --exclude .turd GtkRadiant.app/ $(APP)/
	plutil -replace CFBundleShortVersionString -string $(VERSION) $(APP)/Contents/Info.plist
	plutil -replace CFBundleVersion -string $(VERSION) $(APP)/Contents/Info.plist
	plutil -replace LSMinimumSystemVersion -string $(MIN_MACOS) $(APP)/Contents/Info.plist

# PNG and JPEG are built into gdk-pixbuf; the SVG loader is needed for the symbolic icons
-gtk-runtime-gdk-pixbuf:
	install -d $(RESOURCES)/$(PIXBUF_LOADERS)
	cp -L $(BREW)/$(PIXBUF_LOADERS)/libpixbufloader_svg.so $(RESOURCES)/$(PIXBUF_LOADERS)
	# its install name is a Homebrew path, and dylibbundler would turn both of its rpaths
	# into the same one, which dyld rejects; dylibbundler re-signs it later
	install_name_tool -id @executable_path/../$(PIXBUF_LOADERS)/libpixbufloader_svg.so \
		-delete_rpath @loader_path/../../.. \
		$(RESOURCES)/$(PIXBUF_LOADERS)/libpixbufloader_svg.so 2> /dev/null
	# query the Homebrew copy: macOS won't load code from inside an unsigned bundle
	gdk-pixbuf-query-loaders $(BREW)/$(PIXBUF_LOADERS)/libpixbufloader_svg.so \
		| sed "s:$(BREW)/$(PIXBUF_LOADERS):@RESOURCES@/$(PIXBUF_LOADERS):g" > $(RESOURCES)/etc/gdk-pixbuf-loaders.cache.in

-gtk-runtime: -gtk-runtime-gdk-pixbuf
	install -d $(SHAREDIR)/icons/hicolor
	cp -RL $(BREW)/share/icons/Adwaita $(SHAREDIR)/icons
	cp $(BREW)/share/icons/hicolor/index.theme $(SHAREDIR)/icons/hicolor
	# the file chooser and color chooser read their settings from GSettings
	install -d $(SHAREDIR)/glib-2.0/schemas
	cp $(BREW)/share/glib-2.0/schemas/org.gtk.Settings.*.gschema.xml $(SHAREDIR)/glib-2.0/schemas
	glib-compile-schemas $(SHAREDIR)/glib-2.0/schemas

install: -pre-install -gtk-runtime
	# .game files hold absolute paths from this machine; the first run writes new ones
	install -d $(BINDIR)
	rsync -a --exclude .svn --exclude /games $(INSTALL)/ $(BINDIR)/

bundle:
	# The Radiant plugins (modules) and the gdk-pixbuf loaders are loaded into
	# radiant.bin, so @executable_path is Resources/install for all of them.
	# -s resolves @rpath references (librsvg) that only work from Homebrew's lib dir.
	dylibbundler -b -cd -of \
		-x $(BINDIR)/radiant.bin \
		-x $(BINDIR)/q3map2 \
		-x $(BINDIR)/q3map2_urt \
		-x $(BINDIR)/q3data \
		`find $(BINDIR)/modules -name "*.so" | xargs -I {} echo -x {}` \
		`find $(LIBDIR) -name "*.so" | xargs -I {} echo -x {}` \
		-s $(BREW)/lib -d $(LIBDIR) -p @executable_path/../lib/
	# nothing may still point at the build machine's Homebrew
	! find $(RESOURCES) -type f \( -name "*.dylib" -o -name "*.so" -o -perm +111 \) -exec otool -L {} \; 2>/dev/null \
		| grep "$(BREW)"
	# dyld refuses to load a binary with duplicate rpaths
	! find $(RESOURCES) -type f \( -name "*.dylib" -o -name "*.so" -o -perm +111 \) -exec sh -c \
		'otool -l "$$1" | grep -A2 LC_RPATH | grep " path " | sort | uniq -d | grep -q . && echo "$$1"' sh {} \; \
		| grep .
	# a quarantined file would make Gatekeeper ask before launching the app
	! find $(APP) -xattrname com.apple.quarantine | grep .
	codesign --force --sign - $(APP)

image:
	find $(TARGET) -name .DS_Store -delete
	ln -f -s /Applications $(TARGET)/Applications
	hdiutil create -ov $(DMG) -srcfolder $(TARGET) -volname $(VOLUME_NAME)
	rm $(TARGET)/Applications

clean:
	rm -rf $(TARGET)/*

.PHONY: all install bundle image clean -pre-install -gtk-runtime -gtk-runtime-gdk-pixbuf
