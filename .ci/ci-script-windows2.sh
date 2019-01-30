#!/bin/bash

set -e

export MSYS2_FC_CACHE_SKIP=1

pacman --noconfirm -Suy

pacman --noconfirm -S --needed \
    git \
    mingw-w64-$MSYS2_ARCH-exiv2 \
    mingw-w64-$MSYS2_ARCH-lcms2 \
    mingw-w64-$MSYS2_ARCH-lensfun \
    mingw-w64-$MSYS2_ARCH-dbus-glib \
    mingw-w64-$MSYS2_ARCH-openexr \
    mingw-w64-$MSYS2_ARCH-sqlite3 \
    mingw-w64-$MSYS2_ARCH-libxslt \
    mingw-w64-$MSYS2_ARCH-libsoup \
    mingw-w64-$MSYS2_ARCH-libwebp \
    mingw-w64-$MSYS2_ARCH-libsecret \
    mingw-w64-$MSYS2_ARCH-lua \
    mingw-w64-$MSYS2_ARCH-graphicsmagick \
    mingw-w64-$MSYS2_ARCH-openjpeg2 \
    mingw-w64-$MSYS2_ARCH-gtk3 \
    mingw-w64-$MSYS2_ARCH-pugixml \
    mingw-w64-$MSYS2_ARCH-libexif \
    mingw-w64-$MSYS2_ARCH-libgphoto2 \
    mingw-w64-$MSYS2_ARCH-flickcurl \
    mingw-w64-$MSYS2_ARCH-drmingw \
    mingw-w64-$MSYS2_ARCH-gettext \
    mingw-w64-$MSYS2_ARCH-iso-codes
    
