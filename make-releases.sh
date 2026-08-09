#!/bin/bash

set -e

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
cd "$SCRIPT_DIR"

TIMESTAMP="$(date "+%Y%m%d%H%M%S")"
VERSION="$(cat res/version.txt)"

ARCH_PKG_DIR="pkg/slssteam"
RELEASE_DIR="releases"

if [ -d "$RELEASE_DIR" ]; then
	rm -rv "$RELEASE_DIR"
fi

make_release()
{
	NAME="$1"
	DIR="$RELEASE_DIR/$NAME"

	if [ ! -d "$DIR" ]; then
		mkdir -p "$DIR"
	fi

	sh docker/build.sh release
	mv zips/* $DIR
	rename "SLSsteam" "SLSsteam-Any-$NAME" $DIR/*

	cd "$ARCH_PKG_DIR"
	makepkg -Ccf
	cd "$SCRIPT_DIR"
	mv pkg/slssteam/*.pkg.tar.zst "$DIR"
	rename "slssteam" "SLSsteam-Arch-$NAME" $DIR/*

	rename "$VERSION" "$TIMESTAMP" $DIR/*
}

DEBUG=1 make_release "debug"
make_release "release"
