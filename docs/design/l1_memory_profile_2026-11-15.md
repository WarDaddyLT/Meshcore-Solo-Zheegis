# L1 Pro memory profile — 2026-11-15

Measured directly over USB on the attached Wio Tracker L1 Pro. The installed
production source snapshot was instrumented temporarily with raw allocator
counters and target-side `sizeof` queries. Original production firmware was then
restored and application USB returned. No settings, identity or channel contents
were exported, reset or intentionally changed. No feature cuts were applied.
The unfinished multiplayer Checkers work remains separate in its temporary tree.

See [sanitized measurements](l1_memory_profile_2026-11-15.json).

## What actually consumes RAM

These are distinct portions of the application’s **235,520-byte RAM budget**:

| Allocation | Bytes | Lifetime / purpose |
| --- | ---: | --- |
| Static objects and buffers | 173,736 | Allocated regardless of which menu is visible |
| Allocated heap at home | 27,964 | Runtime libraries, RTOS allocations, display and home state |
| Free heap at home | 33,820 | Raw allocator free blocks plus memory not yet requested from the heap |

The following table expands parts of those totals; do not add the parent-object
rows to their member rows. Removing a menu does not remove these fixed arrays.

| Resident feature | Bytes | Evidence / consequence |
| --- | ---: | --- |
| Contacts plus sorting indexes | **67,304** | 350 ordinary slots plus 8 reserved; 184 bytes/contact and 4 bytes/index. **116 ordinary contacts loaded**. Unused capacity still occupies RAM. |
| Phone offline message queue | **45,312** | 256 frames × 177 bytes; currently empty, but all space remains reserved. Separate from screen chat history and game synchronization. |
| Solo GPS trail history | **8,248** | 512 points; embedded in `UITask`, even with the trail screen closed. Separate from Žygis’s private tracking scheduler. |
| Solo preferences | 2,824 | Shared settings; preserve the serialization schema. |
| Channel table | 2,600 | 40 slots; inexpensive compared with contacts and the offline queue. |
| Žygis tracker reply queue | 2,448 | 24 bounded replies; do not conflate this with the phone queue. |
| Žygis discovery table + tracker preferences | 776 | 704 + 72 bytes, excluding small scalar state. |
| Shared keyboard/editor | 976 | Resident in `UITask`; includes placeholder/editor buffers. |
| Connection table | 768 | 16 × 48 bytes. |

The complete `MyMesh` object occupies 127,416 bytes, including its contact table,
phone queue, preferences, channels and tracker members above. `UITask` occupies
11,640 bytes, including the trail store and keyboard. Other notable static
objects are the BLE/USB interface (5,264 bytes), filesystem cache (4,096), Bluefruit
object (1,940), duplicate tables (1,296), and timer task stack (1,024). These are
object sizes, not a complete attribution of every RTOS/BLE runtime allocation.

## Screen and game costs

Every screen below was opened over USB and measured, then closed. The reported
cost includes allocator overhead and any allocation performed while opening it.

| Screen | Additional heap bytes |
| --- | ---: |
| Settings | 1,620 |
| Messages | 18,492 |
| Tools | 304 |
| Nearby contacts | 3,696 |
| GPS trail | 648 |
| Clock | 52 |
| Bot | 56 |
| Compass | 16 |
| Ringtone editor | 476 |
| Dashboard editor | 28 |
| Automatic adverts | 24 |
| Live location sharing | 32 |
| Waypoint locator | 1,968 |
| Diagnostics | 1,172 |
| Repeater controls | 348 |

| Game screen | Additional heap bytes |
| --- | ---: |
| Tic-Tac-Toe CPU | 168 |
| Tic-Tac-Toe multiplayer menu | 1,136 |
| Snake | 980 |
| Checkers CPU | 660 |
| Battleship CPU | 612 |
| Scorched Earth CPU | 548 |
| Holdem CPU | 300 |

Games and optional screens release their screen-owned state when left. Local
chat costs 18,492 bytes only while Messages is open; deleting additional chat UI
features would not reclaim that amount at home because it is already unloaded.

Opening Diagnostics caused a one-time additional 72 bytes to remain allocated.
Afterward, **25 repeated visits stayed at exactly 33,748 free bytes**: no repeated
growth was observed. Attribution of that small allocation was not established.
The loop-stack watermark remained **2,064 bytes unused**. Total free heap is not
a guarantee that one equally large contiguous allocation will succeed. These
were screen-opening samples, not combined BLE/GPS/radio worst-case stress tests.

## Rotation and previous behavioral changes

The target’s live flags report OLED, 128×64, with both display-rotation and
joystick-rotation settings disabled. Both live preference values are zero.
[Seeed identifies the L1 Pro as the OLED model](https://wiki.seeedstudio.com/wio_tracker_l1_node/).
The separate L1 E-Ink target has a different display driver and feature flags.

Source inspection shows:

- `Features.h` enables rotation controls for e-ink, disables both for OLED.
- `DataStore.cpp` reads display and joystick preferences, then explicitly resets
  joystick rotation to zero when its feature flag is off. Display rotation is
  not reset by that branch; the OLED driver inherits a no-op rotation method.
- These flags and load rules were already present in the base commit, not
  introduced by the game memory changes. Rotation preferences use **two bytes**;
  disabling the controls is not a useful RAM-saving strategy.
- Settings edits normally save on Back/Cancel. Controlled shutdown saves all
  preferences, but a hardware reset/battery disconnect before saving can lose
  pending edits. The new screen collector also deletes inactive Settings without
  an explicit dirty-settings flush. This is a persistence risk to fix separately;
  it was found by source inspection, not reproduced by changing this device’s
  settings during this read-only measurement.

Other deliberate changes already made for games: local chat history/unread state
is discarded when Messages closes; UI-managed DM retries stop; stopwatch screen
state resets on unload. Existing background alarms/countdown, tracking and
companion transport remain. These behaviors deserve a family-use review, rather
than treating every Solo feature as equivalent memory cost.

## Recommended family profile, without further capability cuts yet

1. **Fix settings persistence and define intended OLED rotation behavior first.**
   Flush edits before unloading Settings and make the chosen rotation behavior
   survive restart. Test the real save/load path; do not use rotation flags as
   memory switches.
2. Reduce the **phone offline queue from 256 to 32 frames**: reclaim **39,648
   bytes**. This reduces how much old traffic is available to a phone reconnecting
   later; it does not change the live game protocol’s ACK/state tracking.
3. Reduce **ordinary contact capacity from 350 to 160**: reclaim **35,720 bytes**
   from contacts/indexes, while accommodating the 116 currently loaded contacts
   with 44 spare ordinary slots. Do not apply a smaller limit that silently drops
   current contacts. Preserve contacts/channels/preferences through any migration.
4. Together, those capacity changes would reclaim **75,368 bytes**, projecting
   about **109,188 free heap bytes at home** with otherwise unchanged allocation.
   This is an arithmetic estimate, not a measurement of a new family build.
5. If trail recording is unwanted, review its **8,248-byte** always-resident
   store next. Alternatively allocate it only while recording/viewing and handle
   saving explicitly. Keep private GPS tracking independent of this decision.
6. Bot/repeater-admin/ringtone/advanced location menus can be hidden for a simpler
   family interface, but most already cost only tens/hundreds of bytes while open
   and zero separate screen allocation when closed. Hiding them mainly reduces
   UI complexity or flash usage. Keep channels, identity, alerts, display/input
   settings, tracking, clock/calendar and games functional.

No reduced-capacity firmware was installed. The connected tracker was returned
to its original production image, without temporary profiling hooks or
post-flash checksum verification. The production matchmaking/rematch update and
unfinished Checkers multiplayer work were not mixed into this profiling run.
