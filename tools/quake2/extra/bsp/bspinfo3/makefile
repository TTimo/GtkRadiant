
CFLAGS = -c
LDFLAGS =
ODIR = baddir

EXEBASE = bspinfo3
EXE = $(ODIR)/bspinfo3
all: $(EXE)

_next:
	make "CFLAGS = -c -g -I../../common -DDOUBLEVEC_T" "ODIR = next"
	
_irix:
	make "CFLAGS = -c -Ofast=ip32_10k -I../../common -Xcpluscomm -DDOUBLEVEC_T" "LDFLAGS = -Ofast=ip32_10k" "ODIR = irix"
	
_irixdebug:
	make "CFLAGS = -c -O2 -g -I../../common -Xcpluscomm -DDOUBLEVEC_T" "LDFLAGS = -g" "ODIR = irix"
	
_irixinst:
	make "CFLAGS = -c -Ofast=ip32_10k -I../../common -Xcpluscomm -DDOUBLEVEC_T" "LDFLAGS = -Ofast=ip32_10k" "ODIR = irix"
	cp irix/$(EXEBASE) /limbo/quake2/bin_irix

_irixclean:
	rm -f irix/*.o irix/$(EXEBASE)

_osf:
	make "CFLAGS = -c -O4 -I../../common -threads -DDOUBLEVEC_T" "LDFLAGS = -threads" "ODIR = osf"
	
clean:
	rm -f irix/*.o irix/$(EXEBASE)

install:
	cp irix/$(EXEBASE) /limbo/quake2/bin_irix


FILES = $(ODIR)/bspinfo3.o $(ODIR)/bspfile.o $(ODIR)/cmdlib.o $(ODIR)/scriplib.o

$(EXE) : $(FILES)
	cc -o $(EXE) $(LDFLAGS) $(FILES)
	
$(ODIR)/bspinfo3.o : bspinfo3.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i

$(ODIR)/cmdlib.o : ../../common/cmdlib.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/scriplib.o : ../../common/scriplib.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/bspfile.o : ../../common/bspfile.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
