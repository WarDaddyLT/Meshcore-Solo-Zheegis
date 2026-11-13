# Private tracking and channel commands

Žygis adds private position reports, remote tracking controls, and device alerts
to the Solo messaging and navigation foundation. This guide describes the shared
tracking behavior. Available controls and feedback depend on the build and the
hardware it runs on.

## Before using tracking

1. Choose a build configuration with Žygis tracking enabled. The current feature
   flag is `PRIVATE_STATUS_REPORTER`; inherited Solo configurations do not all
   enable it automatically. See the [hardware integration reference](hardware.md).
2. Install the firmware using your board's flashing instructions and match your
   group's radio preset.
3. Create or import a private channel with a newly generated secret in a
   compatible companion app.
4. Open **Settings → Tracking**, or use the companion app's local CLI, to select
   that channel, set a reporting interval, and enable tracking. See the
   [CLI reference](tracking-cli.md) for commands and transport details.
5. Send `@[Tracker01] !kur` in the selected channel, substituting the device's
   exact configured name, to request a report.

Channel binding and reporting settings persist across reboot. The tracker binds
both the selected channel name and key; if either changes, select the channel
again. Tracker builds suppress automatic public location adverts, remote location
telemetry, and Solo automatic live sharing. Explicit manual shares remain available.

## GPS cycle

At each scheduled interval, enable GPS and acquire a fresh fix. Send the fresh
position and battery percentage, then disable GPS until the next interval. The
radio remains available, along with the companion interfaces supported by the build. The next interval is measured from completion
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
estimate based on voltage limits, not a measured charge counter.

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
fullscreen until a supported physical control dismisses it. Short
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

## Channel access and delivery

Channel checks reject the standard Public key, empty keys, hashtag channels, and
keys derived from the current name. These checks cannot prove that a key is random
or detect every previously name-derived key. Use a newly generated private secret.

Everyone with that secret can send tracker commands. Shared-key messages do not
authenticate individual senders or discovery responders. A locally queued report
is not confirmation that every channel member received it.

## Controls and feedback

An available speaker or buzzer provides message chirps and alarms. A display can
show persistent messages; physical controls dismiss them. LED colors, keypad
handling, audio drivers and GPS power circuitry belong to each board's adapter.
See [hardware integration](hardware.md) for the implementations currently present.

## Persistence and Solo integration

Tracking stores its binding, interval and position-known flag in a separate
versioned `/zygis1` record using atomic replacement. Coordinates and tracking
enablement remain in Solo preferences; the `NodePrefs` binary schema is unchanged.

The tracking scheduler controls GPS acquisition and sleep without an acquisition
timeout. Solo's legacy GPS toggles and duty scheduler cannot override it in tracking
builds; use Settings → Tracking or the tracking CLI. Navigation and trail recording use fixes available during acquisition.
Generic channel notifications and channel-bot dispatch are replaced by tracker
routing, while channel message history remains available.

## Development checks

```sh
python3 test/tracking/test_status.py
python3 test/tracking/test_cycle.py
python3 test/tracking/test_dispatch.py
python3 test/tracking/test_persistence.py
```

Build the relevant target with `pio run -e YOUR_ENVIRONMENT`, replacing the
placeholder with its actual environment name. Host checks and builds do not
establish physical-device behavior; record device results separately.
