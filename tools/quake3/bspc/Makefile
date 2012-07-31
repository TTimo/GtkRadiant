CC=gcc
CFLAGS=\
	-Dstricmp=strcasecmp -DCom_Memcpy=memcpy -DCom_Memset=memset \
	-DMAC_STATIC= -DQDECL= -DLINUX -DBSPC -D_FORTIFY_SOURCE=2 \
	-I. -Ideps -Wall

RELEASE_CFLAGS=-O3 -ffast-math
DEBUG_CFLAGS=-g -O0 -ffast-math
LDFLAGS=-lm -lpthread

DO_CC=$(CC) $(CFLAGS) -o $@ -c $<

#############################################################################
# SETUP AND BUILD BSPC
#############################################################################

.c.o:
	$(DO_CC)

GAME_OBJS = \
	_files.o\
	aas_areamerging.o\
	aas_cfg.o\
	aas_create.o\
	aas_edgemelting.o\
	aas_facemerging.o\
	aas_file.o\
	aas_gsubdiv.o\
	aas_map.o\
	aas_prunenodes.o\
	aas_store.o\
	be_aas_bspc.o\
	deps/botlib/be_aas_bspq3.o\
	deps/botlib/be_aas_cluster.o\
	deps/botlib/be_aas_move.o\
	deps/botlib/be_aas_optimize.o\
	deps/botlib/be_aas_reach.o\
	deps/botlib/be_aas_sample.o\
	brushbsp.o\
	bspc.o\
	deps/qcommon/cm_load.o\
	deps/qcommon/cm_patch.o\
	deps/qcommon/cm_test.o\
	deps/qcommon/cm_trace.o\
	csg.o\
	glfile.o\
	l_bsp_ent.o\
	l_bsp_hl.o\
	l_bsp_q1.o\
	l_bsp_q2.o\
	l_bsp_q3.o\
	l_bsp_sin.o\
	l_cmd.o\
	deps/botlib/l_libvar.o\
	l_log.o\
	l_math.o\
	l_mem.o\
	l_poly.o\
	deps/botlib/l_precomp.o\
	l_qfiles.o\
	deps/botlib/l_script.o\
	deps/botlib/l_struct.o\
	l_threads.o\
	l_utils.o\
	leakfile.o\
	map.o\
	map_hl.o\
	map_q1.o\
	map_q2.o\
	map_q3.o\
	map_sin.o\
	deps/qcommon/md4.o\
	nodraw.o\
	portals.o\
	textures.o\
	tree.o\
	deps/qcommon/unzip.o

        #tetrahedron.o

EXEC = bspc

all: release

debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(EXEC)_g

release: CFLAGS += $(RELEASE_CFLAGS)
release: $(EXEC)

$(EXEC): $(GAME_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(GAME_OBJS)
	strip $@

$(EXEC)_g: $(GAME_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(GAME_OBJS)

#############################################################################
# MISC
#############################################################################
.PHONY: clean depend

clean:
	-rm -f $(GAME_OBJS) $(EXEC) $(EXEC)_g

depend:
	$(CC) $(CFLAGS) -MM $(GAME_OBJS:.o=.c) > .deps

include .deps
