# Common configuration options for all plugins

CC=gcc
CXX=g++
CFLAGS+=`gtk-config --cflags` -Wall -g -I../../include
CPPFLAGS+=`gtk-config --cflags` -Wall -g -I../../include
LDFLAGS+=`gtk-config --libs` -shared
OUTDIR=$(RADIANT_DATA)plugins
OBJS := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC)))
OBJS += $(patsubst %.c,%.o,$(filter %.c,$(SRC)))

all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CXX) -o $(OUTPUT) $(OBJS) $(LDFLAGS)
	@if [ -d $(OUTDIR) ]; then cp $(OUTPUT) $(OUTDIR); fi

## Other targets
.PHONY: clean

clean:
	rm -f *.o *.d $(OUTPUT) core

## Dependencies
-include $(OBJS:.o=.d)

%.d: %.cpp
	@echo -n "$(@) " > $@
	@if { !(eval $(CXX) -MM $(CPPFLAGS) -w $<) >> $@; }; then \
	  rm -f $@; exit 1; \
	fi
	@[ -s $@ ] || rm -f $@
