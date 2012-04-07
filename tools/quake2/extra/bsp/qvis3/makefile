
CFLAGS = -c
LDFLAGS =
ODIR = baddir

EXEBASE = qvis3
EXE = $(ODIR)/qvis3
all: $(EXE)

_next:
	make "CFLAGS = -c -g -I../../common" "ODIR = next"
	
_irix:
	make "CFLAGS = -c -Ofast=ip32_10k -I../../common -Xcpluscomm" "LDFLAGS = -Ofast=ip32_10k" "ODIR = irix"
	
_irixinst:
	make "CFLAGS = -c -Ofast=ip32_10k -I../../common -Xcpluscomm" "LDFLAGS = -Ofast=ip32_10k" "ODIR = irix"
	cp irix/$(EXEBASE) /limbo/quake2/bin_irix

_irixclean:
	rm -f irix/*.o irix/$(EXEBASE)

_osf:
	make "CFLAGS = -c -O4 -I../../common -threads" "LDFLAGS = -threads -lm" "ODIR = osf"
	
clean:
	rm -f irix/*.o irix/$(EXEBASE)

install:
	cp irix/$(EXEBASE) /limbo/quake2/bin_irix


FILES = $(ODIR)/bspfile.o $(ODIR)/cmdlib.o $(ODIR)/mathlib.o $(ODIR)/scriplib.o  $(ODIR)/threads.o $(ODIR)/qvis3.o $(ODIR)/flow.o

$(EXE) : $(FILES)
	cc -o $(EXE) $(LDFLAGS) $(FILES)
	
$(ODIR)/qvis3.o : qvis3.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/flow.o : flow.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
	
$(ODIR)/cmdlib.o : ../../common/cmdlib.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/mathlib.o : ../../common/mathlib.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/polylib.o : ../../common/polylib.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/scriplib.o : ../../common/scriplib.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/threads.o : ../../common/threads.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/bspfile.o : ../../common/bspfile.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
