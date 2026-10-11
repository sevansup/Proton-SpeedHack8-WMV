# Proton-SpeedHack8-WMV

**A custom Proton compatibility tool for Steam Deck that lets you speed up gameplay by holding a button or toggling acceleration on and off, with added support for WMV video playback.**

The WMV additions fix video playback. In my testing, the prerendered WMV cutscenes appear to play at normal speed even while the speedhack is active.

This is based on **[Proton SpeedHack by LtSquigs](https://github.com/LtSquigs/Proton-SpeedHack)**, which provides the speedhack functionality. I used its **8.0-5** release as the base and added WMV playback components from **[GE-Proton8-16 by GloriousEggroll](https://github.com/GloriousEggroll/proton-ge-custom/releases/tag/GE-Proton8-16)**. Full credit to those projects for the original work.

I put this together because I wanted the speedhack and working prerendered cutscenes in the same Proton build. **I've tested it on my Steam Deck with Fatal Frame / Project Zero: Maiden of Black Water, and gameplay, WMV cutscenes, and the speedhack are working.**

It may also help other games with WMV playback issues, such as **Nights of Azure 2** and other Koei Tecmo/Gust titles. I haven't tested Nights of Azure 2 with this build yet, but the added codec support isn't specific to Fatal Frame.

**[Download the latest release](https://github.com/sevansup/Proton-SpeedHack8-WMV/releases/latest)** — no local Linux build tools or Docker setup required.

## Installation

1. Download **Proton-SpeedHack8-WMV.zip** from the release assets.
2. In Steam Deck Desktop Mode, extract it into `~/.steam/root/compatibilitytools.d/`.
3. Restart Steam and select **Proton-SpeedHack8-WMV** under the game's Properties → Compatibility settings.
4. When trying another game, check its cutscenes at normal speed first, then try gameplay acceleration.

## Speed and controls

The examples below use **2× gameplay speed**. Change `SPEEDHACK_SPEED=2` to choose another multiplier.

Map a Steam Input button to **Page Up**, then choose either control mode:

**Hold to speed up** — hold the button for 2× speed; release it to return to normal:

```text
SPEEDHACK_SPEED=2 SPEEDHACK_TOGGLE=0 SPEEDHACK_KEYS=page_up %command%
```

**Toggle on/off** — press once for 2× speed; press again to return to normal:

```text
SPEEDHACK_SPEED=2 SPEEDHACK_TOGGLE=1 SPEEDHACK_KEYS=page_up %command%
```

`SPEEDHACK_TOGGLE=0` selects hold mode; `SPEEDHACK_TOGGLE=1` selects toggle mode. Audio time stretching is not added, and WMV playback support does not mean the videos themselves will speed up.

## What changes

All original Wine binaries, speedhack code, DXVK, VKD3D, and existing GStreamer plugins remain byte-identical. The additions are four FFmpeg libraries (`avcodec`, `avformat`, `avfilter`, `avutil`) and GStreamer's ASF demuxer, for both architectures, including runtime soname links.

SpeedHack and GE pin the same FFmpeg and GStreamer source revisions. The launcher changes one library-path operation to prefer bundled codecs over Steam's supplied FFmpeg libraries. The Steam display name is distinct.

Every original runtime file, symlink, and file mode is checked during assembly. The validated package preserves all 6,606 original runtime entries and adds 18 codec files/library links. See `changed-files.json` in the release assets for the audit. License texts and exact source links accompany the codecs.

## Testing

- **Steam Deck game test:** I verified working gameplay, cutscenes, and the speedhack in Fatal Frame / Project Zero: Maiden of Black Water on my Steam Deck.
- **Game video sample:** all 2,028 frames of a 1920×1080 WMV3 Main-profile video decoded through SpeedHack's original GStreamer libraries with these additions.
- **32-bit and 64-bit media tests:** complete ASF/WMV3 video decoding and WMA2 audio decoding using a checksum-pinned public sample.
- **32-bit and 64-bit speed tests:** verified 1× → 3× → 1× Wine timing.
- **Package checks:** original-runtime file audit, ZIP integrity, and SHA-256 verification passed.

[View the successful automated test run](https://github.com/sevansup/Proton-SpeedHack8-WMV/actions/runs/38171535880).

These results establish a working WMV-capable SpeedHack build and a successful Steam Deck game test. They do not guarantee compatibility with every game or every cutscene. Game video files are not included in the repository or downloads.

## Troubleshooting

If a game crashes, add `PROTON_LOG=1` before the other launch options. On a typical Steam Deck installation the log is `~/steam-<AppID>.log` (for example, `~/steam-1732190.log` for Fatal Frame). Preserve the log before the next launch overwrites it.

## Sources and credits

Thanks to [LtSquigs](https://github.com/LtSquigs/Proton-SpeedHack) for Proton SpeedHack and [GloriousEggroll](https://github.com/GloriousEggroll/proton-ge-custom) for GE-Proton.

See [CODEC-SOURCES.md](CODEC-SOURCES.md) for pinned sources and licensing. Build automation verifies the original release archives' SHA-256/SHA-512 values before using them.
