# Changelog

Žygis changes are tracked separately from the upstream MeshCore and Solo versions.

## Veylith Arena CPU preview — 2026-11-15

- Add on-demand Arena Trainer, manual stat/XP training, 20 families/60 forms,
  four-slot loadouts chosen from level/element/permanent-stat gated pools and
  deterministic three-creature CPU battles. Add short descriptions for all
  80 moves and 60 forms, with readable cards and exact requirement checks.
- Integrate 60 supplied monochrome front sprites in flash; mirror the player
  at render time and remove separate rear/duplicate exports. Preserve prototype-art
  warnings. Add an isolated Arena save and Games/Favourites entries.
- Add native progression, corruption/recovery, OLED bounds and 10,000 CPU-match
  checks. Arena PvP and spectators remain future work.

## Unreleased — community preview

### Added

- Channel-only L1 family profile: no ordinary address book or chat history/
  backlog; eight transient direct repeaters, four live companion frames, and
  the full GPS trail buffer. Measured 110,456 bytes reclaimed.
- Small private-channel tools with game/channel pins; existing contact files
  and contact pins preserved for regular Solo.

- Automatic one-second lobby discovery and joining, host handles in the match
  list, and one-press rematches with session/round/move IDs for replay protection.
  Multiplayer protocol version 2 requires both players to update.

- Lightweight L1 six-game production profile with USB debug hooks, game timing
  counters and screenshots compiled out; the development profile retains them.

- Offline heads-up Texas Hold’em vs CPU with virtual chips, legal betting,
  all-ins/refunds, complete hand ranking, hidden CPU cards and quick launch.

- Offline Scorched Earth with bounded CPU aiming, fixed-point trajectories,
  wind, heavy shells, destructible terrain, falling damage and quick launch.

- Offline Battleship vs CPU with manual/automatic fleet placement, rotation,
  target/fleet views and a quick-launch shortcut.

- Offline 8×8 Checkers with Easy/Normal CPU, backward captures, flying kings,
  immediate promotion during capture chains, and an optional quick-launch pin.

- Offline Snake with speed selection, pause/restart, score, collision handling
  and a quick-launch shortcut; its state is freed when the game closes.

- Offline Tic-Tac-Toe preview with three CPU levels, X/O selection and pause/restart
  controls; see [Games](docs/games.md) for the build target and controls.
- Visible Tools → Games category and persistent shortcuts for all six games in
  the home Favourites quick-launch picker, using the existing preference layout.
- Two-player Tic-Tac-Toe preview over existing private
  channels, with automatic joining, ordered moves and bounded gameplay retries.
- On-demand screen allocation in the games build: inactive UI/chat/game state
  is released while Žygis tracking and companion communications continue.
- Wio Tracker L1 / L1 Pro OLED tracking target based on the Solo BLE/USB build.
- Wio Tracker L1 e-ink tracking target based on the Solo BLE/USB build.

- Private-channel GPS/battery reports, remote tracking controls, and persistent
  channel binding independent of Solo's preference schema.
- Targeted messages, fullscreen alerts, repeating alarms and route ACKs.
- Direct-repeater discovery and T-Echo Lite speaker/keypad/status-LED integration.
- Dedicated T-Echo Lite + KeyShield and ThinkNode M5 tracking environments.
- Regression checks for privacy, dispatch, GPS cycling, layout, LEDs and persistence.
- English and Lithuanian project introductions, community templates and CI.

### Documentation

- Generalized the tracking guide and test suite names; separated board adapters
  and developer CLI details from shared usage documentation.

### Fixed

- Settings flush pending edits on unload; rotation edits save immediately.

- Tracking channel bindings and intervals now survive reboot when unused channel-name
  bytes are nonzero; existing terminated names remain readable.
- Routed tracking commands from normal USB terminals and the companion app's
  protocol-v14 local CLI; moved screenshot requests/replies to `0x7E`.
- Restored the portrait home calendar and added Settings → Tracking with shared
  validation/persistence for channel, interval and tracking controls.


- Native Linux simulator builds now include the fixed-width integer types used
  by the vendored polyline codec explicitly.

### Project foundation

- Tracking port: `17bfaf9d`.
- Solo base: `3d70c515` from `MarekZegare4/MeshCore-Solo`.
- Original MeshCore base: `66e7ff37ced46b40c8243a01606817e35a157401`.

This is a source preview. A field-tested Žygis firmware release has not yet been
published. See [ROADMAP.md](ROADMAP.md) for the remaining validation.
