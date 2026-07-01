#!/usr/bin/env bash

RELEASE_TAG="2026-03-11"
FILENAME="$1.txz"

downloadedFile="dependencies-android-armeabi-v7a"
gh run download 28517689856 \
	--repo vcmi/vcmi-dependencies \
	--dir "$RUNNER_TEMP" \
	--name "$downloadedFile"
conan cache restore "$RUNNER_TEMP/$downloadedFile.txz"
