# MegaZeux libxmp fork.

This is a fork of libxmp generated specifically for MegaZeux
from this branch using the file `mzx-gen.sh`: https://github.com/AliceLR/libxmp/tree/mzx-gen-libxmp

The script that generated this forked libxmp from commit
[e6e28ade](https://github.com/cmatsuoka/libxmp/commit/e6e28ade005052a317a5251fb6068696d22a3dac)
and applied the following branches as patches:

* [mzx-integration-hacks](https://github.com/AliceLR/libxmp/tree/mzx-integration-hacks):
  adds some defines to the start of xmp.h to hack various things to act
  the way MZX would prefer, since this copy of libxmp needs to build static
  against MZX in the MZX build system.
* [mzx-remove-extra-formats](https://github.com/AliceLR/libxmp/tree/mzx-remove-extra-formats):
  removes some of the more obscure formats supported by libxmp that were not
  likely to have been used. This means effectively everything that wasn't
  supported by the MegaZeux fork of libmodplug except for things that could
  be mistaken for other formats (misc. formats for MOD, MED derivatives for MED).
  This keeps the MZX supported formats list roughly the same between libmodplug,
  mikmod, and libxmp and helps reduce executable size for embedded platforms.
* [fix-it-sample-volume-vibrato](https://github.com/AliceLR/libxmp/tree/fix-it-sample-volume-vibrato):
  tweaks the way sample volume and vibrato are handled for IT files.
  This has not been contributed upstream because I'm not convinced this is the
  "correct" fix yet, despite it seeming to fix these issues on the surface level.
  Relevant upstream issue(s):
  https://github.com/cmatsuoka/libxmp/issues/102
  https://github.com/cmatsuoka/libxmp/issues/103

This patch automatically copies only the files required by MegaZeux, so manual file
removal should no longer be necessary.

See README and docs/COPYING.LIB for more libxmp information and the libxmp license.
See the upstream repository for the original libxmp, which you should probably use
instead of this hacked up copy for most purposes: https://github.com/cmatsuoka/libxmp/

## Pending MZX-specific hacks

* asie contributed a patch for tracker detection removal in the S3M and IT loaders.
  This is not currently used by MZX and could save about 2-3kb RAM. This might be
  worth looking into someday. https://github.com/AliceLR/megazeux/pull/202