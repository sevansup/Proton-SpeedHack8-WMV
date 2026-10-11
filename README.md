# Proton-SpeedHack8-WMV

Experimental WMV playback additions for the official **Proton-SpeedHack 8.0-5** release, using codec binaries from **GE-Proton8-16**. Designed for testing Fatal Frame: Maiden of Black Water. No local Linux or Docker setup is needed; GitHub Actions creates the installable ZIP.

**Validation:** 32-bit and 64-bit WMV3/WMA decoding and speed-switching tests passed in [this workflow run](https://github.com/sevansup/Proton-SpeedHack8-WMV/actions/runs/38171535880). In-game Steam Deck testing is still required.

## Scope

All original Wine binaries, speedhack code, DXVK, VKD3D, and existing GStreamer plugins remain byte-identical. The additions are four FFmpeg libraries (avcodec, avformat, avfilter, avutil) and GStreamer's ASF demuxer, for both architectures, including runtime soname links. SpeedHack and GE pin the same FFmpeg and GStreamer source revisions. The launcher changes one library-path operation to prefer bundled codecs over Steam's supplied FFmpeg libraries. The Steam display name is distinct.

Every original runtime file, symlink, and file mode is checked during assembly. No original runtime file may be changed or removed. See `changed-files.json` in the download for the audit. License texts and exact source links accompany the codecs.

## Validation

Local investigation decoded all 2,028 frames of a user-supplied 1920x1080 WMV3 Main-profile Fatal Frame video (EV1601.wmv) through the original SpeedHack GStreamer libraries with these additions. The uploaded game video is not published or included in downloads. Under the same local setup the baseline returned a short 120-frame fallback stream. This does not reproduce the user's full Steam runtime environment.

The workflow tests both 32-bit and 64-bit ASF/WMV3 decoding and WMA2 audio using a checksum-pinned public FFmpeg sample (677 video frames), then verifies 1x -> 3x -> 1x Wine timing. These checks do not establish that Fatal Frame's Wine playback path, save loading, Steam Input hotkey, or every cutscene works. Testing in the game on Steam Deck remains necessary.

## Install and test

1. Download **Proton-SpeedHack8-WMV-download** from a successful Actions run and extract the inner `Proton-SpeedHack8-WMV.zip`.
2. Extract that inner ZIP on the Steam Deck into `~/.steam/root/compatibilitytools.d/`.
3. Restart Steam and select **Proton-SpeedHack8-WMV** under the game's Compatibility settings.
4. Test loading the same save and playing the affected cutscene at normal speed first, then toggle acceleration.

Launch options for a Page Up toggle at 3x:

```
SPEEDHACK_SPEED=3 SPEEDHACK_TOGGLE=1 SPEEDHACK_KEYS=page_up %command%
```

Map a Steam Input button to Page Up. The original SpeedHack configuration behavior is retained. Audio time stretching is not added.

If a game crashes, add `PROTON_LOG=1` before the other launch options. On a typical Deck installation the log is `~/steam-1732190.log` for Fatal Frame. Preserve the log before the next launch overwrites it.

This is a separate experiment from GE-Proton8-16-SpeedHack. Its code does not modify that repository or an installed copy of either tool.

See [CODEC-SOURCES.md](CODEC-SOURCES.md) for pinned sources and licensing. Build automation checks the original release archives' SHA-256/SHA-512 values before using them.
