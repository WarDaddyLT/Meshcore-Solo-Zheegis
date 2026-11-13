# Changelog

Žygis changes are tracked separately from the upstream MeshCore and Solo versions.

## Unreleased — community preview

### Added

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
