# Tracking CLI implementation reference

This is a developer reference for `MyMesh::handleStatusCommand` in
[`examples/companion_radio/MyMesh.cpp`](../examples/companion_radio/MyMesh.cpp).
The commands below exist in the current `PRIVATE_STATUS_REPORTER` implementation.
They are not a tested, universal onboarding sequence for every board or firmware.

The terminal handler currently runs inside Solo CLI rescue mode. Serial framing,
entry controls and transport support must be checked for the selected build.
The generic user guide is [tracking.md](tracking.md); board constraints are in
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
There is no claim that every target exposes the same terminal entry procedure.
