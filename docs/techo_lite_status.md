# Žygis private tracking and channel commands

Targets: `LilyGo_T-Echo-Lite_keyshield_zygis` and
`ThinkNode_M5_zygis`. The T-Echo Lite target includes its kit's
ES8311 speaker and TCA8418 keyboard support. The M5 uses its built-in buzzer. Other targets retain
their original behavior.

## Setup and CLI

Create/import a private channel with a random secret in MeshCore, pair over
Bluetooth, and open the USB terminal at 115200 baud (CR or LF). Hold the physical
user button within eight seconds of startup to enter Solo CLI rescue mode.
Tracking continues in rescue mode. Use `exit` after configuration to restore
normal companion app traffic. The default BLE PIN is 123456 unless previously
changed. Configure the same radio preset as your mesh.

```text
help
status channels
status channels page 1
status channels 1
status channel 1
status interval 300
gps on
verify
```

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

Settings persist across reboot. Screen/app GPS toggles cannot override tracking
policy. Public location adverts and location telemetry are disabled for these
targets, so tracking coordinates are sent through the configured private channel.

## GPS cycle

At each scheduled interval, enable GPS and acquire a fresh fix. Send the fresh
position and battery percentage, then disable GPS until the next interval. The
radio and Bluetooth stay available. The next interval is measured from completion
of the previous acquisition. A new fix is required after every GPS wake; old UART
bytes are drained before acquisition.

There is **no GPS acquisition timeout**. If no fix is available, send a status
explicitly saying `GPS:no fix; last known`, with a clickable map URL for the
cached/manual coordinates. If no position has ever been supplied, say
`GPS:no fix; no last known position`. Keep acquiring until a fresh fix is obtained
or tracking is explicitly stopped. Each later interval can report continued
acquisition, without restarting GPS. Queue pressure does not discard acquisition.

Manually set coordinates in the MeshCore app are accepted, including (0,0).
GPS fixes replace them when acquired. Manual settings persist immediately;
GPS coordinates are saved with preferences and on accepted status reports at
most once per five minutes, with the first known position saved immediately.
A power loss can restore an older saved position. Battery percentage is a voltage
estimate using the same limits as the display, not a measured charge counter.

Example report:

```text
Bat:81% GPS:no fix; last known https://maps.google.com/?q=54.687200,25.279700
```

Map links may open a browser/external map app; native MeshCore map integration
has not been verified. Reports are queued on the local radio, not acknowledged
by every channel recipient. Radio scheduling and airtime limits still apply.

## Channel commands and targeting

Only messages decrypted in the selected, unchanged private channel control the
tracker. The handle is the device's configured MeshCore name, matched exactly.

```text
@[Tracker01] !kur
@[Tracker01] !interval 300
@[Tracker01] !tikra Come home
@[Tracker01] Hello, please call me
```

A prefix targets one device; other devices neither act nor reply. Without a
prefix, commands apply to all receiving devices. Unaddressed conversation text
does not trigger alerts. For an addressed body beginning with `!`, there are
**no conversation chirps**, including for unknown commands.

| Chat command | Behavior |
| --- | --- |
| `!kur` | ACK with incoming route hashes, start GPS if needed, report no-fix/last-known status, then fresh status when acquired |
| `!power down` | Stop GPS and periodic tracking; remain listening/responding |
| `!power up` | Resume tracking and immediately start GPS acquisition |
| `!power` | Report power-saving/tracking state without changing it |
| `!interval N` | Persist the same 0–86400-second reporting interval as CLI; 0 disables periodic requests |
| `!interval` | Report current interval |
| `!tikra [text]` | ACK with route; repeating audible alarm and optional fullscreen text until a physical button dismisses it |
| `!triangulate` | ACK with route; discover repeaters reachable directly and report results |
| `!traingulate` | Accepted spelling alias for `!triangulate` |
| `!help` | One compact help message |

An explicit `!kur` can perform a one-shot acquisition while periodic tracking is
paused. `!power down` cancels the current acquisition. Changing the interval to
zero stops future scheduled requests; it does not cancel a request already
waiting for a fix.

A matching addressed conversation message produces three gentle chirps and stays
fullscreen until a physical button or any kit keyboard key dismisses it. Short
messages use the largest fitting font, centered in the available screen area;
long messages wrap at a smaller font. Commands and messages for other
handles stay silent. An active alarm takes precedence over conversation messages.
An unknown addressed/broadcast command is ACKed by command token and incoming
route, without conversation chirps. Commands with their own reply do not add an
extra generic ACK. `!kur`, `!tikra` and discovery include explicit route ACKs.
Route data consists of the ordered on-air repeater hashes, not authenticated
full repeater names; long routes are split into fitting messages.

## Direct repeater discovery

`!triangulate` transmits a zero-hop discovery request filtered to repeaters. Its
15-second collection window starts **after actual transmission**, rather than
while waiting for airtime. Only direct, zero-hop repeater replies with the
matching request tag are accepted. Repeated replies from the same key prefix
are deduplicated. Reports include a known contact name (or `unknown`), an 8-byte
public-key prefix, receive RSSI, and SNR in both directions. The final message
states how many replies were received. Up to 64 distinct responders are retained;
if the limit is reached, the summary explicitly reports the omission.

Hidden nodes, nodes suppressing discovery, and lost replies are not observable.
This command provides direct-neighbor observations, not a calculated triangulated
coordinate. Results go to the private channel; the zero-hop discovery probe is
MeshCore's normal control packet and contains no tracking coordinates.

## Single-packet help

The following ASCII body is at most 100 bytes. Including the maximum 31-byte
sender name and `: `, the complete text remains within 133 bytes. No explanatory
help packets are emitted.

```text
@[Handle] or all
!kur
!tikra [text]
!power up
!power down
!power
!interval <sec>
!triangulate
!help
```

## T-Echo Lite LEDs and audio

Blue blinks for five seconds after a position-bearing status packet finishes
transmitting. Queueing/failed TX and unrelated radio traffic do not trigger it.
Green blinks for five seconds when GPS acquires/reacquires a fix, including after
a 15-second gap. Routine fix updates do not extend a burst. Both use 250ms on/off
without blocking. Normal Bluetooth and generic TX blinking are disabled.

The kit speaker uses the ES8311 codec and I2S pin mapping from
[LilyGo's kit example](https://github.com/Xinyuan-LilyGO/T-Echo-Lite/blob/main/examples/T-Echo-Lite-KeyShield/es8311/es8311.ino).
Solo's existing `TEchoKeyShield.cpp` owns keypad initialization and events; there
is no second keyboard driver consuming its FIFO. The pinned `cpp_bus_driver`
library builds only the selected codec and nRF52 bus support;
`scripts/techo_audio_build.py` excludes unrelated device drivers. Audio buffers
are serviced without waiting in the main loop. The DAC is powered off when idle.
Alarm dismissal consumes the button press so it does not also navigate/power off.

## Build, flash, checks

```sh
pio run -e LilyGo_T-Echo-Lite_keyshield_zygis
pio run -e LilyGo_T-Echo-Lite_keyshield_zygis -t upload --upload-port /dev/cu.usbmodem1101
python3 test/techo_status/test_status.py
python3 test/techo_status/test_cycle.py
python3 test/techo_status/test_dispatch.py
python3 test/techo_status/test_persistence.py
```

Use `ThinkNode_M5_zygis` and its port for the M5. Keep its physical
GPS switch enabled. A physical power cut cannot be overridden by software.

If a Mac sees a WCH USB device (`1a86:7522`) but no serial port, install the current
DMG driver app from [WCH](https://github.com/WCHSoftGroup/ch34xser_macos), click
Install, approve the driver extension, and reconnect. See also
[Elecrow's M5 guidance](https://forum.elecrow.com/discussion/28300/thinknode-m5-updating-firmware).

Privacy verification rejects Public's key, zero keys, hashtag names, and keys
derived from the current name or `#` plus that name, and binds the selected name
and key. It cannot prove a key is random/secret or detect derivation from an
unrelated previous name. Select a newly generated private key and share it only
with intended channel members. Shared-key channel messages do not authenticate
individual senders or discovery responders.


## Solo integration

Tracking builds store their binding, interval and position-known flag in the
versioned `/zygis1` record using atomic temp-file replacement. Coordinates and
tracking enablement remain in Solo's preferences. Solo's `NodePrefs` binary
schema is unchanged. Tracking uses its own indefinite acquisition scheduler;
Solo's GPS duty scheduler and GPS toggles cannot override it. Navigation/trail
features use the fixes available during acquisition. Solo automatic live
sharing is suppressed on tracking targets; explicit manual shares remain
available. Generic channel notifications and channel-bot dispatch are replaced
by tracker routing, while channel message history remains available.

The M5 tracking environment uses `min_spiffs.csv` to fit the Solo UI and tracker
inside a 4 MiB device with OTA support. Back up data and use a full USB flash
when switching partition layouts. Physical GPS, speaker/buzzer, keypad, LEDs,
and power consumption still require validation on each board.

The pinned ES8311 dependency declares AGPL-3.0 licensing. Preserve its notices
and account for its terms when distributing builds; the upstream MeshCore MIT
notice alone does not describe every firmware dependency.
