#!/usr/bin/env bash

RELEASE_TAG="2026-03-11"
FILENAME="$1.txz"
DOWNLOAD_URL="https://github.com/vcmi/vcmi-dependencies/actions/runs/28319294726/artifacts/7933962392"

downloadedFile="$RUNNER_TEMP/$FILENAME"
curl -Lo "$downloadedFile" "$DOWNLOAD_URL"
conan cache restore "$downloadedFile"
