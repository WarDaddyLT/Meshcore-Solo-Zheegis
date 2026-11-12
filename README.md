# MeshCore Solo Žygis

[English](README.md) · [Lietuviškai](README.lt.md)

[![Žygis checks](https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis/actions/workflows/run-unit-tests.yml/badge.svg?branch=main)](https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis/actions/workflows/run-unit-tests.yml)

**Community preview** · T-Echo Lite + KeyShield and ThinkNode M5 · Built for exploration with the Lithuanian MeshCore community.

**Offline GPS navigation, private location tracking, and messaging over LoRa mesh.**

MeshCore Solo Žygis turns a supported LoRa device into a standalone communicator
and tracker. Use its screen and controls to exchange messages, save waypoints,
follow a GPS trail, and navigate outdoors. Share position and battery reports
with your group through a selected private MeshCore channel, and request a
tracker's location or trigger an audible alert remotely.

LoRa communication works without mobile coverage or an internet connection.
Opening a report's Google Maps link may require internet access on the receiving
phone. Range and delivery depend on the radios, terrain, mesh coverage, and airtime.

Built on [MeshCore Solo](https://github.com/MarekZegare4/MeshCore-Solo) and
[MeshCore](https://github.com/meshcore-dev/MeshCore).

## Start here

- **Explore:** [Lithuanian overview](README.lt.md) and [tracking examples](#request-locations-and-send-alerts).
- **Try a build:** [supported tracking hardware](#tracking-hardware) and [setup](#build-and-get-started).
- **Join in:** [ask a question or share feedback](https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis/discussions), [report a bug](https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis/issues/new/choose), or [contribute](CONTRIBUTING.md).

This preview has passing software checks and local firmware builds. Device
validation is still pending; it is not presented as a field-tested release.
See the [project roadmap](ROADMAP.md) and [changelog](CHANGELOG.md).

## What you can do

### Navigate and communicate

The Solo foundation provides an on-device messaging interface, saved waypoints,
bearing and distance navigation, a GPS trail view, and GPX export. Its GPS compass
uses movement-derived heading, so it needs movement to establish direction.
A compatible companion app can also connect to the device.

Žygis tracking builds acquire GPS fixes on demand or at the reporting interval.
Navigation and trail recording use fixes available during acquisition; they do
not keep GPS running continuously while the tracker is sleeping.

### Share private tracking reports

Choose a private channel and reporting interval. Each cycle wakes GPS, waits for
a fresh fix, queues a position and battery report, then puts GPS to sleep. The
radio remains available for messages and commands.

Reports distinguish a fresh fix from a last-known position or an unknown
position. For example:

```text
Bat:81% GPS:fix https://maps.google.com/?q=54.687200,25.279700
```

Without a fix, GPS keeps acquiring until it succeeds or tracking is stopped.
There is no acquisition timeout, so poor reception can keep GPS powered longer.
Settings and the selected channel binding persist across reboots.

### Request locations and send alerts

Send commands through the configured private channel. Prefix a message with the
device's exact MeshCore name to address one tracker:

```text
@[Tracker01] !kur
@[Tracker01] !interval 300
@[Tracker01] !tikra Please return to camp
@[Tracker01] Please check your radio
```

| Message | Action |
| --- | --- |
| `!kur` | Request location; acquire a fresh GPS fix if needed |
| `!power down` | Stop GPS and periodic tracking; continue listening |
| `!power up` | Resume tracking and start acquisition |
| `!power` | Report the tracking state |
| `!interval 300` | Set the reporting interval in seconds; `0` disables the timer |
| `!interval` | Report the current interval |
| `!tikra [text]` | Start a repeating alarm with optional fullscreen text |
| `!triangulate` | Discover direct repeaters and report signal observations |
| `!help` | Show compact command help |

Omit `@[Tracker01]` to send a command to all receiving trackers on that channel.
An addressed conversation message produces three gentle chirps and a fullscreen
message. Physical controls dismiss messages and alarms. Ordinary unaddressed
conversation does not trigger tracker alerts.

Discovery reports directly reachable repeaters, signal strength, and signal-to-noise
ratios. Route acknowledgements show incoming repeater hashes. Despite the command
name, `!triangulate` does not calculate a geographic position.

## Tracking hardware

| Device | Build environment | Device feedback |
| --- | --- | --- |
| LilyGO T-Echo Lite + KeyShield | `LilyGo_T-Echo-Lite_keyshield_zygis` | Kit speaker, keypad, GPS-fix and position-transmit LEDs |
| ThinkNode M5 | `ThinkNode_M5_zygis` | Built-in buzzer; physical GPS switch must stay enabled |

The T-Echo Lite blinks green after a GPS fix and blue after a position-bearing
report finishes transmitting. The original Solo environments remain available;
Žygis tracking is enabled by the two environments above.

Firmware builds and host checks pass. Physical GPS, sound, keypad, LEDs,
persistence, and power consumption still require validation on the devices.

## Build and get started

Install PlatformIO, clone this repository, and build for your device:

```sh
git clone https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis.git
cd Meshcore-Solo-Zheegis
pio run -e LilyGo_T-Echo-Lite_keyshield_zygis
# Or:
pio run -e ThinkNode_M5_zygis
```

For upload instructions, see the [Solo hardware and flashing reference](README-Solo.md#flashing).
The M5 tracking target changes the partition layout to fit the firmware: back up
device data and perform a full USB flash for the first migration. An
application-only OTA update does not install the new partition table.

Create or import a private channel with a newly generated secret in your
companion app. Match the radio settings to your mesh. Open the USB terminal at
115200 baud and hold the user button within eight seconds of startup to enter
CLI rescue mode:

```text
status channels
status channel 1
status interval 300
gps on
verify
exit
```

Replace `1` with your private channel slot. `verify` checks the binding; `exit`
restores normal companion app traffic. Use `gps off` to pause tracking or
`status send` for a manual report. The [complete tracker guide](docs/techo_lite_status.md)
covers diagnostics, interval behavior, alerts, and discovery.

## Privacy and delivery

Tracking commands and reports use the selected private channel, bound to its
name and key. Public location adverts, remote location telemetry, and Solo's
automatic live sharing are blocked on tracking builds. You can still explicitly
share a waypoint or location yourself.

Everyone with the channel secret can send tracker commands. Shared-key channel
messages do not authenticate individual senders. Reports are queued locally;
queueing does not guarantee receipt by every member of the group.

## Development and documentation

```sh
pio test -e native -e native_kiss_modem -vv
python3 test/techo_status/test_status.py
python3 test/techo_status/test_cycle.py
python3 test/techo_status/test_dispatch.py
python3 test/techo_status/test_persistence.py
```

- [Tracker setup and command reference](docs/techo_lite_status.md)
- [Solo navigation and tools](docs/solo_features/tools_screen/tools_screen.md)
- [Solo messaging interface](docs/solo_features/message_screen/message_screen.md)
- [Solo hardware, flashing, and feature reference](README-Solo.md)
- [Migration notes](docs/zygis_migration.md)
- [Contributing](CONTRIBUTING.md) · [repository guide](AGENTS.md)
- [Project roadmap](ROADMAP.md) · [changelog](CHANGELOG.md)
- [Upstream maintenance](docs/upstream-maintenance.md)

## Credits and licensing

Žygis builds on the work of the MeshCore and MeshCore Solo contributors.
See [the upstream MIT license](license.txt) and retain its notices.
Dependencies have their own licenses; the pinned ES8311 driver dependency
`cpp_bus_driver` declares AGPL-3.0. Review its terms when distributing builds
that include it.
