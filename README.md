# MeshCore Solo Žygis

[English](README.md) · [Lietuviškai](README.lt.md)

[![Žygis checks](https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis/actions/workflows/run-unit-tests.yml/badge.svg?branch=main)](https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis/actions/workflows/run-unit-tests.yml)

**Community preview** · Built for exploration with the MeshCore community.

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

1. **Choose your hardware and build target.** Check the [hardware support notes](#hardware-support)
   and the [board configurations](solo/) for your device.
2. **Build and flash the firmware.** Follow [building and installation](#build-and-get-started)
   and the instructions for your board.
3. **Join your mesh.** Match the radio settings used by your group and create or
   import a private channel through a compatible MeshCore companion app.
4. **Configure tracking for your build.** Follow the [tracker setup guide](docs/tracking.md)
   for the target you are using. Configuration controls depend on the build.
5. **Use the device.** Send messages, save waypoints, or try the
   [tracking commands](#request-locations-and-send-alerts) in your configured channel.

This preview has passing software checks and firmware builds. Physical-device
validation is still pending. See the [roadmap](ROADMAP.md) and [changelog](CHANGELOG.md).

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

## Hardware support

The project aims to bring navigation, messaging, and private tracking across
hardware supported by MeshCore Solo. Feature availability depends on each board's
GPS, display, controls, audio hardware, and build configuration.

Existing Solo board configurations are retained. Žygis tracking coverage is being
extended across targets; an inherited board configuration does not mean every
tracking feature is enabled or tested on that board. Check the
[Solo hardware reference](README-Solo.md#supported-devices), [Solo build targets](solo/),
and [current tracker integration notes](docs/tracking.md) for the
configuration you intend to use.

## Build and get started

Install PlatformIO and clone the repository:

```sh
git clone https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis.git
cd Meshcore-Solo-Zheegis
pio project config
```

Choose the environment matching your device and required features, then build it
with `pio run -e YOUR_ENVIRONMENT`, replacing `YOUR_ENVIRONMENT` with that
configuration's actual name. Target definitions live in `solo/` and `variants/`.

Follow the [flashing instructions](README-Solo.md#flashing) for your board. Review
any target-specific partition or storage migration requirements before flashing.

After installation, match your group's radio settings and create or import a
private channel in a compatible companion app. Use the target's documented
configuration method to select the tracking channel and reporting interval.
The [tracker setup guide](docs/tracking.md) describes the current
implementation, diagnostics, alerts, and discovery. Device setup still needs
physical validation; build and host-test results do not establish it.

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
python3 test/tracking/test_status.py
python3 test/tracking/test_cycle.py
python3 test/tracking/test_dispatch.py
python3 test/tracking/test_persistence.py
```

- [Tracking guide and channel commands](docs/tracking.md)
- [Hardware integration reference](docs/hardware.md) · [developer CLI reference](docs/tracking-cli.md)
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
