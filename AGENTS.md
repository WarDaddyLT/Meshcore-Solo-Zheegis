# Repository Guidelines

## Project Identity & Structure
**MeshCore Solo Žygis** lives at `WarDaddyLT/Meshcore-Solo-Zheegis`; `main` is the project branch. This is a MeshCore Solo fork with private tracking additions. Preserve upstream attribution and dependency license notices.

Networking lives in `src/`, hardware helpers in `src/helpers/`, firmware applications in `examples/`, board support in `variants/` and `boards/`, and Solo environments in `solo/`. Documentation is in `docs/`; tests/mocks are in `test/`.

## Tracking Feature Map
`examples/companion_radio/MyMesh.*` implements private reports, channel commands, route ACKs, discovery, and the GPS scheduler. `TrackerProtocol.h` handles targeting/parsing; `TrackerAudio.*` handles sound. `AbstractUITask.h`, `ui-new/`, and `src/helpers/ui/` provide fullscreen alerts and dismissal. Reuse `variants/lilygo_techo_lite/TEchoKeyShield.cpp` for keypad events; do not add a competing FIFO consumer.

Tracker configuration uses a separate versioned `/zygis1` record with atomic replacement. Coordinates and GPS enablement use Solo preferences. Preserve the existing `NodePrefs` schema. See `docs/tracking.md` and `docs/zygis_migration.md`.

## Build & Test Commands
Install PlatformIO and run from the repository root:

- `pio run -e YOUR_ENVIRONMENT`: build the selected target; see `docs/hardware.md` for current adapters.
- Add `-t upload --upload-port <port>` to flash; review partition migration instructions first.
- `python3 scripts/check_project_docs.py`: validate public documentation links and neutral examples.
- `pio device monitor -b 115200`: serial monitor; transport/CLI behavior depends on the build. See `docs/tracking-cli.md`.
- `pio test -e native -e native_kiss_modem -vv`: upstream host suites.
- `python3 test/tracking/test_status.py`: privacy, parsing, layout and LED checks; also run `test_cycle.py`, `test_dispatch.py`, and `test_persistence.py`.

## Coding & Behavioral Requirements
Use two spaces, attached braces, `camelCase` functions/variables, `PascalCase` classes, and `ALL_CAPS` macros. Follow `.clang-format` and avoid unrelated formatting. Avoid dynamic allocation outside setup/begin.

Gate tracking behavior with `PRIVATE_STATUS_REPORTER`; preserve ordinary Solo targets. Maintain private-channel binding, blocked automatic public location output, fresh-fix semantics, persistent settings, and nonblocking scheduling. Discovery observes neighbors; it does not calculate coordinates. Add meaningful regressions; no numeric coverage threshold is configured. Record physical GPS/audio/keypad/LED checks separately from builds and host tests.

## Commits & Pull Requests
Use concise, component-specific commit subjects. Target this fork's `main`, keep each PR focused, link related issues, describe affected boards and validation, and update feature documentation. Upstream contribution rules apply when submitting upstream.

## Community Documentation
Keep shared tracking documentation hardware-neutral. Put board-specific details in `docs/hardware.md` and developer CLI details in `docs/tracking-cli.md`. Keep English and Lithuanian introductions aligned on support and preview status. Use neutral device names such as `Tracker01`; never use personal family names in examples. Keep release/deployment workflows manual unless explicitly publishing a reviewed release.
