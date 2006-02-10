
CFLAGS = -c
LDFLAGS =
ODIR = baddir

EXEBASE = qdata
EXE = $(ODIR)/qdata
all: $(EXE)

_next:
	make "CFLAGS = -c -g -I../common" "ODIR = next"
	
_irix:
	make "CFLAGS = -c -Ofast=ip32_10k -I../common -Xcpluscomm" "LDFLAGS = -Ofast=ip32_10k" "ODIR = irix"
	
_irixdebug:
	make "CFLAGS = -c -O2 -g -I../common -Xcpluscomm" "LDFLAGS = -g" "ODIR = irix"
	
_irixinst:
	make "CFLAGS = -c -Ofast=ip32_10k -I../common -Xcpluscomm" "LDFLAGS = -Ofast=ip32_10k" "ODIR = irix"
	cp irix/$(EXEBASE) /limbo/quake/bin_irix

_irixclean:
	rm -f irix/*.o irix/$(EXEBASE)

_osf:
	make "CFLAGS = -c -O4 -I../common -threads" "LDFLAGS = -threads" "ODIR = osf"
	
clean:
	rm -f next/*.o next/$(EXEBASE)
	rm -f osf/*.o osf/$(EXEBASE)
	rm -f irix/*.o irix/$(EXEBASE)

install:
	cp next/$(EXEBASE) /limbo/quake2/bin_next
	cp osf/$(EXEBASE) /limbo/quake2/bin_osf
	cp irix/$(EXEBASE) /limbo/quake2/bin_irix


FILES = $(ODIR)/qdata.o $(ODIR)/models.o $(ODIR)/sprites.o $(ODIR)/images.o  $(ODIR)/cmdlib.o $(ODIR)/scriplib.o $(ODIR)/lbmlib.o $(ODIR)/mathlib.o $(ODIR)/l3dslib.o $(ODIR)/trilib.o $(ODIR)/threads.o $(ODIR)/tables.o

$(EXE) : $(FILES)
	cc -o $(EXE) $(LDFLAGS) $(FILES) -lm
	
$(ODIR)/qdata.o : qdata.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/models.o : models.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/sprites.o : sprites.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/images.o : images.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/tables.o : tables.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i

$(ODIR)/cmdlib.o : ../common/cmdlib.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/scriplib.o : ../common/scriplib.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/lbmlib.o : ../common/lbmlib.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/mathlib.o : ../common/mathlib.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/trilib.o : ../common/trilib.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/l3dslib.o : ../common/l3dslib.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/threads.o : ../common/threads.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
