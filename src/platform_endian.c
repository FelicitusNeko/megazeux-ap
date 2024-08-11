/* MegaZeux
 *
 * Copyright (C) 2024 Alice Rowan <petrifiedrowan@gmail.com>
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

#include "compat.h"
#include "platform_endian.h"

#if defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64)
#define PLATFORM_IS_X86_64
#endif

#if defined(__i386__) || defined(_M_IX86) || defined(PLATFORM_IS_X86_64)
#define PLATFORM_IS_X86
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define PLATFORM_IS_ARM64
#endif

#if defined(__arm__) || defined(_M_ARM) || defined(PLATFORM_IS_ARM64)
#define PLATFORM_IS_ARM
#endif

#if defined(__ppc__) || defined(__POWERPC__) || \
 defined(__powerpc64__) || defined(__PPC64__)
#define PLATFORM_IS_PPC
#endif

#ifdef PLATFORM_IS_X86
#if defined(__GNUC__) || defined(__clang__)
#define HAS_GNU_INLINE_ASM
#endif

static inline boolean has_cpuid(void)
{
#ifdef PLATFORM_IS_X86_64
  return true;

#elif defined(HAS_GNU_INLINE_ASM)
  unsigned tmp;
  __asm__(
    "pushfd"                "\n\t" // save eflags
    "pop %%eax"             "\n\t" // load eflags
    "xor $0x200000, %%eax"  "\n\t" // invert ID bit in eflags
    "mov %%eax, %%ebx"      "\n\t"
    "push %%eax"            "\n\t" // save modified eflags
    "popfd"                 "\n\t"
    "pushfd"                "\n\t"
    "pop %%eax"             "\n\t" // reload modified(?) eflags
    "xor %%ebx, %%eax"      "\n\t" // should be 0
    : "=a"(tmp)
    :
    : "ebx"
  );
  if(tmp)
    return false;
  return true;

#else
  return false;
#endif
}
#endif

int platform_has_sse2(void)
{
#ifdef PLATFORM_IS_X86_64
  return true;

#elif defined(PLATFORM_IS_X86)
  static boolean checked = false;
  static boolean has_sse2 = false;
  if(!checked)
  {
    unsigned eax = 1;
    unsigned edx;

    if(has_cpuid())
    {
#ifdef HAS_GNU_INLINE_ASM
      __asm__(
        "cpuid"
        : "=d"(edx)
        : "a"(eax)
        : "ebx", "ecx"
      );
      if((edx & (3 << 25)) == 3 << 25) /* SSE + SSE2 */
        has_sse2 = true;
#endif
    }
    checked = true;
  }
  return has_sse2;

#else
  return false;
#endif
}

int platform_has_avx(void)
{
#ifdef PLATFORM_IS_X86
  static boolean checked = false;
  static boolean has_avx = false;
  if(!checked)
  {
    if(has_cpuid())
    {
#ifdef HAS_GNU_INLINE_ASM
#define CPUID_1_AVX_XGETBV ((1 << 28) | (1 << 27))
#define XGETBV_XMM_YMM_ENABLED ((1 << 2) | (1 << 1))
      int eax;
      int ecx;
      __asm__(
        "cpuid"
        : "=c"(ecx)
        : "a"(1)
        : "ebx", "edx"
      );
      if((ecx & CPUID_1_AVX_XGETBV) == CPUID_1_AVX_XGETBV)
      {
        __asm__(
          "xgetbv"
          : "=a"(eax)
          : "c" (0)
          : "edx"
        );
        if((eax & XGETBV_XMM_YMM_ENABLED) == XGETBV_XMM_YMM_ENABLED)
          has_avx = true;
      }
#endif
    }
    checked = true;
  }
  return has_avx;

#else
  return false;
#endif
}

int platform_has_avx2(void)
{
#ifdef PLATFORM_IS_X86
  static boolean checked = false;
  static boolean has_avx2 = false;

  if(!checked)
  {
    if(platform_has_avx())
    {
#ifdef HAS_GNU_INLINE_ASM
      int ebx;
      __asm__(
        "cpuid"
        : "=b"(ebx)
        : "a"(7), "c"(0)
        : "edx"
      );
      if(ebx & (1 << 5)) /* AVX2 */
        has_avx2 = true;
#endif
    }
    checked = true;
  }
  return has_avx2;

#else
  return false;
#endif
}

/* iOS processor feature detection header (32-bit). */
#if defined(PLATFORM_IS_ARM) && !defined(PLATFORM_IS_ARM64)
#if defined(__APPLE__) && defined(__arm__)
#include <sys/sysctl.h>
static boolean has_neon_check(void)
{
  size_t val = 0;
  size_t sz = sizeof(val);
  int ret = sysctlbyname("hw.optional.neon", &val, &sz, NULL, 0);
  return (ret == 0 && val);
}

/* Android API <18 feature detection via SDL (32-bit). */
#elif defined(__ANDROID__) && defined(__arm__) && \
 (!defined(__ANDROID_API__) || __ANDROID_API__ < 18)
#include "SDLmzx.h"
static boolean has_neon_check(void)
{
#if SDL_VERSION_ATLEAST(2,0,6)
  return SDL_HasNEON();
#else
  return false;
#endif
}

/* Linux processor feature detection headers (32-bit). */
#elif defined(__linux__) && defined(__arm__)
#include <sys/auxv.h>
#include <asm/hwcap.h>
static boolean has_neon_check(void)
{
  int tmp = getauxval(AT_HWCAP);
  return !!(tmp & HWCAP_NEON);
}

/* Assume AArch64, anything with intrinsics that reached this point has it. */
#elif defined(__ARM_NEON)
#define has_neon_check() true

/* No intrinsics and no runtime check available; disable. */
#else
#define has_neon_check() false
#endif
#endif /* PLATFORM_IS_ARM && !PLATFORM_IS_ARM64 */

int platform_has_neon(void)
{
#ifdef PLATFORM_IS_ARM64
  return true;

#elif defined(PLATFORM_IS_ARM)
  static boolean checked = false;
  static boolean has_neon = false;

  if(!checked)
  {
    has_neon = has_neon_check();
    checked = true;
  }
  return has_neon;

#else
  return false;
#endif
}

int platform_has_sve(void)
{
  return false;
}

#if (defined(__riscv) || defined(__riscv__)) && defined(__linux__)
#include <sys/auxv.h>
// The position of this is standard but none of the headers seem to agree
// on the define name or on which extensions actually get defines.
#define MZX_HWCAP_ISA_V ('V'-'A')

int platform_has_rvv(void)
{
  int tmp = getauxval(AT_HWCAP);
  return !!(tmp & MZX_HWCAP_ISA_V);
}
#else
int platform_has_rvv(void)
{
  return false;
}
#endif

#ifdef PLATFORM_IS_PPC
#if defined(__linux__)
#include <sys/auxv.h>
#include <asm/cputable.h>
static int has_altivec_check(void)
{
  int tmp = getauxval(AT_HWCAP);
  return !!(tmp & PPC_FEATURE_HAS_ALTIVEC) + !!(tmp & PPC_FEATURE_HAS_VSX);
}

#elif defined(CONFIG_SDL)
#include "SDLmzx.h"
static int has_altivec_check(void)
{
#if SDL_VERSION_ATLEAST(2,0,0)
  return SDL_HasAltiVec();
#else
  return false;
#endif
}

#else
#define has_altivec_check() false
#endif
#endif

int platform_has_altivec()
{
#ifdef PLATFORM_IS_PPC
  static boolean checked = false;
  static boolean has_altivec = false;

  if(!checked)
  {
    has_altivec = has_altivec_check() >= 1;
    checked = true;
  }
  return has_altivec;
#else
  return false;
#endif
}

int platform_has_altivec_vsx()
{
#ifdef PLATFORM_IS_PPC
  static boolean checked = false;
  static boolean has_vsx = false;

  if(!checked)
  {
    has_vsx = has_altivec_check() >= 2;
    checked = true;
  }
  return has_vsx;
#else
  return false;
#endif
}
