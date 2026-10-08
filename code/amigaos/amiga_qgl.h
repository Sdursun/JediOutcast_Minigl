/*
 * AmigaOS 3.x / MiniGL (PiStorm3D) compatibility layer for the renderer.
 *
 * Included from renderer/qgl.h after qgl_linked.h. It fills the gaps
 * between what the Quake 3 based renderer calls and what minigl.library
 * offers (written for the RTCW port, shared by the Jedi Outcast one).
 *
 * MiniGL's GL_* constants are a positional enum, not the real OpenGL
 * values. Every constant defined here is therefore given a value outside
 * both MiniGL's range (0..~240, 0x0BA5, 0x1702, 0x2400..0x2502, 0x84C0..)
 * and any real GL value the renderer might mix it up with.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef AMIGA_QGL_H
#define AMIGA_QGL_H

/* ---- constants MiniGL does not define ---------------------------------- */

/* minigl.library 29.1 defines several of these itself, with their real GL
   values (and takes GL_ADD, GL_LINE and normal arrays); its definitions win. */

/* texture_env_add is not offered; glimp leaves textureEnvAddAvailable
   false, so the renderer never passes this down. */
#ifndef GL_ADD
#define GL_ADD                          0x7A01
#endif

/* No stencil buffer. glimp reports stencilBits = 0, which makes the
   renderer skip stencil shadows (r_shadows 2) and r_measureOverdraw. */
#ifndef GL_STENCIL_TEST
#define GL_STENCIL_TEST                 0x7A02
#endif
#ifndef GL_STENCIL_INDEX
#define GL_STENCIL_INDEX                0x7A03
#endif
#ifndef GL_KEEP
#define GL_KEEP                         0x7A04
#endif
#ifndef GL_INCR
#define GL_INCR                         0x7A05
#endif
#ifndef GL_DECR
#define GL_DECR                         0x7A06
#endif
/* Used in glClear masks: must not set a bit MiniGL would act on. */
#ifndef GL_STENCIL_BUFFER_BIT
#define GL_STENCIL_BUFFER_BIT           0
#endif

/* No user clip planes: portals and mirrors render without the clip. */
#ifndef GL_CLIP_PLANE0
#define GL_CLIP_PLANE0                  0x7A07
#endif

#ifndef GL_NORMAL_ARRAY
#define GL_NORMAL_ARRAY                 0x7A08
#endif
#ifndef GL_TEXTURE_BORDER_COLOR
#define GL_TEXTURE_BORDER_COLOR         0x7A09
#endif

/* 16-bit RGBA texture request (r_texturebits 16), remapped below. */
#ifndef GL_RGBA4
#define GL_RGBA4                        0x7A0A
#endif

/* Wireframe (r_showtris and friends, debugging only). MiniGL has no line
   polygon mode and rejects the value, so polygons stay filled. */
#ifndef GL_LINE
#define GL_LINE                         0x7A0B
#endif

/* ---- functions MiniGL does not provide --------------------------------- */

#undef qglStencilFunc
#undef qglStencilOp
#undef qglStencilMask
#undef qglClearStencil
#undef qglClipPlane
#undef qglCallList
#undef qglTexParameterfv
#undef qglTexImage2D
#undef qglRasterPos3fv
#undef qglCallLists

#define qglStencilFunc( func, ref, mask )       ( (void)0 )
#define qglStencilOp( fail, zfail, zpass )      ( (void)0 )
#define qglStencilMask( mask )                  ( (void)0 )
#define qglClearStencil( s )                    ( (void)0 )
#define qglClipPlane( plane, equation )         ( (void)0 )
#define qglCallList( list )                     ( (void)0 )

/* Text labels on camera spline points (splines.cpp, editor debugging). */
#define qglRasterPos3fv( v )                    ( (void)0 )
#define qglCallLists( n, type, lists )          ( (void)0 )

/* The only glTexParameterfv call is the fog image's border colour, which
   MiniGL cannot take; anything else is forwarded as a scalar. */
#define qglTexParameterfv( target, pname, params ) \
	( ( pname ) == GL_TEXTURE_BORDER_COLOR ? (void)0 : glTexParameterf( target, pname, *( params ) ) )

/* The renderer still uses the GL 1.0 internal formats 3 and 4, which
   collide with unrelated members of MiniGL's enum (GL_ALPHA_BITS and
   GL_ALPHA_SCALE, neither a valid texture format). Map them, and the
   formats MiniGL lacks, onto ones it understands. 1 and 2 are left
   alone: in MiniGL they are GL_ALPHA and GL_ALPHA8. */
static __inline__ GLint AmigaQGL_InternalFormat( GLint fmt ) {
	switch ( fmt ) {
	case 3:         return GL_RGB;
	case 4:         return GL_RGBA;
	case GL_RGBA4:  return GL_RGBA;
	default:        return fmt;
	}
}

#define qglTexImage2D( target, level, internalformat, width, height, border, format, type, pixels ) \
	glTexImage2D( target, level, AmigaQGL_InternalFormat( internalformat ), width, height, border, format, type, pixels )

#endif
