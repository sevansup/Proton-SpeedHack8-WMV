#!/usr/bin/env bash
set -euo pipefail
name=Proton-SpeedHack8-WMV
# Preserve upstream's normal first-launch extraction layout.
tar -czf "output/$name/proton_dist.tar.gz" -C "output/$name/dist" .
cd output
zip -q -0 -r -y "$name.zip" "$name" -x "$name/dist/*" "$name/dist/"
unzip -tq "$name.zip"
sha256sum "$name.zip" > "$name.zip.sha256"
