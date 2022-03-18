#!/bin/bash

version=$(grep "VERSION =" Makefile | head -1 | sed "s/.*= //" )
patchlevel=$(grep "PATCHLEVEL =" Makefile | head -1 | sed "s/.*= //")
sublevel=$(grep "SUBLEVEL =" Makefile | head -1 | sed "s/.*= //")
extraversion=$(grep "EXTRAVERSION =" Makefile | head -1 | sed "s/.*= //")

inclevel=$((sublevel + 1))
patch_url="https://cdn.kernel.org/pub/linux/kernel/v$version.x/incr/patch-$version.$patchlevel.$sublevel-$inclevel.xz"

echo "Upstreaming $version.$patchlevel.$sublevel to $version.$patchlevel.$inclevel..."

sed -i -e "0,/EXTRAVERSION/ s/\(EXTRAVERSION =\).*/\1/" Makefile

rm /tmp/patch.xz /tmp/patch
wget "$patch_url" -O /tmp/patch.xz
unxz /tmp/patch.xz

patch -Np1 < /tmp/patch
if [ ! $? -eq 0 ]; then
    echo "Patch failed to apply."
    exit 1
fi

echo "Applied patch."

rm /tmp/patch

sed -i -e "0,/EXTRAVERSION/ s/\(EXTRAVERSION =\)/\1 $extraversion/" Makefile
sed -i -e "0,/pkgver/ s/\(pkgver=$version.$patchlevel.\)$sublevel/\1$inclevel/" PKGBUILD

git add -A
git commit -sm "Merge $version.$patchlevel.$inclevel"

echo "Merged $version.$patchlevel.$inclevel."
