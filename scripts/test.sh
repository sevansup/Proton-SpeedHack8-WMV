#!/usr/bin/env bash
set -euo pipefail
project=$(pwd)
dist="$project/output/Proton-SpeedHack8-WMV/dist"
export CCACHE_DISABLE=1
for bits in 64 32; do
  if [[ "$bits" == 64 ]]; then host=x86_64-linux-gnu; pkg_host=x86_64-linux-gnu; lib=lib64; else host=i686-linux-gnu; pkg_host=i386-linux-gnu; lib=lib; fi
  flags=$(PKG_CONFIG_LIBDIR="/usr/lib/$pkg_host/pkgconfig:/usr/share/pkgconfig" "$host-pkg-config" --cflags --libs gstreamer-1.0)
  "$host-gcc" -O2 scripts/test-media.c -L"$dist/$lib" -Wl,-rpath-link,"$dist/$lib" $flags -o "work/test-media$bits"
  LD_LIBRARY_PATH="$dist/$lib" GST_PLUGIN_SYSTEM_PATH_1_0="$dist/$lib/gstreamer-1.0" \
    GST_PLUGIN_PATH_1_0= GST_REGISTRY_FORK=no GST_REGISTRY="$project/work/registry$bits.bin" \
    "work/test-media$bits" "$project/work/test.wmv"
done
for bits in 64 32; do
  if [[ "$bits" == 64 ]]; then cross=x86_64-w64-mingw32; else cross=i686-w64-mingw32; fi
  "$cross-gcc" -O2 scripts/timer-test.c -o "work/timer$bits.exe"
done
export LD_LIBRARY_PATH="$dist/lib64:$dist/lib:$dist/lib64/wine/x86_64-unix:$dist/lib/wine/i386-unix"
export WINEPREFIX="$project/work/test-prefix" WINESERVER="$dist/bin/wineserver"
export WINEARCH=win64 WINEDEBUG=-all WINEDLLOVERRIDES='winemenubuilder.exe=d'
export WINEESYNC=0 WINEFSYNC=0
trap '"$WINESERVER" -k || true' EXIT
timeout 120 "$dist/bin/wine64" "$project/work/timer64.exe"
timeout 120 "$dist/bin/wine" "$project/work/timer32.exe"
"$WINESERVER" -w
