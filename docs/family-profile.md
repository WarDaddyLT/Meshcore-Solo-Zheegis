# Žygis family tracker profile

This Wio Tracker L1 / L1 Pro OLED profile keeps private-channel tracking,
alerts, the full GPS trail recorder, waypoint/GPX tools, clock/calendar and all
six single-player games. Tic-Tac-Toe multiplayer includes automatic joining,
named hosts and Play again.

## Family behavior

- No general contact address book, contact sorting, direct chat or room chat.
- Eight temporary slots cache directly heard repeater adverts; distant
  repeater/client adverts do not populate a persistent address book.
- No local message history or phone offline backlog. Four bounded live handoff
  frames preserve the companion pull protocol; they clear on disconnection.
- The home Messages entry becomes **Channels**: private-channel add/edit,
  waypoint sharing, and game/channel quick-launch pins.
- Existing contact files and legacy contact pins remain available to a regular
  Solo build. Such pins display “Solo contact” here and may be replaced.
- Dirty Settings edits save before the screen is destroyed. Existing rotation
  controls save immediately on change. This repairs the save path; it does not
  add portrait rotation to the fixed-orientation OLED driver.

## GPS trails are retained

The **512-point, 8,248-byte trail store remains resident**, including background
sampling while a game is visible. Leaving Trail does not destroy that store.
Trail, waypoint and GPX code is unchanged. Private Žygis tracking is independent
of Solo trail recording; recorded routes are not chat history.

## Use the tracker

Configure private channels through the home **Channels** entry or the companion
app. Family tools use the same private-channel predicate as tracking/Games;
game selection gains no new binding, metadata or confirmation requirement.
Use **Settings → Tracking** for the normal tracker setup; see the
[tracking guide](tracking.md). Keys, identity, radio settings and the `/zygis1`
record retain their existing formats.

Open **Tools → Trail** for routes and **Tools → Games** for games. On Favourites,
choose **+ → Game** or **+ → Channel**. A channel pin opens channel settings;
a game pin launches its game. The companion still configures the tracker and
exchanges live private-channel text. Direct messages, rooms and retrospective
phone message sync are unavailable. See [Games](games.md) for controls and
multiplayer compatibility; both players need the updated protocol.

## On-device memory results

Direct USB counters and target-side object sizes from a temporary instrumented
copy of the family source:

| Resource | Previous production | Family |
| --- | ---: | ---: |
| Static RAM | 173,736 B | 63,280 B |
| Home free heap | 33,820 B | **144,276 B** |
| Contact/index storage | 67,304 B | 1,472 B; no sort array |
| Phone backlog/handoff storage | 45,312 B | 708 B, live only |
| GPS trail store | 8,248 B / 512 points | unchanged |
| Channel/message screen while open | 18,492 B | 356 B |

Gain: **110,456 bytes (~108 KiB)**. Home returned to **144,276 free bytes** after
every tested screen/game exit. Minimum unused loop stack was 2,064 bytes. The
previous contact file is still present (18,544 bytes); family contact load/save
paths leave it untouched. These samples do not guarantee worst-case combined
radio/BLE/GPS headroom. See the [sanitized results](design/l1_family_memory_2026-11-15.json).

## Build and validation

```sh
pio run -e WioTrackerL1_zygis_family
python3 test/zygis/test_channel_only.py
python3 test/zygis/test_family_channels.py
python3 test/zygis/test_settings_lifetime.py
```

Output: `.pio/build/WioTrackerL1_zygis_family/firmware.uf2` or `firmware.zip`
(serial DFU). It identifies itself as `zygis-family`. The ordinary Solo and
previous games-production environments remain available. No partition or
preference-schema migration is added.

Production omits USB game input, raw memory/game probes, timing counters and
screenshots. Temporary measurement code is separate. Compilation, 69 native
cases, family storage/channel/settings checks, tracking regressions, game
simulations and ordinary Solo/simulator builds passed. The connected tracker
runs the debug-free family production image. Its stale clock was synchronized
to UTC and retained after a controlled reboot. Real two-radio matches, phone
app operation under traffic, physical controls and outdoor GNSS recording still
need field validation.

The [Veylith Arena GDD](design/Veylith_Arena_GDD_v1.0.md) is alongside the other
design documents; Arena remains unimplemented and disabled. Its spectator-history
proposal needs a later explicit decision: this profile does not recreate chat
history for spectators. Multiplayer Checkers remains separate unfinished work.
