/* MegaZeux
 *
 * Copyright (C) 2008 Alistair John Strachan <alistair@devzero.co.uk
 * Copyright (C) 2020, 2024 Alice Rowan <petrifiedrowan@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __ENDIAN_H
#define __ENDIAN_H

/* Use GCC/clang or a list of architectures (both checks borrowed from SDL) to
 * determine the endianness. If SDL is enabled, platform_sdl.c will check this.
 */

#define PLATFORM_LIL_ENDIAN 0x1234
#define PLATFORM_BIG_ENDIAN 0x4321

#if defined(__BIG_ENDIAN__)
#define PLATFORM_BYTE_ORDER PLATFORM_BIG_ENDIAN
#elif defined(__LITTLE_ENDIAN__)
#define PLATFORM_BYTE_ORDER PLATFORM_LIL_ENDIAN
/* predefs from newer gcc and clang versions: */
#elif defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__) && defined(__BYTE_ORDER__)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define PLATFORM_BYTE_ORDER PLATFORM_LIL_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define PLATFORM_BYTE_ORDER PLATFORM_BIG_ENDIAN
#else
#error Unsupported endianness
#endif /**/
#elif defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    ((defined(__mips__) || defined(__mips) || defined(__MIPS__)) && defined(__MIPSEB__)) || \
    defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
    defined(__s390__) || defined(__s390x__) || defined(__zarch__) || defined(__SYSC_ZARCH__) || \
    defined(__sparc__)
#define PLATFORM_BYTE_ORDER PLATFORM_BIG_ENDIAN
#else
#define PLATFORM_BYTE_ORDER PLATFORM_LIL_ENDIAN
#endif

/* ModPlug and XMP both use this name to find out about endianness. It's not
 * too bad to pollute our namespace with it, so just do so here.
 */
#if PLATFORM_BYTE_ORDER == PLATFORM_BIG_ENDIAN
#define WORDS_BIGENDIAN
#endif

/**
 * Also try to get the platform bit width.
 * Emscripten natively supports 64-bit math when compiling to Wasm.
 */
#if defined(_WIN64) || defined(__EMSCRIPTEN__) || \
  (defined(__sparc__) && defined(__arch64__)) || \
  ((defined(__riscv) || defined(__riscv__)) && __riscv_xlen >= 64) || \
  ((defined(__mips__) || defined(__mips) || defined(__MIPS__)) && \
    defined(_MIPS_SIM) && defined(_ABI64) && _MIPS_SIM == _ABI64) || \
  (defined(__GNUC__) && \
    (defined(__x86_64__) || defined(__powerpc64__) || defined(__PPC64__) || \
     defined(__aarch64__) || defined(__alpha__) || \
     defined(__s390x__) || defined(__zarch__)))
#define ARCHITECTURE_BITS 64
#else
#define ARCHITECTURE_BITS 32
#endif

/**
 * Also define some useful constants for byte alignment.
 */
#define ALIGN_16_MODULO 0x02

/**
 * Some 32-bit-capable processors (such as the Motorola 68000) still align
 * their data to 16-bit boundaries.
 */
#if defined(__m68k__) || defined(mc68000) || defined(_M_M68K)
#define ALIGN_32_MODULO ALIGN_16_MODULO
#else
#define ALIGN_32_MODULO 0x04
#endif

#if ARCHITECTURE_BITS >= 64
#define ALIGN_64_MODULO 0x08
#else
#define ALIGN_64_MODULO ALIGN_32_MODULO
#endif

/**
 * Functions for extended processor feature detection.
 */

#ifdef __cplusplus
extern "C" {
#endif
int platform_has_sse2(void);
int platform_has_avx(void);
int platform_has_avx2(void);
int platform_has_neon(void);
int platform_has_sve(void);
int platform_has_rvv(void);
#ifdef __cplusplus
}
#endif

#endif // __ENDIAN_H
