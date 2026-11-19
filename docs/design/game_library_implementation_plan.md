# Game Library Implementation Plan — Wio Tracker L1 Pro OLED

Date: 2026-11-14. Based on the [v1.1 design specification](MeshCore_Solo_Game_Library_GDD_v1.1.md), local source inspection, a successful target build, and direct USB measurements. This plan records the original baseline and phased design. The first six offline games, the CPU/multiplayer Tic-Tac-Toe preview and on-demand UI lifecycle are now implemented; see [Games](../games.md) for current behavior and limitations.

## Current scope adjustment

The channel-only family profile now reclaims 110,456 bytes on the L1 Pro while
retaining its 512-point trail store; see [family measurements](../family-profile.md).
[Veylith Arena v1.0](Veylith_Arena_GDD_v1.0.md) supersedes the older Creature
Arena section. The separate `WioTrackerL1_zygis_veylith` CPU candidate now adds
Trainer/progression and offline battles; see [Arena](../veylith-arena.md). The
base six-game/family environments do not enable it. PvP and spectators remain pending.

The latest lobby direction replaces manual acceptance/readiness/start with
automatic one-second announcements and negotiation. The list displays host
handles, and either player can request a fresh round from the result screen.

The later request to implement six **single-player** games one at a time
supersedes the original multiplayer/history prerequisite gates below. Tic-Tac-Toe,
Snake, Checkers, Battleship and Scorched Earth each passed host checks and
connected-device play/lifecycle checks before the next game began. Texas Hold’em
is local heads-up CPU play; its group-channel encryption blocker applies only
to a future multiplayer version. The older Creature Arena outline is superseded
by the Veylith CPU increment. The numbered
phases below retain the original design record, not the current execution order.

Requester rules override for Checkers: men capture in all diagonal directions;
kings slide/capture along the full diagonal; promotion during a capture is
immediate and the new king continues capturing. The original English rules in
v1.1 are superseded by this explicit direction.

The user prioritizes RAM for the visible screen/game and the always-running Žygis
tracking engine. Local chat histories are disposable, not a persistence target.
The isolated persistent-history experiment is excluded: its measured atomic
write blocked the loop for about 200 ms. No background game discovery or
invitations are retained after Games closes, and spectator replay is unavailable.

The first device cleanup reading was 33,860 bytes free heap versus 3,868 before
cleanup. CPU play, menus and configuration preservation were checked over USB;
multiplayer has two-peer host simulation coverage. A two-radio match remains a
physical validation requirement. The resource table below is the older baseline,
not the final games-build runtime budget.

## 1. Repository and target

The specification is currently stored in the `MeshCore` checkout at commit `66e7ff37ced46b40c8243a01606817e35a157401`. That checkout lacks the Solo UI needed by this design. Implement games in **`WarDaddyLT/Meshcore-Solo-Zheegis`**, whose local HEAD is `3e38b85eff8005f7c9956e6d556739e45ec1ae95`, with existing uncommitted L1 OLED target and tracking persistence fixes. Source paths below are relative to that Solo working tree, not this documentation checkout. Preserve those changes and record the complete starting diff before implementation.

Use `WioTrackerL1_zygis`, extending `WioTrackerL1_companion_solo_dual`. Preserve Žygis tracking alongside all ordinary Solo functions. Keep `WioTrackerL1_companion_solo_dual` as a compatibility build. Bring this specification and plan into the implementation repository before opening implementation PRs.

## 2. Verified hardware and resource budget

| Resource | Evidence and implementation consequence |
| --- | --- |
| MCU | nRF52840, 64 MHz Cortex-M4F, nominal 1 MiB flash/256 KiB RAM. Use bounded deterministic C++; external flash does not expand RAM. |
| Display | L1 OLED driver is `SH1106Display`, 128×64 monochrome with a 1,024-byte framebuffer. Reuse its buffer and `DisplayDriver`; no second framebuffer. |
| Input | Variant defines four joystick directions, joystick press, and menu/back. Reuse normalized `KEY_UP/DOWN/LEFT/RIGHT`, `KEY_ENTER`, and `KEY_CANCEL`, including rotation. Physical control testing remains a release gate. |
| Audio | Existing buzzer on Arduino pin 12. Reuse `buzzer` and Solo mute/volume policies; no new audio stack. Physical sound has not been tested in this audit. |
| Radio/GPS | Existing SX1262 and L76K drivers. The connected device answers companion protocol 14. Retain BLE/USB, radio scheduling, and GPS tracking; do not reconfigure them from Games. |
| Application flash | Baseline **501,588 / 708,608 bytes**, leaving **207,020 bytes** in the application slot. The nominal 1 MiB includes reserved regions. |
| Static RAM | PlatformIO reports **173,684 / 235,520 bytes**. This is not a runtime-free-memory measurement. |
| Heap region | Linker symbols reserve **61,836 bytes** for dynamic allocations, including allocated task stacks and UI objects. |
| Live heap | Four direct USB samples after diagnostic reboot: **57,728 bytes used**, **0 bytes on allocator free lists**, **4,108 bytes unclaimed**. Thus only **4,108 bytes** were available in that state. |
| Loop stack | Core config allocates **4,096 bytes**. Direct watermark: **516 words × 4 = 2,064 bytes minimum unused since boot**. Earlier UI report was 1,656 bytes at a different uptime. Neither is a worst-case guarantee. |
| Persistent storage | USB reports **2,048 KiB total, 60 KiB used**, leaving approximately **1,988 KiB** in the secondary filesystem at measurement time. It already stores contacts/channels and related data. |

The temporary diagnostic adds 192 bytes of application flash and no additional static RAM. Its update rebooted the device, so the slightly higher readings do not demonstrate an optimization. Configuration fingerprints matched before and after. Counters were read using binary USB extension `0x7D`, not screen capture. See the [sanitized audit data](game_library_hardware_audit.json).

The [Seeed hardware specification](https://wiki.seeedstudio.com/wio_tracker_l1_node/) corroborates the MCU, OLED, radio and GPS specifications. Board definitions indicate a P25Q16H-class QSPI device; the exact chip JEDEC identity was not queried. Do not infer free storage from the part number alone.

## 3. Source integration points and blockers

| Concern | Actual source and required change |
| --- | --- |
| Screen lifecycle | `ui-new/UITask.cpp`, `UITask.h`, and `src/helpers/ui/UIScreen.h`: screens implement `render()`, `handleInput()`, optional `poll()` and `onShow()`. Register a game screen through the existing lifecycle. |
| Entry point | `ui-new/ToolsScreen.h`: add **Tools → Games** and a dedicated Games screen. A home shortcut can follow after the first playable release. |
| Rendering | `src/helpers/ui/SH1106Display.*`: framebuffer reuse and unchanged-frame flush suppression already exist. Default OLED font is 6×9, not the GDD's suggested 5×7. Design layouts against the real font. |
| UI scheduling | `UITask.cpp` uses each screen's returned refresh delay. Keep simulation separate from rendering; process bounded work per poll. Measure full OLED I²C flush time before choosing an animation rate. |
| Incoming text | `MyMesh::onChannelMessageRecv()` receives decrypted channel text and appends it through `_ui->addChannelMsg()`. Parse recognized game events at that boundary without consuming ordinary chat or tracker commands. |
| Outgoing text | `BaseChatMesh::sendGroupMessage()` plus `MyMesh::mirrorOwnChannelMsg()`: reuse scoped sends, visible history and delivery indication. A repeater echo is not a player's acknowledgement. |
| History | `ui-new/MessageHistory.h` has **48 shared channel entries and 32 DM entries, all RAM-only**. Ring eviction and reboot destroy replay evidence. `MessagesScreen` owns the store. Persistence is required work, not an existing feature to assume. |
| Channel eligibility | `ChannelDetails` contains a secret and name, with no provenance/type attribute. `ChannelsView` supports hashtag, passphrase and raw-key entry. Existing tracking rejection heuristics do not prove a secret was randomly generated. |
| Settings | `NodePrefs.h`/`DataStore.cpp` use explicit, versioned binary serialization. Preserve existing layouts. Follow the independent `/zygis1` record pattern for new game preferences and metadata. |

Current user direction supersedes the original provenance/history gates: existing private channels are eligible without additional metadata, and persistent chat history is outside this build.

## 4. Phase A1 — recover and validate memory

This is the first engineering milestone. The GDD's proposed extra 4–8 KiB cannot be allocated safely on top of the measured 4 KiB free heap.

1. Record raw counters at boot, after browsing screens, during GPS acquisition, during BLE connection/reconnection, and under chat traffic. Include the largest reusable allocation where the allocator can report it without allocating test blocks. Retain stack minima across a sustained run, rather than rebooting between every sample.
2. Measure `sizeof` and ownership of eagerly created screens, keyboard/editor scratch, packet pool and persistent models. `UITask::begin()` currently allocates screens at startup. Share transient scratch or use a fixed arena initialized at startup; do not introduce heap allocation on every game/menu entry. Keep asynchronously updated models alive when their screens are hidden.
3. The two history arrays alone occupy **16,064 bytes**: channel entries are 196 bytes each, DMs 208. Keep existing capacities and behavior. First assess a filesystem-backed channel-history service with a small RAM text cache and bounded index; this may reclaim several KiB. DM history must retain current semantics unless a separate, explicit persistence decision is made. Do not assume all 16 KiB can be reclaimed.
4. Preserve contact limits, offline queue capacity, BLE, GPS, chat and alarms. Reducing these to fit games requires a separate product decision, not an implicit optimization.
5. Proposed engineering gates: at least **8 KiB idle free heap before the offline slice**, and **16 KiB before multiplayer**; retain **8 KiB under combined multiplayer/BLE/GPS/message load** and **1,536 bytes loop-stack minimum**. These are acceptance targets to validate with stress measurements, not measured present capacity. Stop and revise if they cannot be met.

Deliver allocation accounting, before/after resource figures and a messaging regression result before allocating a general game framework.

## 5. Phase B — polished offline Tic-Tac-Toe

Proposed new files, not existing interfaces: pure rules in `src/games/TicTacToe.*`; a small controller/registry in `src/games/`; UI in `examples/companion_radio/ui-new/GamesScreen.h`.

- Add `SOLO_GAMES_ENABLED` and a Tic-Tac-Toe flag in a separate opt-in environment extending `WioTrackerL1_zygis`. Do not enable unfinished games on ordinary targets.
- Use a fixed game-state structure; initial target **≤512 bytes** including bounded AI search state, with no gameplay allocation and no radio access from offline code.
- Implement X/O selection and Easy/Normal/Hard CPU. Use iterative or explicitly bounded search, advancing within a small per-loop work budget; avoid deep recursion on the 4 KiB loop stack.
- Fit a 48×48 board with 16-pixel cells and a readable status column. Joystick selects, press places, menu/back opens pause/exit. Add short nonblocking result effects and respect sound settings.
- Exhaustively test reachable boards, legal turns, all wins/draws, and unbeatable Hard play. Verify zero game transmissions, 100 menu-entry cycles, and messaging/alarms/GPS responsiveness.

Exit gate: a complete offline game on the physical OLED, measured flash/RAM deltas, and memory margins retained. Do not implement Snake yet.

## 6. Phase C foundations — privacy and shared history

### Private channel eligibility

Use the same private-channel predicate as tracking. List every configured private
channel, with no extra eligibility flag, origin record, tracking binding or
confirmation. The user chooses a channel; do not auto-select it. Public and
hashtag channels remain excluded. This replaces the original origin-metadata
proposal at the user's explicit request.

### Durable visible channel history — deferred original proposal

The following proposal is not enabled in the current build; the user prioritizes
disposable chat memory and active game state over persistent logs.


Extend the existing history service, rather than adding an invisible game-only log. Preserve original game bytes and channel/session/sequence metadata; display records through normal channel chat and game views. Propose a **64 KiB bounded journal** as an initial storage quota, subject to filesystem and latency tests. Use bounded caches, record length/version/CRC/commit markers, and explicit eviction/truncation metadata. Do not rewrite a whole journal per move or block radio processing with bulk scans.

Host commits become durable before being treated as committed; surface storage failure. Recovery must detect torn writes and conflicting duplicates. Normal history retention and any new persistence behavior must be documented. Add atomic game preferences in `/games1`; leave the `NodePrefs` and tracker schemas unchanged.

## 7. Phase C — private-channel Tic-Tac-Toe

Proposed modules: `GameCodec`, `PrivateChannelProvider`, `LobbyManager`, `GameSession`, and history adapters. Prefer fixed tables and one active game-state union. Budget the complete initial online layer at **≤4 KiB additional live RAM**, then measure it; do not grant each module its own 4 KiB allowance.

1. Use `~G1:` plus canonical Base64url events in ordinary private-channel text. The real limit is **160 bytes including the sender-name prefix**; `sendGroupMessage()` silently truncates excess. Compute the exact remaining budget before sending and reject oversize events. A conservative body target is **100 characters**, supporting up to 72 packed bytes including the proposed 20-byte header/check fields. Do not fragment Tic-Tac-Toe turns.
2. Support eight discovered lobbies, one playing session, explicit join/host acceptance/readiness, and random 64-bit session IDs. Bind sessions to channel-key fingerprints, not mutable slot numbers or display names.
3. Host assigns contiguous commit sequences. Proposals carry client action IDs; duplicates return the existing result. Serialize concurrent joins and validate actors, turns, rules versions and moves before commit.
4. Persist every authoritative transition into visible channel history. Mirror host-originated commits through the same store as incoming events. Resends retain session/event IDs and do not become new commits.
5. Bound retry count, jitter, acknowledgements and out-of-order holding buffers. No periodic heartbeat, polling discovery or per-frame radio traffic. Manual reannouncement uses the same lobby ID; automatic reannouncement is off initially.
6. Add persistent invite-popup preference, deferred notifications during active play, special chat actions, and passive matchmaking independent of popup preference.
7. Channel deletion/rekey immediately stops session transmissions. Treat private-group sender claims as unverified unless an actual authentication layer is added; encoded actor IDs are not proof of identity.

Gate: two real devices complete matches while ordinary mesh functions remain usable. Test simultaneous hosts, no eligible channel, rekeys, full lobbies, packet loss/duplicates/reordering and malformed input.

## 8. Phase D — complete-history replay and spectators

Replay through the same deterministic rules engine. Admit spectators only with the original `LOBBY_OPEN`, compatible rules, and every contiguous authoritative transition through the known head. Missing opening, gaps, eviction, corruption or conflicting duplicates disable spectating. A snapshot cannot replace missing history.

Provide an explicit host-advertised commit head; without recent head knowledge label the view complete only through event N, with live status unverified. Spectators send no moves, require no player slot and do not trigger automatic resync traffic.

Test power loss, reboot, torn records, ring/journal eviction, sequence overflow, host disappearance and complete versus partial histories. Release **Tic-Tac-Toe 1.0** only after physical two-player plus third-device spectator checks and resource gates pass.

## 9. Later games, each behind its own gate

| Order | Next increment and hardware constraints |
| --- | --- |
| Snake | Offline only; fixed-step movement and measured OLED refresh/input latency. The GDD's 20–30 FPS remains aspirational, not promised. |
| Checkers | 8×8 board at 7-pixel cells; the requested backward captures/flying kings/immediate promotion, mandatory capture tests; bounded CPU search and complete replay. |
| Battleship | 10×10 board at 5-pixel cells; private local fleets, salted cryptographic commitment and reveal audit; public-state-only spectators. No shared-channel plaintext placements. |
| Scorched Earth | Fixed-point deterministic physics and fixed terrain arrays; local effects only, 2–4-player host commits. |
| Texas Hold'em | Remains disabled until recipient-specific authenticated encryption works inside the selected group channel and fits packet/memory limits. Shared channel encryption cannot hide hole cards from other members. |
| Creature Arena | Superseded by Veylith: 60 original 1-bit fronts, mirrored at runtime for the player, Trainer/CPU first, then PvP. The 7,680-byte creature bitmap data excludes UI art, rules and metadata. No seven-game fit or schedule is promised. |

Do not add games until offline, online, matchmaking and full-history Tic-Tac-Toe meet their gates. Keep CPU/memory budgets explicit and preserve existing device functions.

## 10. Validation and first implementation deliverable

Run `pio test -e native -e native_kiss_modem -vv`, all current `test/tracking/test_*.py` entry-point checks, and builds for the OLED target plus existing tracking targets. Add a host-native game environment containing pure rules and simulated 2–4-device sessions, without display/radio dependencies. Exercise loss, delay, duplicates, full history, eviction, privacy classification and protocol fuzz cases.

For every milestone record application-section sizes, heap counters, minimum stack headroom, OLED frame time, maximum work per loop, journal capacity/recovery behavior and actual transmitted game bytes. Do not count `.heap` as occupied application state when interpreting ELF section totals.

The first implementation PR should contain **memory accounting/recovery and bounded shared-history foundations**, with gameplay still disabled. The next PR delivers offline Tic-Tac-Toe. No phase should silently consume the entire nominal RAM or flash capacity, cut existing functions, or claim multiplayer/spectator validation from a single attached device.
