#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
FT_ROOT="$ROOT_DIR/third_party/freetype"
OUT_DIR="$ROOT_DIR/.build"
OBJ_DIR="$OUT_DIR/freetype_obj_unix"
OUT_LIB="$OUT_DIR/libfreetype.a"

BUILD_TYPE="release"
REBUILD=0

for arg in "$@"; do
  case "$arg" in
    debug) BUILD_TYPE="debug" ;;
    release) BUILD_TYPE="release" ;;
    rebuild) REBUILD=1 ;;
  esac
done

mkdir -p "$OUT_DIR"

if [ "$REBUILD" -eq 1 ]; then
  rm -rf "$OBJ_DIR"
  rm -f "$OUT_LIB"
fi

mkdir -p "$OBJ_DIR"

CC="${CC:-cc}"
AR="${AR:-ar}"

if ! command -v "$CC" >/dev/null 2>&1; then
  echo "Error: compiler '$CC' not found in PATH."
  exit 1
fi
if ! command -v "$AR" >/dev/null 2>&1; then
  echo "Error: archiver '$AR' not found in PATH."
  exit 1
fi

BASE_CFLAGS="-c -DFT2_BUILD_LIBRARY -D_REENTRANT -I$FT_ROOT/include -I$FT_ROOT"
if [ "$BUILD_TYPE" = "debug" ]; then
  CFLAGS_USE="-O0 -g -D_DEBUG"
else
  CFLAGS_USE="-O2 -DNDEBUG"
fi

SYSTEM="$(uname -s 2>/dev/null || echo unknown)"
FTSYSTEM_SRC="src/base/ftsystem.c"
case "$SYSTEM" in
  Linux|Darwin)
    FTSYSTEM_SRC="builds/unix/ftsystem.c"
    ;;
esac

SOURCES="
src/autofit/autofit.c
src/base/ftbase.c
src/base/ftbbox.c
src/base/ftbdf.c
src/base/ftbitmap.c
src/base/ftcid.c
src/base/ftfstype.c
src/base/ftgasp.c
src/base/ftglyph.c
src/base/ftgxval.c
src/base/ftinit.c
src/base/ftmm.c
src/base/ftotval.c
src/base/ftpatent.c
src/base/ftpfr.c
src/base/ftstroke.c
src/base/ftsynth.c
src/base/fttype1.c
src/base/ftwinfnt.c
src/base/ftdebug.c
src/bdf/bdf.c
src/bzip2/ftbzip2.c
src/cache/ftcache.c
src/cff/cff.c
src/cid/type1cid.c
src/gzip/ftgzip.c
src/lzw/ftlzw.c
src/pcf/pcf.c
src/pfr/pfr.c
src/psaux/psaux.c
src/pshinter/pshinter.c
src/psnames/psnames.c
src/raster/raster.c
src/sdf/sdf.c
src/sfnt/sfnt.c
src/smooth/smooth.c
src/svg/svg.c
src/truetype/truetype.c
src/type1/type1.c
src/type42/type42.c
src/winfonts/winfnt.c
"

echo "Building FreeType ($BUILD_TYPE) with $CC..."

for rel_src in $SOURCES; do
  src="$FT_ROOT/$rel_src"
  obj="$OBJ_DIR/$(basename "$rel_src" .c).o"
  "$CC" $BASE_CFLAGS $CFLAGS_USE -o "$obj" "$src"
done

"$CC" $BASE_CFLAGS $CFLAGS_USE -o "$OBJ_DIR/ftsystem.o" "$FT_ROOT/$FTSYSTEM_SRC"

"$AR" rcs "$OUT_LIB" "$OBJ_DIR"/*.o
echo "FreeType built: $OUT_LIB"
