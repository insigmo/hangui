#!/bin/bash
# Builds the Qt 5 probe against public Qt 5 headers and the target app's own Qt libraries.
#   QT5_INCLUDE  directory with QtCore/, QtGui/, QtQuick/, QtNetwork/ (any Qt 5.x)
#   TARGET_QT    directory with the target's libQt5*.so.5 (link-time check: a
#                symbol the target's Qt lacks fails here, not inside the app)
#   TARGET_RPATH where the target's Qt lives on the machine the probe runs on
set -euo pipefail
: "${QT5_INCLUDE:?}" "${TARGET_QT:?}" "${TARGET_RPATH:?}"
OUT="${OUT:-libhangui_qt5_probe.so}"
SRC="$(dirname "$0")/hangui_qt5_probe.cpp"
LINK="$(mktemp -d)"
for l in Core Gui Quick Qml Network; do ln -sf "$TARGET_QT/libQt5$l.so.5" "$LINK/libQt5$l.so"; done
g++ -std=c++11 -O2 -fPIC -shared -fvisibility=hidden \
    -I"$QT5_INCLUDE" -I"$QT5_INCLUDE/QtCore" \
    "$SRC" -o "$OUT" \
    -L"$LINK" -lQt5Quick -lQt5Gui -lQt5Network -lQt5Core \
    -static-libstdc++ -static-libgcc \
    -Wl,--no-undefined -Wl,--allow-shlib-undefined \
    -Wl,-rpath,"$TARGET_RPATH"
rm -rf "$LINK"
echo "built $OUT"
