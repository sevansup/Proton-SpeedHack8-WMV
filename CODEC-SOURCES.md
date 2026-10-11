# Codec sources and provenance

The added shared libraries are unmodified binaries from the official GE-Proton8-16 release:
https://github.com/GloriousEggroll/proton-ge-custom/releases/tag/GE-Proton8-16

Corresponding build scripts: https://github.com/GloriousEggroll/proton-ge-custom/tree/614f8bebd889eac7a2cf0da9f79826a1b56b4550

- FFmpeg (libavcodec, libavformat, libavfilter, libavutil): https://github.com/FFmpeg/FFmpeg/tree/a77521cd5d27e955b16e8097eecefc779ffdcb6d
- GStreamer gst-plugins-ugly ASF plugin: https://github.com/GStreamer/gstreamer/tree/bf6ce1d64a0697e7910826147b48f8f658366a5a/subprojects/gst-plugins-ugly

These source revisions match those pinned by Proton-SpeedHack8.0-5. GE enables WMV decoders and packages the libraries; SpeedHack's build disables decoders and relies on Steam's FFmpeg libraries. The source code for the codec libraries is unmodified; see GE's Makefile.in for its configure options. Full applicable license texts accompany this notice. Shared libraries remain replaceable.

Original SpeedHack source and Wine changes: https://github.com/LtSquigs/Proton-SpeedHack/tree/proton-speedhack-8.0-5

This experiment does not import GE's Wine builds, game fixes, DXVK, or VKD3D. It adds four required FFmpeg shared libraries and the ASF plugin in each architecture, with their soname links. The existing SpeedHack gst-libav plugin loads these libraries. The launcher appends Steam's optional video-library directory rather than putting it ahead of the bundled libraries.
