# Changelog

All notable changes to Montagem Finisher are documented here. Format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versioning
follows [Semantic Versioning](https://semver.org/) adapted for a pre-1.0
beta:

- **PATCH** (0.x.y): bug fixes only, no behavior/feature changes
- **MINOR** (0.x.0): new features or notable user-facing changes
- **MAJOR** (1.0.0+): first stable release, then breaking changes only

## [0.2.0] - 2026-07-15

### Fixed
- No per-channel bus layout restriction meant JUCE's default negotiation
  could in theory accept a >2-channel configuration for this plugin's
  matched-in/out-count bus; every internal state array here is fixed at 2
  elements, so that would have indexed them out of bounds. Now explicitly
  restricted to stereo.

### Added
- Resizable window (360x280 up to 900x700); the knob now scales with the
  available space instead of staying a fixed size.

## [0.1.0] - 2026-07-11

### Added
- First public beta: one-knob drive/tone/loudness finisher, VST3/AU/
  Standalone on macOS and Windows.
