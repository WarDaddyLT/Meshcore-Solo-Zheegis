# Repository Guidelines

## Project Identity & Structure
**MeshCore Solo Žygis** lives at `WarDaddyLT/Meshcore-Solo-Zheegis`; `main` is the project branch. This is a MeshCore Solo fork with private tracking additions. Preserve upstream attribution and dependency license notices.

Networking lives in `src/`, hardware helpers in `src/helpers/`, firmware applications in `examples/`, board support in `variants/` and `boards/`, and Solo environments in `solo/`. Documentation is in `docs/`; tests/mocks are in `test/`.

## Tracking Feature Map
`examples/companion_radio/MyMesh.*` implements private reports, channel commands, route ACKs, discovery, and the GPS scheduler. `TrackerProtocol.h` handles targeting/parsing; `TrackerAudio.*` handles sound. `AbstractUITask.h`, `ui-new/`, and `src/helpers/ui/` provide fullscreen alerts and dismissal. Reuse `variants/lilygo_techo_lite/TEchoKeyShield.cpp` for keypad events; do not add a competing FIFO consumer.

Tracker configuration uses a separate versioned `/zygis1` record with atomic replacement. Coordinates and GPS enablement use Solo preferences. Preserve the existing `NodePrefs` schema. See `docs/techo_lite_status.md` and `docs/zygis_migration.md`.

## Build & Test Commands
Install PlatformIO and run from the repository root:

- `pio run -e LilyGo_T-Echo-Lite_keyshield_zygis`: build T-Echo Lite tracking with Solo UI.
- `pio run -e ThinkNode_M5_zygis`: build M5 tracking with its larger OTA partitions.
- Add `-t upload --upload-port <port>` to flash; review partition migration instructions first.
- `pio device monitor -b 115200`: configuration console; enter rescue mode, then use `exit` to restore USB framing.
- `pio test -e native -e native_kiss_modem -vv`: upstream host suites.
- `python3 test/techo_status/test_status.py`: privacy, parsing, layout and LED checks; also run `test_cycle.py`, `test_dispatch.py`, and `test_persistence.py`.

## Coding & Behavioral Requirements
Use two spaces, attached braces, `camelCase` functions/variables, `PascalCase` classes, and `ALL_CAPS` macros. Follow `.clang-format` and avoid unrelated formatting. Avoid dynamic allocation outside setup/begin.

Gate tracking behavior with `PRIVATE_STATUS_REPORTER`; preserve ordinary Solo targets. Maintain private-channel binding, blocked automatic public location output, fresh-fix semantics, persistent settings, and nonblocking scheduling. Discovery observes neighbors; it does not calculate coordinates. Add meaningful regressions; no numeric coverage threshold is configured. Record physical GPS/audio/keypad/LED checks separately from builds and host tests.

## Commits & Pull Requests
Use concise, component-specific commit subjects. Target this fork's `main`, keep each PR focused, link related issues, describe affected boards and validation, and update feature documentation. Upstream contribution rules apply when submitting upstream.
