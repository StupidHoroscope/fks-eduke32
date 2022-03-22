
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
PROJECT(eduke32)

option(FUNKEYS no)

if (NOT DEFINED FUNKEYS)
  set(FUNKEYS no)
endif()

# Include Directories
INCLUDE_DIRECTORIES(
  build/include
  source/jmact
  source/jaudiolib/include
  source/enet/include
  vorbis/include
  vorbis/lib
  libogg/include
)

if (NOT FUNKEYS)
  include_directories(/usr/include/SDL)
endif()

# Source Lists
SET(SRC_FILES
  build/src/a-c.c
  build/src/baselayer.c
  build/src/cache1d.c
  build/src/compat.c
  build/src/crc32.c
  build/src/defs.c
  build/src/engine.c
  build/src/polymost.c
  build/src/hightile.c
  build/src/textfont.c
  build/src/smalltextfont.c
  build/src/kplib.c
  build/src/quicklz.c
  build/src/md4.c
  build/src/osd.c
  build/src/pragmas.c
  build/src/scriptfile.c
  build/src/mutex.c

  build/src/sdlayer.c
  build/src/polymer.c

  source/jmact/file_lib.c
	source/jmact/control.c
	source/jmact/keyboard.c
	source/jmact/mouse.c
	source/jmact/mathutil.c
	source/jmact/scriplib.c
	source/jmact/animlib.c

  source/game.c
	source/actors.c
	source/anim.c
	source/config.c
	source/demo.c
	source/gamedef.c
	source/gameexec.c
	source/gamevars.c
	source/global.c
	source/menus.c
	source/namesdyn.c
	source/net.c
	source/player.c
	source/premap.c
	source/savegame.c
	source/sector.c
	source/rts.c
	source/osdfuncs.c
	source/osdcmds.c
	source/grpscan.c
	source/sounds.c
  source/sdlmusic.c

  rsrc/game_icon.c

#  build/src/build.c

# source/sounds_mapster32.c
#  source/astub.c
#  source/m32def.c
#  source/m32exec.c
#  source/m32vars.c

  source/jaudiolib/src/vorbis.c
  source/jaudiolib/src/fx_man.c
  source/jaudiolib/src/multivoc.c
  source/jaudiolib/src/mix.c
  source/jaudiolib/src/mixst.c
  source/jaudiolib/src/pitch.c
  source/jaudiolib/src/drivers.c
  source/jaudiolib/src/driver_nosound.c
  source/jaudiolib/src/driver_sdl.c

  source/enet/src/host.c
  source/enet/src/list.c
  source/enet/src/packet.c
  source/enet/src/peer.c
  source/enet/src/callbacks.c
  source/enet/src/protocol.c
  source/enet/src/compress.c
  source/enet/src/unix.c

  vorbis/lib/vorbisfile.c
  vorbis/lib/vorbisenc.c
  vorbis/lib/synthesis.c
  vorbis/lib/smallft.c
  vorbis/lib/sharedbook.c
  vorbis/lib/res0.c
  vorbis/lib/registry.c
  vorbis/lib/psy.c
  vorbis/lib/mdct.c
  vorbis/lib/mapping0.c
  vorbis/lib/lsp.c
  vorbis/lib/lpc.c
  vorbis/lib/lookup.c
  vorbis/lib/info.c
  vorbis/lib/floor1.c
  vorbis/lib/floor0.c
  vorbis/lib/envelope.c
  vorbis/lib/codebook.c
  vorbis/lib/block.c
  vorbis/lib/bitrate.c
  vorbis/lib/analysis.c
  vorbis/lib/window.c

  libogg/src/framing.c
  libogg/src/bitwise.c
)

add_definitions(-DRENDERTYPESDL -DSUPERBUILD -DHAVE_VORBIS -DNOASM)
add_definitions(-D_GNU_SOURCE=1 -D_REENTRANT)

add_executable(eduke32 ${SRC_FILES})

set(OPTIMIZATIONS -O3 -flto -fomit-frame-pointer -fexpensive-optimizations
  -funsafe-math-optimizations -mvectorize-with-neon-quad -ftree-vectorize
  -funsigned-char
)

if (FUNKEYS)
  set(OPTIMIZATIONS ${OPTIMIZATIONS}
    -march=armv7-a -mtune=cortex-a7 -mfpu=neon -mfloat-abi=hard -ffast-math
  )
  target_link_libraries(eduke32 SDL_sound)
    
  link_directories(/opt/FunKey-sdk-2.3.0/arm-funkey-linux-musleabihf/sysroot/usr/lib/)
  include_directories(/opt/FunKey-sdk-2.3.0/arm-funkey-linux-musleabihf/sysroot/usr/include/SDL/)
else()
  target_link_libraries(eduke32 m)
endif()

target_compile_options(eduke32 PUBLIC "$<$<CONFIG:RELEASE>:${OPTIMIZATIONS}>")

target_link_libraries(eduke32 SDL SDL_mixer pthread)
