# Star Wars Jedi Knight II: Jedi Outcast single player for AmigaOS 3.x
# (68k), rendering through minigl.library (PiStorm3D or any other
# implementation). Follows the Return to Castle Wolfenstein SP port.
#
# Needs bebbo's m68k-amigaos-gcc toolchain; build.sh runs this Makefile in
# the amigadev/m68k-amigaos-gcc Docker image. Output: objects/jk2sp.
#
# The game module (jk2gamex86.dll on Windows: game, cgame and ICARUS in one)
# cannot be a shared object on AmigaOS 3.x, so it is linked into the
# executable. Every global symbol it defines gets a "game_" prefix
# (objcopy --redefine-syms), so its private copies of q_shared.cpp and
# q_math.cpp and its vmMain/dllEntry do not clash with the engine's (the
# ui module is part of the engine here, as on Windows). See
# amiga_modules.cpp for how the module is reset between loads.

CC      = m68k-amigaos-gcc
CXX     = m68k-amigaos-g++
NM      = m68k-amigaos-nm
OBJCOPY = m68k-amigaos-objcopy
STRIP   = m68k-amigaos-strip

SRC     = ..
MINIGL ?= ../../PiStorm3D/SDK
SWAPSTACK ?= /opt/amiga/m68k-amigaos/libnix/lib/swapstack.o

CPU     = -m68040

# See the RTCW port's Makefile for the float math flags: libnix's float
# functions return in the wrong register (amiga_libfix.c) and gcc must not
# merge sin+cos into libnix's broken cexp. No -fomit-frame-pointer (broke
# the view on hardware in the RTCW port).
BASEFLAGS = -O2 -noixemul $(CPU) -fsigned-char -fno-strict-aliasing -fno-math-errno \
            -fno-builtin-sin -fno-builtin-cos -fno-builtin-sinf -fno-builtin-cosf \
            -DNDEBUG -DFINAL_BUILD -I$(MINIGL)/minigl-shared-library/include \
            -Wno-write-strings -Wno-narrowing \
            -fbbb=- \
            -MMD -MP

# -fbbb=-: bebbo's extra optimizer pass ("bbb") miscompiles. Found in
# build 0.1.6: G2_Init_Bolt_List (vector::clear on a reference argument)
# came out as
#	move.l ([8,a5],0),4(a0)
# with the load of a0 folded into the source operand but the destination
# still using the never loaded a0; it overwrote the caller's return
# address (a0 happened to hold the caller's frame pointer). Without bbb:
#	movea.l 8(a5),a0 / move.l (a0),4(a0)

# Game code: 4-byte aligned ints, as on the PC, so that data files read
# straight into structures line up.
GAME_CFLAGS = $(BASEFLAGS) -malign-int

# Code that talks to the operating system: the NDK structures assume the
# 68k's native 2-byte alignment, so no -malign-int here.
OS_CFLAGS = $(BASEFLAGS)

# The engine throws const char * from Com_Error (common.cpp) through game
# module frames, so everything keeps exceptions enabled.
CXXONLY = -std=gnu++98 -fpermissive -fno-operator-names

ENGINE_DEFS = -D_JK2EXE

# The module additionally needs -fno-common, so every variable it defines
# lands in .data or .bss between its marker objects.
MOD_CFLAGS = $(GAME_CFLAGS) -fno-common -I$(SRC)/icarus -I$(SRC)/game

LIBS = -L$(MINIGL)/lib -lminigl -lm

OBJDIR = objects

###########################################################################
# Engine (starwars.dsp without the Windows, force feedback, SmartHeap and
# encryption parts)

ENGINE_OBJS = \
	client/cl_cgame.o \
	client/cl_cin.o \
	client/cl_console.o \
	client/cl_input.o \
	client/cl_keys.o \
	client/cl_main.o \
	client/cl_mp3.o \
	client/cl_parse.o \
	client/cl_scrn.o \
	client/cl_ui.o \
	client/snd_ambient.o \
	client/snd_dma.o \
	client/snd_mem.o \
	client/snd_mix.o \
	client/snd_music.o \
	client/vmachine.o \
	qcommon/cm_load.o \
	qcommon/cm_patch.o \
	qcommon/cm_polylib.o \
	qcommon/cm_test.o \
	qcommon/cm_trace.o \
	qcommon/cmd.o \
	qcommon/common.o \
	qcommon/cvar.o \
	qcommon/files.o \
	qcommon/hstring.o \
	qcommon/md4.o \
	qcommon/msg.o \
	qcommon/net_chan.o \
	qcommon/strip.o \
	qcommon/unzip.o \
	game/genericparser2.o \
	game/q_math.o \
	game/q_shared.o \
	server/exe_headers.o \
	server/sv_ccmds.o \
	server/sv_client.o \
	server/sv_game.o \
	server/sv_init.o \
	server/sv_main.o \
	server/sv_savegame.o \
	server/sv_snapshot.o \
	server/sv_world.o \
	ghoul2/g2_api.o \
	ghoul2/g2_bolts.o \
	ghoul2/g2_bones.o \
	ghoul2/g2_misc.o \
	ghoul2/g2_surfaces.o \
	renderer/matcomp.o \
	renderer/tr_animation.o \
	renderer/tr_backend.o \
	renderer/tr_bsp.o \
	renderer/tr_cmds.o \
	renderer/tr_curve.o \
	renderer/tr_draw.o \
	renderer/tr_font.o \
	renderer/tr_ghoul2.o \
	renderer/tr_image.o \
	renderer/tr_init.o \
	renderer/tr_jpeg_interface.o \
	renderer/tr_light.o \
	renderer/tr_main.o \
	renderer/tr_marks.o \
	renderer/tr_mesh.o \
	renderer/tr_model.o \
	renderer/tr_noise.o \
	renderer/tr_quicksprite.o \
	renderer/tr_scene.o \
	renderer/tr_shade.o \
	renderer/tr_shade_calc.o \
	renderer/tr_shader.o \
	renderer/tr_shadows.o \
	renderer/tr_sky.o \
	renderer/tr_stl.o \
	renderer/tr_surface.o \
	renderer/tr_surfacesprites.o \
	renderer/tr_world.o \
	renderer/tr_worldeffects.o \
	ui/ui_atoms.o \
	ui/ui_connect.o \
	ui/ui_debug.o \
	ui/ui_main.o \
	ui/ui_shared.o \
	ui/ui_syscalls.o \
	jpeg-6/jcapimin.o \
	jpeg-6/jccoefct.o \
	jpeg-6/jccolor.o \
	jpeg-6/jcdctmgr.o \
	jpeg-6/jchuff.o \
	jpeg-6/jcinit.o \
	jpeg-6/jcmainct.o \
	jpeg-6/jcmarker.o \
	jpeg-6/jcmaster.o \
	jpeg-6/jcomapi.o \
	jpeg-6/jcparam.o \
	jpeg-6/jcphuff.o \
	jpeg-6/jcprepct.o \
	jpeg-6/jcsample.o \
	jpeg-6/jctrans.o \
	jpeg-6/jdapimin.o \
	jpeg-6/jdapistd.o \
	jpeg-6/jdatadst.o \
	jpeg-6/jdatasrc.o \
	jpeg-6/jdcoefct.o \
	jpeg-6/jdcolor.o \
	jpeg-6/jddctmgr.o \
	jpeg-6/jdhuff.o \
	jpeg-6/jdinput.o \
	jpeg-6/jdmainct.o \
	jpeg-6/jdmarker.o \
	jpeg-6/jdmaster.o \
	jpeg-6/jdpostct.o \
	jpeg-6/jdsample.o \
	jpeg-6/jdtrans.o \
	jpeg-6/jerror.o \
	jpeg-6/jfdctflt.o \
	jpeg-6/jidctflt.o \
	jpeg-6/jidctfst.o \
	jpeg-6/jmemmgr.o \
	jpeg-6/jmemnobs.o \
	jpeg-6/jutils.o \
	mp3code/cdct.o \
	mp3code/csbt.o \
	mp3code/csbtb.o \
	mp3code/csbtl3.o \
	mp3code/cup.o \
	mp3code/cupini.o \
	mp3code/cupl1.o \
	mp3code/cupl3.o \
	mp3code/cwin.o \
	mp3code/cwinb.o \
	mp3code/cwinm.o \
	mp3code/hwin.o \
	mp3code/l3dq.o \
	mp3code/l3init.o \
	mp3code/mdct.o \
	mp3code/mhead.o \
	mp3code/msis.o \
	mp3code/towave.o \
	mp3code/uph.o \
	mp3code/upsf.o \
	mp3code/wavep.o \
	amigaos/amiga_layout.o \
	amigaos/amiga_modules.o \
	amigaos/amiga_openal.o \
	amigaos/amiga_shared.o

OS_OBJS = \
	amigaos/amiga_glimp.o \
	amigaos/amiga_in.o \
	amigaos/amiga_layout.o \
	amigaos/amiga_libfix.o \
	amigaos/amiga_main.o \
	amigaos/amiga_snd.o \
	amigaos/amiga_watchdog.o

###########################################################################
# Game module (game.dsp). The marker objects must stay first and last.

game_OBJS = \
	amigaos/amiga_modmark_begin.o \
	icarus/BlockStream.o \
	icarus/Instance.o \
	icarus/Sequence.o \
	icarus/Sequencer.o \
	icarus/TaskManager.o \
	game/bg_misc.o \
	game/bg_pangles.o \
	game/bg_panimate.o \
	game/bg_pmove.o \
	game/bg_slidemove.o \
	cgame/cg_credits.o \
	cgame/cg_camera.o \
	cgame/cg_consolecmds.o \
	cgame/cg_draw.o \
	cgame/cg_drawtools.o \
	cgame/cg_effects.o \
	cgame/cg_ents.o \
	cgame/cg_event.o \
	cgame/cg_info.o \
	cgame/cg_lights.o \
	cgame/cg_localents.o \
	cgame/cg_main.o \
	cgame/cg_marks.o \
	cgame/cg_players.o \
	cgame/cg_playerstate.o \
	cgame/cg_predict.o \
	cgame/cg_scoreboard.o \
	cgame/cg_servercmds.o \
	cgame/cg_snapshot.o \
	cgame/cg_syscalls.o \
	cgame/cg_text.o \
	cgame/cg_view.o \
	cgame/cg_weapons.o \
	cgame/FX_ATSTMain.o \
	cgame/FX_Blaster.o \
	cgame/FX_Bowcaster.o \
	cgame/FX_BryarPistol.o \
	cgame/FX_DEMP2.o \
	cgame/FX_Disruptor.o \
	cgame/FX_Emplaced.o \
	cgame/FX_Flechette.o \
	cgame/FX_HeavyRepeater.o \
	cgame/FX_RocketLauncher.o \
	cgame/FxParsing.o \
	cgame/FxPrimitives.o \
	cgame/FxScheduler.o \
	cgame/FxSystem.o \
	cgame/FxTemplate.o \
	cgame/FxUtil.o \
	game/AI_Atst.o \
	game/AI_Default.o \
	game/AI_Droid.o \
	game/AI_GalakMech.o \
	game/AI_Grenadier.o \
	game/AI_Howler.o \
	game/AI_ImperialProbe.o \
	game/AI_Interrogator.o \
	game/AI_Mark1.o \
	game/AI_Mark2.o \
	game/AI_MineMonster.o \
	game/AI_Remote.o \
	game/AI_Seeker.o \
	game/AI_Sentry.o \
	game/AI_Sniper.o \
	game/AI_Jedi.o \
	game/AI_Stormtrooper.o \
	game/AI_Utils.o \
	game/g_active.o \
	game/g_breakable.o \
	game/g_camera.o \
	game/g_client.o \
	game/g_cmds.o \
	game/g_combat.o \
	game/g_functions.o \
	game/g_fx.o \
	game/g_ICARUS.o \
	game/g_itemLoad.o \
	game/g_inventory.o \
	game/g_items.o \
	game/g_main.o \
	game/g_mem.o \
	game/g_misc.o \
	game/g_misc_model.o \
	game/g_missile.o \
	game/g_mover.o \
	game/g_nav.o \
	game/g_navigator.o \
	game/g_navnew.o \
	game/g_object.o \
	game/g_objectives.o \
	game/g_ref.o \
	game/g_roff.o \
	game/g_savegame.o \
	game/g_session.o \
	game/g_spawn.o \
	game/g_svcmds.o \
	game/g_target.o \
	game/G_Timer.o \
	game/g_trigger.o \
	game/g_turret.o \
	game/g_usable.o \
	game/g_utils.o \
	game/g_weapon.o \
	game/g_weaponLoad.o \
	ui/gameinfo.o \
	game/NPC.o \
	game/NPC_behavior.o \
	game/NPC_combat.o \
	game/NPC_goal.o \
	game/NPC_misc.o \
	game/NPC_move.o \
	game/NPC_reactions.o \
	game/NPC_senses.o \
	game/NPC_sounds.o \
	game/NPC_spawn.o \
	game/NPC_stats.o \
	game/NPC_utils.o \
	game/Q3_Interface.o \
	game/Q3_Registers.o \
	game/q_math.o \
	game/q_shared.o \
	qcommon/tri_coll_test.o \
	game/wp_saber.o \
	amigaos/amiga_modmark_end.o

###########################################################################

all: $(OBJDIR)/jk2sp

ENGINE = $(addprefix $(OBJDIR)/engine/,$(ENGINE_OBJS))
OS     = $(addprefix $(OBJDIR)/os/,$(OS_OBJS))

$(OBJDIR)/engine/amigaos/amiga_layout.o: EXTRA_DEFS = -DAMIGA_LAYOUT_ENGINE

# Replaces C library functions; see the comments in the file.
# Built without -malign-int: it shares no structures with the game, and
# gcc 6.5.0b crashes on stb_sprintf.h with -O2 and -malign-int together.
# -fexceptions: its qsort calls the game's comparators, and Com_Error throws
# through it (without unwind tables that ends in std::terminate).
$(OBJDIR)/os/amigaos/amiga_libfix.o: EXTRA_DEFS = -fno-builtin -fexceptions

$(OBJDIR)/engine/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(GAME_CFLAGS) $(ENGINE_DEFS) $(EXTRA_DEFS) -c $< -o $@

$(OBJDIR)/engine/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(GAME_CFLAGS) $(CXXONLY) $(ENGINE_DEFS) $(EXTRA_DEFS) -c $< -o $@

$(OBJDIR)/os/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(OS_CFLAGS) $(ENGINE_DEFS) $(EXTRA_DEFS) -c $< -o $@

$(OBJDIR)/os/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(OS_CFLAGS) $(CXXONLY) $(ENGINE_DEFS) $(EXTRA_DEFS) -c $< -o $@


game_RAW = $(addprefix $(OBJDIR)/game/raw/,$(game_OBJS))
game_REN = $(addprefix $(OBJDIR)/game/ren/,$(game_OBJS))

$(OBJDIR)/game/raw/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(MOD_CFLAGS) $(EXTRA_DEFS) -c $< -o $@

$(OBJDIR)/game/raw/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(MOD_CFLAGS) $(CXXONLY) -include game_pch.h $(EXTRA_DEFS) -c $< -o $@

# Every global the module defines, mapped to game_<name>: functions, data,
# and the weak definitions C++ emits for inline functions, templates and
# vtables (W/V), so the module never shares one with the engine that was
# compiled with different settings. Undefined references (the C and C++
# libraries, the engine's exports reached through the import table) are
# left alone.
$(OBJDIR)/game/syms.txt: $(game_RAW)
	$(NM) -g --defined-only $^ | awk '$$2 ~ /^[TDBRWV]$$/ { print $$3 " _game_" substr($$3, 2) }' | sort -u > $@

$(OBJDIR)/game/ren/%.o: $(OBJDIR)/game/raw/%.o $(OBJDIR)/game/syms.txt
	@mkdir -p $(dir $@)
	$(OBJCOPY) --redefine-syms=$(OBJDIR)/game/syms.txt $< $@

# Every link gets the next build number (build_number.txt), compiled into
# amiga_build.c's version strings: the log's first line and Version show it.
BUILDFILE = build_number.txt

$(OBJDIR)/jk2sp: $(ENGINE) $(OS) $(game_REN) amiga_build.c
	@n=$$(( $$(cat $(BUILDFILE) 2>/dev/null || echo 0) + 1 )); \
	echo $$n > $(BUILDFILE); \
	echo "*** jk2sp build 0.1.$$n ***"; \
	$(CC) $(OS_CFLAGS) -DAMIGA_BUILD=$$n -c amiga_build.c -o $(OBJDIR)/amiga_build.o
	$(CXX) $(CPU) -noixemul -o $@.db $(ENGINE) $(OS) $(OBJDIR)/amiga_build.o $(game_REN) $(SWAPSTACK) $(LIBS)
	$(STRIP) --strip-unneeded --remove-section=.comment $@.db -o $@
	@# keep each build's symbols: watchdog reports from testers are offsets into it
	@mkdir -p symbols && cp $@.db symbols/jk2sp-0.1.$$(cat $(BUILDFILE)).db
	@# and the test package gets every successful build, so it is never stale
	@if [ -d ../../JK2_Pistorm ]; then cp $@ ../../JK2_Pistorm/jk2sp && echo "*** copied to JK2_Pistorm/jk2sp ***"; fi

# Standalone checks of the toolchain, to run on the Amiga.
tests: $(OBJDIR)/mathtest $(OBJDIR)/cxxtest

$(OBJDIR)/mathtest: tests/mathtest.c $(OBJDIR)/os/amigaos/amiga_libfix.o
	@mkdir -p $(dir $@)
	$(CC) $(GAME_CFLAGS) -o $@ $^ -lm

$(OBJDIR)/cxxtest: tests/cxxtest.cpp $(OBJDIR)/os/amigaos/amiga_libfix.o
	@mkdir -p $(dir $@)
	$(CXX) $(GAME_CFLAGS) -o $@ $^ -lm

clean:
	rm -rf $(OBJDIR)

.PHONY: all clean tests
.SECONDARY:

# Header dependencies written by -MMD next to each object.
-include $(shell find $(OBJDIR) -name "*.d" 2>/dev/null)
