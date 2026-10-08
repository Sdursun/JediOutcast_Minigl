# Jedi Outcast for AmigaOS / MiniGL

Star Wars Jedi Knight II: Jedi Outcast (single player) ported to AmigaOS 3.x on
PiStorm (Emu68) with PiStorm3D hardware 3D through `minigl.library`.

This is Raven Software's GPL source release of Jedi Outcast with an AmigaOS
port added. The Amiga specific code is in [code/amigaos](code/amigaos).

**Status: 1.0 beta 1.** The first level with its cutscenes is playable at about
25-30 fps (640x480, Pi 4). See the release notes for known issues.

## Download and installation

Get `JediOutcast_Minigl.lha` from the
[Releases](https://github.com/Sdursun/JediOutcast_Minigl/releases) page.
Unpack it and copy `assets0.pk3`, `assets1.pk3`, `assets2.pk3` and
`assets5.pk3` from the PC game's `GameData/base` folder into
`JediOutcast_Minigl/base`. The game data is not included; you need your own
copy of Jedi Outcast (GOG or Steam).

Requirements: PiStorm with Pi 4 / CM4 and Emu68, AmigaOS 3.2, RTG (Picasso96),
AHI, `minigl.library` 29.1 (PiStorm3D) and plenty of Fast RAM.

## Building

The build uses bebbo's m68k-amigaos-gcc in Docker
(`amigadev/m68k-amigaos-gcc:with-make`) and the PiStorm3D / minigl.library 29.1
SDK, unpacked so that `PiStorm3D/SDK` sits next to `code/`:

    cd code/amigaos
    ./build.sh -j8

The executable is `code/amigaos/objects/jk2sp`.

## Changes to the original source

This is a modified version of Raven Software's Jedi Outcast source release
(https://github.com/jedis/jedioutcast). Changes made in 2026 by Serkan Dursun
for the AmigaOS port:

- new files in [code/amigaos](code/amigaos) (platform layer, build, tools)
- in the original files, changes are inside `#ifdef __amigaos__` blocks, apart
  from portability fixes (`#include` paths, byte order handling of the game data
  formats, C++ fixes for GCC)
- [code/jpeg-6/jidctfst.cpp](code/jpeg-6/jidctfst.cpp) added from the
  Independent JPEG Group's libjpeg 6 (integer IDCT)

## License

Jedi Outcast source code copyright (C) 2002 Activision / Raven Software,
released under the GNU General Public License version 2, see
[LICENSE.txt](LICENSE.txt). The AmigaOS port is distributed under the same
license.

Parts not covered by the GPL, under their own licenses (see the notices in
the files):

- JPEG library: this software is based in part on the work of the Independent
  JPEG Group
- zlib, copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler
- stb_sprintf by Sean Barrett and Jeff Roberts (public domain / MIT)

Star Wars, Jedi Knight and Jedi Outcast are trademarks of Lucasfilm Ltd. This
project is not affiliated with Lucasfilm, LucasArts, Disney, Activision or
Raven Software.
