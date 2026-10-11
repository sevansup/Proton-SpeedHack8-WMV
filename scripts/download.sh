#!/usr/bin/env bash
set -euo pipefail
mkdir -p work/base work/ge
curl -fL --retry 3 https://github.com/LtSquigs/Proton-SpeedHack/releases/download/proton-speedhack-8.0-5/proton-speedhack-8.0-5.tar.gz -o work/speedhack.tar.gz
printf '%s  %s\n' 980e53aa988e06d4b70c77caef49223fb7897310672d020e6136eec9b346259b work/speedhack.tar.gz | sha256sum -c -
curl -fL --retry 3 https://github.com/GloriousEggroll/proton-ge-custom/releases/download/GE-Proton8-16/GE-Proton8-16.tar.gz -o work/ge.tar.gz
printf '%s  %s\n' 7c34cb1005fb634a256db3ba47e71d04b41477ac03881ee4ab918fc3725142ffe9fff49f6ad8a5104a1cef86e8187274be3c6bcd9a66ce810c3db1a4632be5bb work/ge.tar.gz | sha512sum -c -
tar --no-same-owner -xzf work/speedhack.tar.gz -C work/base
tar --no-same-owner -xzf work/ge.tar.gz -C work/ge
curl -fL --retry 3 https://samples.ffmpeg.org/V-codecs/WMV9/nokia_n90.wmv -o work/test.wmv
printf '%s  %s\n' a4145c073333b2b54d0f8dafaa75366e2e72b9d84f38d5a01294ed3fcf5b7e98 work/test.wmv | sha256sum -c -
