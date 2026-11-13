# Tracking CLI implementation reference

This is a developer reference for `MyMesh::handleStatusCommand` in
[`examples/companion_radio/MyMesh.cpp`](../examples/companion_radio/MyMesh.cpp).
The commands below are handled by the current `PRIVATE_STATUS_REPORTER` implementation.

## Choosing an interface

- **Android/iOS app:** reconnect after a firmware update so the app reads protocol
  version 14. Its local companion CLI sends command 66 and receives text reply 29.
- **USB terminal:** open the device port at 115200 baud and send commands ending
  in CR, LF or CRLF. Tracker commands work during normal operation; rescue-mode
  entry is not required. Binary companion frames remain supported.
- **On the device:** open **Settings → Tracking** for tracking on/off, private
  channel selection, interval editing, GPS state, report-now and channel checks.

All three interfaces use the same tracking command handler and saved preferences.
A private channel must already exist before selecting it. Selecting a channel and
turning tracking on are user actions; testing does not silently configure them.

The general guide is [tracking.md](tracking.md); board details are in
[hardware.md](hardware.md).

## Implemented commands

| CLI | Meaning |
| --- | --- |
| `help` / `status help` | Configuration commands |
| `status channels` | First three channel slots with names |
| `status channels page N` | Next pages of three slots, with names |
| `status channels N` | Inspect one slot's name and privacy eligibility |
| `status channel N` | Bind the destination's current slot, name and key |
| `status interval N` | Save interval in seconds, 0–86400; 0 disables the timer |
| `gps on` | Enable tracking and start acquiring a fix |
| `gps off` | Pause tracking and stop GPS; continue listening/responding |
| `gps status` | Tracking policy and actual acquiring/asleep state |
| `gps diag` | GPS UART bytes, NMEA checksums, satellites and fix diagnostics |
| `status show` | Selected channel, interval and configured GPS policy |
| `status send` | Request status and GPS acquisition without changing tracking policy |
| `verify` / `status verify` | Check bound private-channel configuration and coordinate validity |
| `tracker hardware` | Audio hardware, GPS detection and tracking policy diagnostics |
| `exit` | Leave CLI rescue and restore the USB companion protocol |

## Diagnostic scope

`verify` and `status verify` check the bound channel and the validity of a known
coordinate. A `READY` response does not test GPS hardware, radio delivery,
power consumption, or device compatibility. It is an optional developer diagnostic.

`exit` leaves the current rescue handler and resumes companion protocol handling.
Normal USB tracker commands continue to work afterward. Filesystem rescue commands
remain restricted to rescue mode.


## Verification scope

The connected Wio e-ink was used to verify USB terminal command responses, GPS
acquisition start/stop, settings restoration, and binary companion queries. App
CLI request/reply frames are exercised separately from the phone UI; a protocol
check is not a claim that every mobile app version has been manually tested.

Screenshot tooling now uses extension command/reply `0x7E`, matching newer Solo.
The old screenshot IDs 66/29 conflicted with the official local CLI protocol.
