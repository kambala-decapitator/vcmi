#!/usr/bin/env bash

boostPrefix=~/dev/other/Apple-Boost-BuildScript/build/boost/1.75.0/ios/debug/prefix
ffmpegDir=~/dev/ios/vcmi-ios-deps/mobile-ffmpeg-min-universal-4.4
sdlLibsDir=~/dev/ios/vcmi-ios-deps/SDL2-lib
prefixPath="$boostPrefix;$ffmpegDir;$sdlLibsDir"

# prefixPath="$boostPrefix;$sdlLibsDir"
# xcodeMajorVersion=$(xcodebuild -version | fgrep Xcode | cut -d ' ' -f 2 | cut -d . -f 1)
# if [[ $xcodeMajorVersion -ge 12 ]]; then
#   extraVars=-DCMAKE_FRAMEWORK_PATH=~/dev/ios/vcmi-ios-deps/mobile-ffmpeg-min-gpl-4.4-xc12-frameworks
# else
#   prefixPath+=;~/dev/ios/vcmi-ios-deps/mobile-ffmpeg-min-universal
# fi

srcDir="../vcmi"
cmake "$srcDir" -G Xcode -T buildsystem=1 \
  -DBUNDLE_IDENTIFIER_PREFIX=com.kambala \
  -Wno-dev \
  -DCMAKE_TOOLCHAIN_FILE="$srcDir/ios.toolchain.cmake" \
  -DPLATFORM=${1:-OS64} \
  -DDEPLOYMENT_TARGET=11.0 \
  -DENABLE_BITCODE=0 \
  -DCMAKE_BINARY_DIR=$(pwd) \
  -DCMAKE_PREFIX_PATH="$prefixPath" \
  -DSDL2_INCLUDE_DIR=~/dev/ios/vcmi-ios-deps/SDL/include \
  -DSDL2_IMAGE_INCLUDE_DIR=~/dev/ios/vcmi-ios-deps/SDL_image-release-2.0.5 \
  -DSDL2_MIXER_INCLUDE_DIR=~/dev/ios/vcmi-ios-deps/SDL_mixer-release-2.0.4 \
  -DSDL2_TTF_INCLUDE_DIR=~/dev/ios/vcmi-ios-deps/SDL_ttf-release-2.0.15 \
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY='Apple Development' \
  -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM='4XHN44TEVG'
  # -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO
