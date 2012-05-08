
CFLAGS = -c
LDFLAGS =
ODIR = baddir

EXEBASE = qbsp3
EXE = $(ODIR)/qbsp3
all: $(EXE)

_next:
	make "CFLAGS = -c -g -I../../common -DDOUBLEVEC_T" "ODIR = next"
	
_irix:
	make "CFLAGS = -c -Ofast=ip27 -OPT:IEEE_arithmetic=3 -I../../common -Xcpluscomm -DDOUBLEVEC_T" "LDFLAGS = -Ofast=ip27 -OPT:IEEE_arithmetic=3" "ODIR = irix"
	
_irixdebug:
	make "CFLAGS = -c -O2 -g -I../../common -Xcpluscomm -DDOUBLEVEC_T" "LDFLAGS = -g" "ODIR = irix"
	
_irixinst:
	make "CFLAGS = -c -Ofast=ip27 -OPT:IEEE_arithmetic=3 -I../../common -Xcpluscomm -DDOUBLEVEC_T" "LDFLAGS = -Ofast=ip27 -OPT:IEEE_arithmetic=3" "ODIR = irix"
	cp irix/$(EXEBASE) /limbo/quake2/bin_irix

_irixclean:
	rm -f irix/*.o irix/$(EXEBASE)

_osf:
	make "CFLAGS = -c -O4 -I../../common -threads -DDOUBLEVEC_T" "LDFLAGS = -threads" "ODIR = osf"
	
clean:
	rm -f irix/*.o irix/$(EXEBASE)

install:
	cp irix/$(EXEBASE) /limbo/quake2/bin_irix


FILES = $(ODIR)/brushbsp.o $(ODIR)/bspfile.o $(ODIR)/cmdlib.o $(ODIR)/faces.o $(ODIR)/nodraw.o $(ODIR)/glfile.o $(ODIR)/leakfile.o $(ODIR)/map.o $(ODIR)/mathlib.o $(ODIR)/polylib.o $(ODIR)/portals.o $(ODIR)/prtfile.o $(ODIR)/qbsp3.o $(ODIR)/scriplib.o $(ODIR)/textures.o $(ODIR)/threads.o $(ODIR)/tree.o $(ODIR)/writebsp.o $(ODIR)/csg.o

$(EXE) : $(FILES)
	cc -o $(EXE) $(LDFLAGS) $(FILES) -lm
	
$(ODIR)/brushbsp.o : brushbsp.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/faces.o : faces.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/nodraw.o : nodraw.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/glfile.o : glfile.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/leakfile.o : leakfile.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/map.o : map.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/portals.o : portals.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/prtfile.o : prtfile.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/qbsp3.o : qbsp3.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/tree.o : tree.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/textures.o : textures.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/writebsp.o : writebsp.c
	cc $(CFLAGS) -E $? | tr -d '\015' > /tmp/temp.i
	cc $(CFLAGS) -o $@ /tmp/temp.i
$(ODIR)/csg.o : csg.c
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
