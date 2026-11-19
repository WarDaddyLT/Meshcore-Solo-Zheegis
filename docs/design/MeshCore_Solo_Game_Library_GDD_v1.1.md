# MeshCore Solo — Game Library Design & Codex Implementation Specification

**Status:** Implementation-ready design specification (source integration requires repository audit)  
**Revision:** 1.1 — 2026-11-14 (Checkers + Battleship replace Connect Four + Dots and Boxes)  
**Primary target:** Seeed Studio Wio Tracker **L1 Pro OLED**  
**Firmware:** [MeshCore Solo](https://github.com/MarekZegare4/MeshCore-Solo) (custom fork/feature)  
**First playable milestone:** Tic-Tac-Toe, single player versus CPU and two human players over MeshCore  
**Final flagship milestone:** Original monochrome **Creature Arena**  
**Priority:** Good games and reliable messaging over maximum game count.

---

**Arena amendment (2026-11-15):** [Veylith Arena](Veylith_Arena_GDD_v1.0.md)
supersedes the Creature Arena roster, progression and content outline below.
Store 60 front sprites only (7,680 bytes); horizontally mirror the front at
render time for the player. Separate rear images and duplicate exports are
removed. Other games retain their existing design sections.

## 0. Instructions to Codex

Implement this document **in phases**, starting with the smallest shippable Tic-Tac-Toe increment. Inspect the actual checked-out firmware before writing integrations: do **not** invent Solo class names, channel APIs, nonvolatile-storage layout, message-size limits, display drivers, or build environment names.

1. Record the checked-out Git commit; inspect the actual `solo/`, `src/`, `include/`, board configuration, display abstraction, input, chat history, channel metadata, settings persistence, receive/send routing, and existing unit-test infrastructure.
2. Establish and **measure a baseline build** for the L1 OLED/L1 Pro target. Report used/free flash and static RAM and any available runtime heap/stack high-water information. Preserve all original radio, BLE, GPS, chat, contacts and device functions.
3. Build the shared game interfaces and **offline Tic-Tac-Toe versus CPU** first. No radio packets in offline mode.
4. Add **private-channel-only** lobby creation, discovery, invitations, chat event presentation, popup preference, and two-player Tic-Tac-Toe using the existing MeshCore communications stack.
5. Add loss/deduplication handling, history replay and **spectator mode based on a complete existing local chat history**. Do not skip this before expanding the library.
6. Add later games one at a time, with individual release/quality gates. **Do not implement all games simultaneously.**
7. Provide compilable code, automated tests where practical, hardware test notes, firmware size diff and an updated implementation checklist for each phase.

**Non-negotiable:** Gameplay is an extension of MeshCore Solo, not an alternative mesh stack. **Never provide channel creation, channel import, key editing, or channel deletion UI inside Games**. That infrastructure belongs to Solo or its companion app.

**Known baseline facts** (verify in the checked-out tree): nRF52840 Cortex-M4F 64 MHz, 1 MB internal flash, 256 KB RAM; 1.3-inch 128×64 black/white OLED; 4-way joystick, menu button, buzzer; L1 Pro documentation shows 2 MB external QSPI flash (availability to game code not assumed). Existing Solo documentation identifies the OLED Wio build as `solo-<version>-WioTrackerL1.uf2`; determine the actual PlatformIO environment from the tree. **L1 Pro uses the L1 OLED class of display; confirm pin/input board variation during integration.**

Sources to consult:
- https://github.com/MarekZegare4/MeshCore-Solo
- https://wiki.seeedstudio.com/wio_tracker_l1_node/
- https://github.com/tchellow/MeshCore-Solo/blob/main/docs/companion_protocol.md (channel distinctions; secondary document, not a substitute for source inspection)

---

## 1. Product vision and scope

A compact, polished monochrome game collection available from a **Games** menu in MeshCore Solo. The first purpose is enjoyable offline play, followed by deliberately joined 2–4-player, low-airtime turn-based games over **existing random-key private MeshCore channels**. A game broadcast to a channel is not an invitation that a receiver has automatically accepted. Channels may contain several unrelated lobbies and several ongoing matches simultaneously.

Core features:

- Single-player offline games with no radio dependency.
- Multiplayer game-hosted lobbies announced as special chat messages on **pre-existing private channels**.
- Discoverable open lobbies; players explicitly request and receive admission. No automatic joining merely because devices share a channel.
- Optional, persistent **Game invitation popups**; disabling popups **does not disable lobby discovery**.
- **Resend lobby announcement** manually; optional low-frequency automatic re-announcements, with stable lobby IDs and deduplication.
- Interactive special game-message presentation in existing chat and in Games > Matchmaking.
- **Read-only spectator mode** when the spectator's **locally retained chat history** includes the complete authoritative match event log **from matchmaking onward**, with no missing transitions.
- Minimal 1-bit assets, shared engine, deterministic rules, minimal heap allocations, no unnecessary radio traffic.
- A handful of excellent games; creature battler is the final, most ambitious feature.

**Out of scope:** full Pokémon-style world, collection/catching, leveling or campaign; general-purpose scripting VM; network services, cloud accounts, internet leaderboards, custom MeshCore channel provisioning; a separate LoRa stack; and guaranteed support for arbitrary third-party firmware that lacks this game extension.

---

## 2. Platform constraints and design rules

| Resource | Design implication |
|---|---|
| CPU: nRF52840 @ 64 MHz | C/C++ fixed-size structures; animation, physics and basic AI feasible. |
| Internal flash: 1 MB *total* | Solo, bootloader/SoftDevice/reserved regions and games share the budget. Measure linker map before assigning sizes. |
| RAM: 256 KB *total* | Preserve Solo's BLE/radio, chat, GPS, stacks and buffers. Avoid dynamic allocation in frame loops. |
| External QSPI: 2 MB reported | Do not assume free or writable: investigate partition/ownership before using for game assets or saves. |
| OLED: 128×64, 1-bit | 1,024-byte full framebuffer; reuse the firmware renderer/buffer, do not allocate duplicate full screens without evidence. |
| Input: 4-way joystick, menu button | Every core action reachable with available controls, including back/cancel/confirm. Test short/long press behavior against Solo navigation. |
| Buzzer | Optional, nonblocking effects, respect Solo sound/mute settings. |
| LoRa mesh | Turns, not frames. Never send sprites, animations, per-frame coordinates or polling heartbeats. |

**Visual rules:**

- Only black and white (no grayscale). Never rely on color to distinguish players, pieces, types or cards.
- Target smooth local UI/animations when practical (~20–30 render updates/s as a *target*, not an asserted hardware guarantee). Network events are asynchronous and usually much slower.
- Typical 5×7 or 6×8 bitmap text; test physical legibility on the real 1.3-inch screen. Offer scroll/pagination instead of microscopic text.
- Tile-based games use 6–12 pixel tiles. Creature battles use high-quality **front and back** sprites, hit/flash/attack animations and separate move screens.
- Player differentiation: X/O, filled/outlined, hatch/dotted, shape icons, text labels—not colors.
- Use shared 1-bit image operations: blit, XOR/invert, clip, line, rectangle, text, fill, simple masked sprite, dirty-region optimization if supported.
- Preserve Solo's status indicators or restore them as soon as Games is exited. Game rendering must never starve radio processing.

**Memory discipline:** `constexpr`/flash-backed immutable content; one active game-state union; fixed-size packet buffers; bounded lobby table; no STL vectors/strings in hot paths unless the existing project already safely uses them. **Do not promise a game count before measuring headroom.**

---

## 3. HARD INVARIANT — all multiplayer games use existing PRIVATE channels only

**Single-player local mode is networkless and needs no channel. Every radio message belonging to a multiplayer game MUST use a private MeshCore channel that already exists in Solo or its companion app.** This includes lobby announcements, joins, ready, starts, moves, results, replay events and future games. No public-channel or predictable-hashtag-channel fallback.

MeshCore distinction:

- Default public channel: **ineligible**.
- Hashtag/channel-name-derived-key channel (`#something`): **ineligible** even though encrypted on air, because its key can be derived from a guessable name.
- Random-secret-key **private channel**, already configured in Solo: **eligible**.

**Implementation requirement:** determine channel type from authoritative channel metadata, stored origin/mode, or a reliable implementation-specific representation of randomly assigned private keys. **Do not decide privacy by display name alone.** If the present Solo data model cannot reliably distinguish private from public/hashtag, implement a read-only classification bridge from existing channel metadata or add a migration-safe private-channel type attribute **without modifying any keys**. When classification is unknown, fail closed: channel is ineligible. Do not silently promote a hashtag channel to private.

### Required flows

**Host a game:**

1. Games > Multiplayer > Host > select installed game.
2. Show a picker containing **only existing eligible private channels**.
3. If none exist, show exactly the concept: **“No private channels configured. Add one in MeshCore Solo or the companion app.”** Offer Back only; no channel edit/add/import button, no key prompt.
4. Host chooses channel and permitted player count (within game-specific limits); a new session ID is generated; host slot P1 is allocated.
5. Host publishes `LOBBY_OPEN` game chat event to that channel. The lobby enters `OPEN`.

**Join or spectate:**

- Discover only from currently configured, eligible private channels.
- A received `LOBBY_OPEN` on public/hashtag/unknown channels is **never presented as joinable** and must not trigger game UI notifications.
- Joining uses the lobby's original private channel. A player must still have that same channel configured at join time.
- If a channel is deleted or no longer eligible during an active match, stop game transmissions, show channel unavailable, and allow a local read-only history view if complete.
- Games never change channel frequencies, settings, keys or channel membership.

### Channel selection is infrastructure, not matchmaking

The private channel is only the transport. Several lobbies, games and observers can coexist on it, each isolated by a unique session ID. Private channel membership does **not** imply game membership, acceptance, readiness, a player slot or an observer view.

---

## 4. Navigation, settings and 128×64 UI

### Menu

```text
GAMES
  Single Player
  Multiplayer
  Matchmaking
  Invitations
  Settings
```

- `Single Player`: launch installed games with offline support.
- `Multiplayer`: host; optionally view own active lobby/match.
- `Matchmaking`: **passive lobby discovery** across existing eligible private channels, even if popups are off. Select lobby > details > Join/Back. Optional Spectate for a match with a complete local history.
- `Invitations`: local pending/open game notices, distinct from accepting or actually joining.
- `Settings`: `Invite popups [ON/OFF]`, `Invite sound [ON/OFF]` (optional); `Show full lobbies [OFF]` (optional); no channel manager.

Popup preference defaults to **ON** for installed game announcements unless Solo's existing notification policy dictates a different safe default. Persist via current Solo preferences with schema migration and sensible fallback for old settings.

### Popup conditions (ALL required)

1. Packet received on an **eligible private channel**.
2. Recognized, supported game ID and compatible rules/protocol version are installed in **this firmware**.
3. Lobby is open, has an available slot, is not expired, and isn't a lobby the player already joined/ignored.
4. **Invite popups = ON**.
5. No popup already shown for this session (repeat announcements update an existing listing, not generate repeated interruptions).

Popup UI on 128×64, split into screens if needed:

```text
GAME LOBBY
TIC-TAC-TOE
Host: Alex
Players: 1/2
[ JOIN ] [ LATER ]
```

`JOIN` sends request, shows waiting; **does not automatically create a player slot**. `LATER` dismisses this popup only; lobby remains in Matchmaking and chat. If the user is currently in an active game, defer/queue the notification rather than interrupting gameplay. Settings OFF: no popup, but discovery continues and the special chat message remains visible.

### Special chat messages

Render recognized game events inside the **existing** private-channel chat history as compact contextual rows/cards, with button-like actions available on device:

- `Game lobby opened · Tic-Tac-Toe · Alex · 1/2` → `View` / `Join`.
- `Player joined · Alex vs Ben`.
- `Game started · Tic-Tac-Toe` → `Open` if participant, `Spectate` if eligible.
- `Move · Ben placed O at B2` (human-readable summary optional, preserve underlying structured payload).
- `Match ended · Alex wins` → `Replay` if full log.

**Important:** retain the actual machine-readable event and its channel/session/sequence metadata in the chat history; merely displaying a shortened text summary is insufficient for observer replay. Use existing persisted history facilities, with migration and retention checks. A game event should behave as a special message type **in chat**, not an independent unlogged hidden network subsystem.

Unknown game types/older firmware: show the compact marked payload as regular text or a neutral `Game event (unsupported)` line without crash. No incorrect assumption that the companion smartphone app parses custom game controls; device-side UI is the priority.

---

## 5. Generic matchmaking and lobby manager

### Requirements

- `LOBBY_OPEN` is **broadcast once immediately** to selected existing private channel.
- Host may press **Announce Again** whenever a lobby remains open, subject to a bounded minimum interval/radio-queue backpressure; same session ID each time.
- Optional automatic rebroadcast, conservative default **off** in first release; if enabled start with **about 5-minute intervals + jitter** and check actual airtime limits. Never announce more often simply because nobody joined.
- No network-wide authoritative persistent lobby directory. Discovery is based on locally received announcements and local history.
- On repeated announcements, update the existing lobby record/time/status. **Never duplicate lobby entries or reshow popup** for a previously displayed session.
- Maintain a small bounded list (target up to 8 discovered lobbies initially), expire stale entries (e.g. after ~10 min without host updates); do not infer that a historical chat announcement is still open.
- Host controls join acceptance and slots; `JOIN_REQUEST` is a request, not acceptance.
- Lobby `OPEN`, `FULL`, `STARTED`, `CANCELLED`, `EXPIRED`; player `HOST`, `PENDING`, `JOINED`, `READY`, `SPECTATOR`, `LEFT`.
- Readiness required for start (host can configure minimum participant count; Tic-Tac-Toe always 2 humans online).
- Host sends `LOBBY_CANCEL` if cancelled, `MATCH_START` when launched, `MATCH_END` at completion, and appropriate updates when slots change.
- One active multiplayer **playing** session per device for v1; discovery of other sessions is allowed in background without spamming/interrupting.
- Hosts cannot force incoming spectators or players into a game by simply sending chat messages.

### Host-authoritative event ordering

For deterministic replay, **the host assigns a monotonically increasing committed-event sequence**. Clients submit proposals (`JOIN_REQUEST`, `READY_REQUEST`, `MOVE_REQUEST`), host validates and **broadcasts authoritative event commits** (`PLAYER_JOINED`, `PLAYER_READY`, `MATCH_START`, `MOVE_COMMIT`, `MATCH_END`). Only committed events mutate the authoritative game state on other devices. A host's own action goes through the same commit path.

- Host must serialize conflicting join requests; at most `maxPlayers` slots.
- Player binds to a session-assigned slot only after `PLAYER_JOINED`.
- Invalid/out-of-turn moves never become commits; display a local error or send a small rejection where justified.
- Keep event identifiers/idempotency tokens so retransmissions cannot apply a move twice.
- Match packets are always associated with `(private_channel_id, session_id, game_id, rules_version)`; never mix matches merely because they share a channel.
- Human-friendly chat names are **display data**; do not pretend that Base64/ASCII/CRC/hash proves a radio sender's identity. Initial trusted-private-group play is acceptable; note limits in docs, avoid overclaiming anti-cheat.

---

## 6. Complete-history observer/spectator mode

**User rule:** An observer may view a game only if their device already has **the entire log of that match, including matchmaking, in its local private-channel chat history**. Being on the channel alone is insufficient. Spectating is a **local read-only replay/live-follow feature**, not an invitation to join a player slot.

### Authoritative log contract

The following commits must be written into normal local channel history, with session ID, sequence and payload retained:

1. **First `LOBBY_OPEN`** (start marker; includes game/rules version, initial host, lobby capacity, session ID).
2. Every authoritative membership/slot/ready transition that affects the session.
3. `MATCH_START`, including complete initial public game configuration and deterministic RNG seed or all public random outcomes required for replay.
4. Every authoritative `MOVE_COMMIT` and other state-changing event, in sequence, without gaps.
5. `MATCH_END` (if completed) or cancellation/termination event.

Repeated lobby advertisements and redundant retry copies are **informational duplicates** and need not be present in the replay *as separate state changes*. The initial matchmaking event and all distinct authoritative events **are mandatory**. If a member loses some state-changing event, it cannot qualify as complete merely because it later received a summary or snapshot.

### Observer admission check

`canSpectate(history, privateChannel, session)` must return true only when:

- Local chat storage has `LOBBY_OPEN` from the beginning for the same channel/session.
- Game ID and rules version are supported locally.
- The entire **committed event sequence is contiguous** from opening through current known committed sequence; no gaps, corruption, conflicting duplicates or unsupported state-changing events.
- All committed events replay successfully under the local deterministic rules and state checksums; private-channel classification still valid.
- For *live* spectating, the local receiver knows current commit head reliably (e.g. host announcement/status including latest committed index). Otherwise label the view **“History complete through event N; live status not verified”** and don't incorrectly claim the game is caught up.

On success, show `Spectate`/`Replay`. Observer reconstructs from the local chat event log, renders board/animations and follows future commits; **never sends a MOVE**, never takes a player slot, never affects turn order. No host approval needed just to **view data already available to that channel member**. `Leave Spectator` simply exits the view.

If history is absent, truncated, reordered with a missing gap, or began after matchmaking, show **“Cannot spectate: complete match history is not available on this device.”** Do not invent earlier moves or silently substitute a snapshot. Consider allowing `View chat` to inspect the partial conversation, but **do not enable spectator game view**.

### Storage implications

- Investigate actual Solo chat storage/persistence/retention behavior. MeshCore is not a cloud archive; observer access depends entirely on local preserved history.
- Preserve the original `LOBBY_OPEN` and game event sequence for active sessions as long as possible **within existing history/storage limits**, without silently changing retention policy or consuming unbounded flash.
- Make history eviction detectable. If oldest match events have been evicted, **Spectate becomes unavailable**. For long creature/poker matches, log length and storage limits may make retrospective spectating unavailable; document this honestly.
- If future work introduces session-specific compact archival replay, it must still be verifiably derived from the complete recorded chat event sequence; **v1 must not grant observer access using a mere snapshot**.
- Poker and Battleship observers can reconstruct only **public** state before proper reveal; shared-channel logs alone must never expose concealed cards or ship placements. See §10.

---

## 7. Game-over-channel packet format (v1 proposal, adapt to actual Solo APIs)

Use an application-layer **distinctive text marker** in existing MeshCore private-channel text messages, unless the current code exposes a stable existing user-data payload that is equally persisted/displayable in chat. Do not change the underlying MeshCore RF protocol. Example wire representation:

```text
~G1:<base64url_of_packed_binary_event>
```

**Encoding is not encryption.** The private channel provides its configured on-air group encryption; payload encoding exists to keep messages short, parseable, and not confused with casual chat. Avoid raw binary in a text interface if it breaks UTF-8 assumptions. Do not use ASCII conversion, XOR or hashing as a substitute for actual secrecy for poker hole cards.

### Suggested packed fields

| Field | Size | Meaning |
|---|---:|---|
| `protocol_version` | 1 B | Currently 1 |
| `event_type` | 1 B | See types below |
| `game_id` | 1 B | Stable numeric game identifier |
| `rules_version` | 1 B | Independent per-game compatibility |
| `session_id` | 8 B | Random 64-bit session identifier generated by host |
| `event_seq` | 2 B | Authoritative commit index; proposals carry reference rather than allocating commits |
| `turn` | 2 B | Game turn / round |
| `actor_slot` | 1 B | Player 1–4 or 0=host/system |
| `payload_len` | 1 B | Actual data length |
| `payload` | variable | Event-specific compact fields |
| `check` | 2 B | CRC16 for corruption/detection, not authentication |

The exact packed layout is **provisional**. Implement canonical byte serialization explicitly (no compiler struct-padding/pointer casting), stable byte order, length/version validation, bounds checking and test vectors. Bound ASCII-encoded payload length to what the **current Solo implementation** can actually transmit in one message; aim for compact messages, e.g. ordinarily under ~80–100 text characters where the real limit permits. **Never fragment normal Tic-Tac-Toe moves.** If payload is too large, redesign content before adding fragmentation.

### Event types

- `LOBBY_OPEN`: includes title/game ID, host display name or stable compact identity as supported, max/min players, session ID, open state, current committed head.
- `LOBBY_ADVERTISE`: repeat existing open-lobby notice with same session ID and updated player count/head; non-authoritative for replay.
- `JOIN_REQUEST`: prospective player asks; no automatic admission.
- `PLAYER_JOINED`: committed slot assignment from host.
- `JOIN_REJECT`: full, closed, incompatible or other reason.
- `READY_REQUEST` / `PLAYER_READY`: proposal and host commit.
- `MATCH_START`: authoritative initial state, player order, RNG seed/parameters if applicable.
- `MOVE_REQUEST` / `MOVE_COMMIT`: submitted action and canonical committed action.
- `MATCH_END`: result, public final state/checksum.
- `LOBBY_CANCEL`, `PLAYER_LEFT`, `TIMEOUT`, `SESSION_STATUS`.
- `ACK` / optional `RESYNC_REQUEST`: non-gameplay control packets; duplicate-safe, minimized.

**Important observer rule:** proposal requests need not be part of the *authoritative replay* if they caused no change. All **committed** transitions must be in chat history. Raw requests may still be shown/retained as special messages where useful. `SESSION_STATUS` conveys latest committed index so live observers know whether they have missed an event.

### Sequence, reliability and airtime

- Host accepts a proposal once using `(session, actor, client_action_id)`; duplicate proposals get the same result, never two committed moves.
- Host rebroadcasts a commit on bounded retry if a required player has not acknowledged, with randomized backoff; **dedup by `(channel, session, commit_seq)`**.
- Receiver maintains a small bounded out-of-order holding queue. If sequence N+1 arrives before N, wait/retry; **never apply it ahead of N**.
- No recurring per-second radio heartbeat, animation update or lobby polling. Manual re-announcement is allowed; default lobby refresh should be conservative.
- All event reception enters through a single parser after Solo's channel decryption; dispatch must be nonblocking.
- Integrate with Solo's duty cycle/airtime queue and existing message retry policy. Do not bypass radio limits.
- If a commit is irrecoverably missing, show `Waiting for missing move / out of sync`, not a fabricated board. A resync snapshot may help active participants in a later iteration but **does not** satisfy the spectator's complete-history condition.
- Define behavior for device sleeping, reboot, missing host, host crash, session ID collisions, malformed messages, incompatible versions, user exits, and stale lobbies. No feature may stall normal messaging.

---

## 8. Shared software architecture

The following are **proposed responsibilities**, not asserted existing Solo source file names:

```text
Solo existing UI / input / chat / storage / radio
    |
    +-- GamesMenuController        (navigation, settings, popup queue)
    +-- GameRegistry                (installed IDs, versions, metadata)
    +-- PrivateChannelProvider     (read-only lookup, eligibility)
    +-- GameChatCodec              (safe parse/encode + special chat views)
    +-- LobbyManager               (host/discover/join/ready/close/reannounce)
    +-- SessionManager             (participants, commit ordering, retries)
    +-- EventHistoryReplay         (contiguous log validation, spectating)
    +-- GameRenderer               (shared 1-bit text/sprite/draw primitives)
    +-- GameInputAdapter           (joystick/button mapping, back behavior)
    +-- GameRuntime                (one active game state + update/render loop)
        +-- TicTacToe
        +-- Snake
        +-- Checkers
        +-- Battleship
        +-- ScorchedEarth
        +-- TexasHoldem
        +-- CreatureArena
```

### Minimal game interface sketch

```cpp
struct GameInfo {
    uint8_t id;
    uint8_t rulesVersion;
    const char* title;
    uint8_t minPlayers, maxPlayers;
    bool supportsSolo;
    bool supportsSpectators;
};

struct GameEventView { /* validated, bounded parsed wire event */ };

struct IGameRules {
    virtual void reset(const GameConfig& config) = 0;
    virtual bool isLegal(uint8_t actorSlot, const GameAction& action) const = 0;
    virtual void applyCommitted(const GameEventView& event) = 0;
    virtual uint32_t publicStateHash() const = 0;
    virtual bool isFinished() const = 0;
};
```

This sketch **is not an instruction to use virtual functions everywhere**; static dispatch/table-of-functions or tagged unions may be preferable for constrained firmware. Prefer existing project conventions. Store one game state at a time, but allow a small background lobby discovery cache.

### Separation of concerns

- `GameRules`: pure deterministic logic without hardware, radio or UI dependencies (host unit tests).
- `GameRuntime`: local navigation and rendering, never sends messages directly from render callbacks.
- `SessionManager`: turns, session IDs, network delivery, deduplication and authoritative commits.
- `LobbyManager`: public-on-private-channel announcements, consent, acceptance, readiness.
- `HistoryReplay`: reconstructs **the same rule engine** from the canonical local event stream; not a second hand-written simulator.
- `PrivateChannelProvider`: **read-only**, uses existing Solo channel configuration.
- `GameSettings`: extends existing settings safely; never configures mesh infrastructure.

---

## 9. Game roadmap — EASIEST to HARDEST

A **7-game** quality-first portfolio. Do not fill remaining flash with filler games. Each game must be polished, testable and playable before moving to the next one.

| Order | Game | Modes | Players | Why at this stage | Core graphics |
|---:|---|---|---:|---|---|
| 1 | **Tic-Tac-Toe** | Offline vs CPU; private-channel PvP | 1 or 2 | Smallest state, proves all shared infrastructure | Large clean X/O board, win line, highlight, sound |
| 2 | **Snake** | Offline | 1 | Simple active animation, no network, verifies responsiveness | Crisp grid, movement, fruit, speed scaling |
| 3 | **Checkers** | Offline vs CPU; private PvP | 1 or 2 | Adds forced captures, multi-jumps, kings and a search-based AI while keeping the entire state public | 8×8 board, monochrome patterned sides, king markers and move/capture cues |
| 4 | **Battleship** | Private PvP; optional solo CPU after PvP is reliable | 2 | Adds hidden information, placement commitments, shot results and two separate board views | Readable 10×10 targeting/fleet grids, hit/miss/ship symbols and animated impacts |
| 5 | **Scorched Earth** | Private-channel multiplayer; optional CPU | 2–4 | Introduces deterministic physics and shared effects | Terrain silhouette, tanks, visible projectile arcs |
| 6 | **Texas Hold'em** | Private-channel multiplayer | 2–4 | Hidden information, bets/rounds and host dealing | Legible suit/rank icons, pot, hand, actions |
| 7 | **Creature Arena** | Offline testing/training; private-channel PvP | 1 or 2 (2v2 later) | Largest rules/content/art and synchronization challenge | High-quality 1-bit fronts, runtime mirroring and attacks |

**Milestone sequencing:** Game #1's **single-player + two-player + matchmaking + observer replay** is the first full release. Snake and other games come **after** this infrastructure works. Do not delay the online Tic-Tac-Toe slice to implement easy solo games first.

### Game 1: Tic-Tac-Toe — REQUIRED FIRST

**Rules:** Standard 3×3 X/O, alternate turns, X starts, 8 winning lines, draw after 9 valid placements. An invalid or occupied cell cannot overwrite the board, consume a turn or transmit a committed action. Timer optional and disabled by default.

**Solo:** CPU difficulty `Easy` (random legal), `Normal` (basic tactical block/win), `Hard` (minimax, never loses). CPU computation must not block the MeshCore main loop. User may choose X/O; if player is O the CPU makes the initial move.

**PvP:** Exactly two participants (host P1/X, guest P2/O by default, optionally alternate on rematch). Lobby: create on private channel, broadcast notice, receiver popup only if enabled, join request, acceptance, both ready, start, coordinate commit, result. Each move payload can be a **single 0–8 cell index**. Two simultaneous attempted moves: only correct current player's legal move is committed. A duplicate move retransmission cannot alter game state.

**Rendering:** Real 128×64 board with approximately 16–18 px cells, clear selected cell cursor, current player, simple win line and result overlay; use remaining pixels for score/turn hints. Controls: joystick moves selection, press confirms, menu/back pauses or exits with confirmation. Avoid rendering unreadable 5-pixel text.

**Observer:** A third device that received/logged **initial `LOBBY_OPEN` and every committed event** may choose **Spectate**, see current board and replay individual turns. Device arriving halfway or with evicted early history **cannot**. An observer may not send moves or occupy a player slot.

**Required automated cases:** all 8 wins; empty draw; occupied cell; invalid index; 2-player alternation; premature move; duplicate `MOVE_REQUEST`, duplicate `MOVE_COMMIT`; host rejection; popup preference on/off; no private channels; public/hashtag excluded; replay complete/incomplete log; three devices with spectator; loss/reorder; multiple lobby IDs on same channel.

**Done means:** A device can play locally, two devices can complete a full private-channel match, a third device with the complete history can spectate, and normal MeshCore messaging continues simultaneously.

### Game 2: Snake

- Offline only, no channel selection, no radio traffic.
- Board sized to leave a legible score banner; 1-bit head/body/food shapes; joystick steering (reject 180° reverse into self); adjustable starting speed; increasing challenge; restart.
- Aim for smooth motion with fixed timestep; input must not lag; game pauses/backs out cleanly.
- Reuse existing framebuffer, basic rendering and sound; no sprite asset bloat.

### Game 3: Checkers

- **Ruleset:** implement a clearly labeled, fixed **American/English checkers (8×8)** ruleset for v1; two players, 12 pieces each on dark squares. Do not mix international checkers rules into the same game version.
- Men move diagonally **forward** and capture diagonally forward; kings move and capture one diagonal step in either direction (not flying kings). Capturing is **mandatory** when available. Continue **mandatory multiple captures** with the same piece until no further capture is available. A man reaching the far rank is crowned; under this ruleset the move ends at crowning, even if a newly crowned king could continue capturing. Fix all rule nuances in tests, not in UI assumptions.
- Alternate players after a completed legal move/multi-jump sequence; invalid or non-capturing moves when a capture is available do **not** advance the turn. A player with no pieces or no legal moves loses. Define a deterministic, documented draw policy (e.g. agreed draw and a bounded no-capture/no-promotion move threshold) rather than allowing indefinite games.
- **Solo:** selectable Easy and Normal CPU using legal-move generation and bounded search. Hard mode only if measured computation stays nonblocking and preserves Solo responsiveness. Do not overpromise a deep AI.
- **PvP:** 2-player private-channel lobby with host-authoritative committed actions. Encode move as start square plus full capture/landing path (or a bounded sequence of jump steps), not just source/destination where multi-jumps could be ambiguous. Reject impossible paths, ownership violations, premature king moves and stale requests.
- **Graphics:** entire board readable at 128×64, e.g. 8×8 board at ~7 px/cell with a narrow status column; distinguish players by filled versus outlined/patterned pieces and kings by a central mark/crown, never color. Clearly indicate selected piece, available destinations and mandatory captures. Local piece slide/capture animation only; no per-frame radio packets.
- **Spectator:** public deterministic replay from `LOBBY_OPEN`, `MATCH_START` and every contiguous `MOVE_COMMIT`; render full board and move history. Add regression tests for captures, forced jumps, crowning, no-move losses, draw logic, replay, duplicate and out-of-turn moves.

### Game 4: Battleship

- **Ruleset:** classic **two-player** hidden-fleet naval duel; v1 uses a documented 10×10 grid and standard five-ship fleet lengths **5, 4, 3, 3, 2** (no diagonal ship placement; no overlaps). Fix a single deterministic rule for shots/turns (default: one shot per player turn, including after a hit). Keep four-player support for other games; do **not** complicate Battleship with experimental 3–4-player rules in v1.
- **Setup:** each player privately places ships using the joystick, rotating and confirming them; provide an optional auto-place function with valid non-overlapping placement and local preview. Store actual placement **locally**; it must not appear in shared channel messages during play.
- **Commit/reveal:** before first shot each player sends a cryptographic **commitment** to their board encoding plus a fresh, unpredictable per-match nonce, and a `FLEET_READY` public event. The commitment must not reveal the board. At match end, publish board plus nonce, verify they match the original commitment, and recompute all hit/miss/sunk answers from the complete public shot log. A commitment prevents after-the-fact board changes, **but does not prove truthful hit/miss answers during the game**; disclose that casual-play limitation. Never claim a CRC or plain board hash without nonce guarantees secrecy. If a suitable cryptographic digest cannot fit, postpone this game's network release rather than pretending encoding hides the fleet.
- **Turn protocol:** host commits canonical `SHOT_REQUEST` (coordinates), defending player replies with hit/miss and sunk-status claim, host commits the validated-format `SHOT_RESULT` and advances turn. Maintain a bounded outstanding-shot identifier, handle retries and deduplication; reject repeated coordinates, results for nonexistent shots, out-of-turn attacks and altered commitments. In v1 the host is authoritative for event ordering **not** omniscient about the opponent's hidden fleet; final reveal audits claims.
- **Graphics:** two switchable views: own fleet with incoming shots, and targeting board with known misses/hits/sunk ships. A 10×10 5-pixel-cell grid occupies 50×50 pixels; remaining area for turn/status, legend and selection details. Distinguish unknown, miss, hit, selected coordinate and sunk segments by 1-bit symbols/patterns. Use short local fire/splash/explosion animations, no streamed frames.
- **Solo:** optional CPU opponent after 2-player rules and synchronization work; use a basic hunt/target strategy, no separate online dependency.
- **Spectator:** observe **public information only** until final fleet reveal. Admit observers only if their local normal channel history contains `LOBBY_OPEN` and the full contiguous authoritative history including both `FLEET_READY` commitments, each shot, every published result, and match end where applicable. Never reconstruct or show unrevealed private ship placements. After a valid reveal, provide optional audit/replay with actual fleets. If the commitment, turn log, or reveal is missing/inconsistent, show incomplete/unverified status and do not claim full verified final replay.
- **Quality tests:** fleet bounds, no overlap, rotation/placement, repeated/missed shots, hit/sunk/win rules, commitment mismatch, maliciously incorrect result detected on reveal, disconnection, retries, incomplete spectator history and OLED readability.

### Game 5: Scorched Earth

- Turn-based 2–4 players, original minimalist artillery game; one shot = selected angle, power, weapon type (if multiple), committed by host.
- Deterministic integer/fixed-point ballistic physics (avoid differing floating-point results and platform-specific math). Terrain seed and per-shot effects established in shared committed data; host may also commit post-shot canonical damage if deterministic equality is difficult.
- High-quality 1-bit terrain, tank silhouettes, projectile motion, pixel explosion and terrain deformation, using **local animation only**.
- Limit terrain complexity and keep state in fixed arrays. Replay every shot; handle simultaneous team elimination and ties.

### Game 6: Texas Hold'em

- 2–4 participants, clear betting turns (fold/check/call/raise), blinds, public flop/turn/river, pot accounting and showdown. Chips are **virtual only**; no real-money play, wagering integrations or payments.
- Host is trusted dealer in v1. However **all game-related radio traffic stays on the selected existing private channel**.
- **Critical hidden-hand constraint:** a private channel key is shared by all members; channel encryption alone does **not** hide one player's cards from other members. Base64/ASCII encoding or simple hashes **cannot** fix this. To implement genuine hidden hands while keeping the private-channel-only requirement, use independently encrypted **recipient-specific game payloads within messages on the same selected private channel**, using suitable key agreement/encryption that is actually available/implemented and tested. Do not introduce another user-created channel or unapproved direct-message transport.
- If recipient-specific secrecy cannot be provided within memory, packet and code constraints, **leave networked poker behind a feature gate** rather than ship trivially readable cards and call it fair poker. A trusted-host design permits host knowledge of all hands, but not unrestricted other-player knowledge.
- Observer log contains all committed **public** outcomes and player/bet actions; spectators **never** display another player's unrevealed hole cards. Replay engine has a public-state projection.
- Cards at 128×64 need multiple screens/pagination; suits must use clear distinct pixel glyphs, not red/black semantics.

### Game 7: Creature Arena — FINAL flagship

**Game identity:** Original creature designs, original names/art/moves. Borrow genre mechanics of compact turn-based monster combat, not copyrighted Pokémon art or species. Battle-only: no world, catching, leveling or inventory.

- Each player builds a team of **six creatures** from an unlock-free roster; **all level 99**. Each creature has **four customizable legal moves** selected from its allowed move pool.
- Start quality-first with about **24 original species** and **48 well-balanced moves**, rather than promise 151 poorly polished species. Increase only if measured flash/assets budget, balance tests and art quality justify it.
- Model HP, Attack, Defense, Special, Speed, type interactions (start with 6–8 original types), accuracy, priority, status effects, fainting, forced replacements and switching. Avoid a complicated breeding/IV system in v1.
- Six-member roster editor; select creature, assign 4 moves from legal pool, view type/stats and save several teams if storage permits. Roster integrity/version compatibility validation before starting PvP.
- Battle is 1v1 active creature at a time, two players with six-member teams; both players select actions, then host deterministically commits the resolved turn in speed/priority order. Combat RNG must be replayable from committed seed or explicit outcomes.
- **Graphics target: Gen-1-style 1-bit battles, NOT just static cards.** Each species stores one detailed **front** silhouette, typically 32×32 (larger if composition/UI permits); the player uses a runtime horizontal mirror of it without another bitmap or scratch buffer. Have purposeful distinct anatomy/readable outlines at true OLED scale. Visual features: attack projectiles, shake/flash on hit, recoil, impact particles, HP-bar drain, send-out/switch animation, faint sequence, critical-hit cue, status icons. Share effects between species to avoid per-creature animation-frame explosion. The screen may alternate between battle view, move selection, status and team list instead of overloading one frame.
- Illustration budget example: 60 creatures × one 32×32 raw 1-bit front sprite × 128 bytes = **7,680 bytes** for baseline silhouettes, excluding extra frames, masks, metadata and effects. This is an asset-only calculation, **not** total game firmware footprint.
- Spectator display of public battle state needs full canonical history from lobby through each action/turn resolution. Hidden preselected move choices may remain hidden until both actions resolve; reveal only the public resolved events in spectator view. The host/game rules must avoid leaking pre-resolution choices if that creates unfair play.
- Roll out in layers: (a) original sprite renderer and UI; (b) single local battle with fixed teams; (c) roster editor; (d) complete rules; (e) online PvP; (f) replay/spectator and balance polish. Keep the flagship disabled in production until complete.

---

## 10. Privacy, integrity and fairness

- **Required:** game radio messages travel only over configured random-key private MeshCore channels, never public or hashtag channels.
- A private channel is a shared encrypted group, **not** a per-player secrecy boundary.
- `~G1:` and Base64url are **format/obfuscation only**. CRC validates accidental corruption; a state hash detects divergence; neither authenticates the human sender.
- For simple cooperative board games among channel members, host-authoritative turn commits and sequence checking are sufficient for **casual trusted-group play**, but they do not prevent deliberate forgery by channel insiders. Document this limitation.
- For Battleship, poker and simultaneous hidden decisions in Creature Arena, make explicit rules about what stays private until reveal. Battleship's unpredictable-nonce board commitment permits *after-match* audit without publishing placements before play; it does not guarantee live honesty. For poker and other modes needing per-recipient secrecy, if required protection cannot be implemented, postpone those modes. Never treat an encoded secret as secretly encrypted.
- Do not print private channel keys or secret hands into debug logs or public telemetry.
- Treat all incoming message bytes as untrusted: bound-check lengths, names, slots, cell indices, turn ranges, game versions and packet structure before processing.
- No radio flooding, unreliable unbounded retries, auto-response loops or per-frame network messaging.

---

## 11. Persistent preferences, game saves and backward compatibility

- Add game settings through Solo's **existing** preference persistence mechanism; use a schema-safe migration/default path.
- At minimum persist: popup setting, optional invite sound, local game preferences, installed game exposure. Do not persist new channel secrets or duplicate Solo channel configuration.
- Save local high scores and Creature Arena prebuilt team rosters only if flash/FS quotas permit; use bounded writes and protection against flash wear. If space is inadequate, defer save slots without degrading normal messages.
- Preserve backward compatibility with earlier Solo firmware preference blobs and BLE companion app. Upgrading games must not erase channel configuration or existing message history.
- Changing a game rules version should reject incompatible PvP sessions but still allow users to read a neutral older event in chat.
- Provide a build flag such as `SOLO_GAMES_ENABLED` and per-game build flags so memory can be controlled. Use actual build conventions from the checked-out tree.

---

## 12. Memory, firmware size and performance gates

**Do not treat these figures as measured facts.** Baseline memory is unknown until Codex compiles this exact revision.

| Component or phase | Initial engineering target, subject to compilation |
|---|---|
| Shared framebuffer | Reuse current display buffer (~1 KB at 128×64 if single 1-bit buffer) |
| Extra RAM for lobby/session/parser/history cursor | Aim <= 4–8 KB active, then profile |
| Additional active game state | Aim <= 4 KB for early board games; allow larger measured budget for Creature Arena |
| Tic-Tac-Toe game logic/assets | A few KB extra flash; measure actual compiled delta |
| Shared game engine + UI/network | Prefer a few tens of KB, not an unmeasured 100 KB abstraction |
| 60 Veylith front-only raw 32×32 images; runtime player mirroring | 7,680 B *base sprites only* |
| FPS | Smooth local game visuals where possible (~20–30 fps aspirational); no networked rendering updates |
| Message length | Under actual safe Solo channel text limit, including marker and encoding |
| Persistent history | Must not exceed Solo's known storage quotas; detection of truncation mandatory |

At **every** phase, record:

1. Before/after `.elf`/`.map` flash sections, RAM sections and firmware artifact sizes.
2. Measured heap margin and stack high-water if the runtime allows.
3. Frame timing on physical L1 Pro; responsiveness under active LoRa/BLE/GPS operations.
4. Mesh messaging/regression and radio airtime behavior.
5. History storage behavior: capacity, eviction, reboot persistence.

Never trim MeshCore's core functionality to squeeze in an optional game without a deliberate user-approved decision. If a later game exceeds headroom, optimize assets/rules or omit that game.

---

## 13. Testing and validation matrix

### Offline / hardware

- Exact target builds for L1 Pro OLED; basic UI rendering, contrast/readability and every button direction tested on real hardware.
- Menu entry/exit repeated >100 times without leaked game state or slowdowns.
- Normal chat, GPS, BLE app connection, alerts and receive interrupts continue while game displayed.
- Idle game does not cause excess OLED/radio wakeups; optional animations stop when backgrounded.
- Games never transmit in offline mode.

### Channel privacy and discovery

- Host with 0 private channels: sees correct explicit explanatory message, **no Add Channel UI**, no game packet emitted.
- Mixture of public/hashtag/random-key private channels: picker contains **only random-key private** entries.
- Unknown channel type: blocked, not guessed from name.
- Incoming game marker on public/hashtag: no joinable lobby and no game invitation popup.
- Incoming valid lobby for absent/incompatible game: no popup, message remains neutral/visible in chat.
- Popup OFF: matchmaking still discovers lobby; manual join works.
- Popup ON: one notification per lobby, repeated broadcasts refresh listing without duplicate popups.
- Same-channel hosts simultaneously announce distinct sessions; no cross-session state.
- Host resend after a missed announcement: second device discovers lobby. Same ID, accepted players retained.
- Stale/full/started/cancelled lobby: correct UI and no invalid join.

### Match progression

- Join requires explicit user action and host acceptance; two simultaneous join requests cannot overfill slots.
- Readiness gates match start; disconnect and resume do not double-allocate slots.
- Delayed/repeated/out-of-order packets do not duplicate commits or corrupt state.
- Host/device reboot causes a defined recovery or clearly reported lost session, never silent false win.
- Every accepted turn is persisted as a structured event in local channel chat history.
- Repeated lobby announcements do not create duplicate committed opening events.

### Spectator history

- Third device present since `LOBBY_OPEN` with full retained committed log: `Spectate` available, board reconstructs correctly and updates live.
- Third device missed opening or move 3: `Spectate` denied; reason explicit.
- Old event evicted from local chat history: `Spectate` denied, even if latest snapshot matches.
- Corrupt/reordered/duplicate events: exact deterministic replay or denial on missing/conflicting commits.
- Spectator sends no game moves and occupies no player slot; can replay completed match from local chat history if complete.
- Poker spectators cannot view unrevealed hole cards.

### Game correctness

- Tic-Tac-Toe exhaustive reachable-state tests versus minimax (unbeatable hard AI).
- Checkers forced captures, legal chained jumps, crowning, king movement, terminal/draw rules, CPU legality and full-game spectator replay.
- Battleship placement bounds/no overlap, shot/turn validation, duplicate handling, salted commitment and end-of-game verification, partial-information spectators and incomplete-history refusal.
- Scorched Earth fixed-point trajectory replay equality on different devices.
- Poker correct hand evaluation and pot distribution, including split pots, all-in/side pots if supported (otherwise explicitly scope out until implemented).
- Creature Arena deterministic outcome consistency across devices, move legality, invalid deck rejection, all status/type cases, visual effects not affecting rules.

---

## 14. Delivery plan / milestone acceptance checklist

### Phase A — repository survey and baseline

- [ ] Identify actual game integration points: display, input, UI, private channel classification, transmit/receive, history persistence, settings.
- [ ] Produce baseline Wio OLED build, map/size report and current regression-test result.
- [ ] Decide how the chat history can retain structured game events and detect truncation.
- [ ] Propose minimal feature-flag and source layout consistent with Solo conventions.

### Phase B — Tic-Tac-Toe solo (first code)

- [ ] Games menu and offline Tic-Tac-Toe with X/O selection and three CPU difficulties.
- [ ] Pixel-perfect 128×64 board, selection, move feedback and result animation.
- [ ] Unit tests for all rule cases and AI; memory diff.
- [ ] No private channel required and **zero radio messages** in solo mode.

### Phase C — Tic-Tac-Toe private-channel multiplayer

- [ ] Existing-private-channel-only picker; exact no-private-channel message.
- [ ] Announce, resend, discover, join request, host acceptance, ready and start.
- [ ] `Invite popups` persistent ON/OFF and background passive discovery.
- [ ] Special game message in existing chat, interaction to view/join.
- [ ] Authoritative move commits, retry/dedup, end/result and rematch.
- [ ] Full private/public/hashtag/channel-less negative tests.

### Phase D — complete-history spectators and reliability

- [ ] Persist/parse original `LOBBY_OPEN` and complete ordered authoritative log in existing chat history.
- [ ] Reconstruct Tic-Tac-Toe state from chat history using the same rule engine.
- [ ] Explicit `canSpectate` admission, no missing-history bypass, one live observer on third device.
- [ ] Reboot, history eviction, missing event, duplicate commit and version mismatch tests.
- [ ] Release-quality **Tic-Tac-Toe 1.0**; stop here if core memory/safety gates fail.

### Phase E — additional polished games, one at a time

- [ ] Snake (offline graphics/performance benchmark).
- [ ] Checkers (offline CPU + 2-player, complete mandatory-capture rules and replay).
- [ ] Battleship (2-player hidden fleets, commitment/reveal, public-only spectators; optional CPU later).
- [ ] Scorched Earth (2–4-player deterministic local effects).
- [ ] Texas Hold'em **only if private-channel, per-recipient hidden-card handling is sound**.
- [ ] Creature Arena as final milestone, using original detailed 1-bit creatures/animations and a six-creature team builder.

### Definition of done for each game

- Controls and UI work without a phone.
- Readable at actual OLED size; no color dependency.
- No loss of messaging/BLE/GPS responsiveness.
- Rules covered by deterministic automated tests.
- Networking only over eligible configured private channels for online games.
- Matching uses explicit join; spectators use complete retained history.
- Frame time, flash delta, RAM delta and channel traffic documented.
- No unfinished/placeholder game enabled in production menu.

---

## 15. Suggested implementation notes for Codex

- Inspect the actual Solo source before deciding where to intercept channel texts. Prefer the **post-decryption receive path** and integrate with existing history rendering; don't duplicate chat storage.
- Extract pure game rule engines so host-native unit tests can run without hardware; design a simulation harness that runs 2–4 virtual devices, message loss, delay, duplicates and chat-history eviction.
- Keep protocol decoder fuzzable with malformed input fixtures.
- Use a stable registry of game IDs and per-game rules versions. Unsupported IDs must be safe to parse and ignore.
- Minimize added dependencies; prefer existing Solo drawing and preferences libraries.
- Do not use a separate network stack, Wi-Fi or internet for matchmaking.
- Never automatically push game invitations as direct messages to every contact; open lobby broadcasts go only to the specifically selected existing private channel.
- Preserve device-native human experience: host starts the game and sees lobby open; other players see the special message, optionally a popup; anyone can visit Matchmaking even with popups disabled; host can resend notice because there is no central history server.
- Implement spectating on the **same chat history that users can see**. History is the audit trail and the replay source; no invented past events.
- When integrating future hidden-information games, do not weaken these invariants to keep them in the library. Prioritize quality and correctness.

## 16. Codex execution prompt (copy into repo root)

> Implement `MeshCore_Solo_Game_Library_Design.md` in the current MeshCore Solo repository. Begin with Phase A: audit actual interfaces, baseline build and memory usage. Then implement Phase B, a polished single-player Tic-Tac-Toe with CPU levels, and continue to Phase C, private-channel lobby matchmaking and two-player Tic-Tac-Toe, followed by Phase D history-based spectating. All online games must use *only pre-existing randomly keyed private channels* selected from Solo; do not add channel configuration UI. Announcements must be recognizable special messages in normal channel chat, resends must reuse the same lobby ID, popups must be optional, and a listener with a complete retained chat-event log including the opening matchmaking event must be able to spectate without taking a player slot. Preserve the existing Solo messaging stack. Keep later games behind feature flags and don't start them before Tic-Tac-Toe meets acceptance tests. Report firmware size/RAM deltas and test results after each phase. Do not invent API names or hardware behavior; inspect the actual source first.

---

### Design decisions fixed by requester

1. All **multiplayer** games: pre-existing **private** MeshCore channels only; **no Games-side channel provisioning**.
2. Host opens lobby and posts special channel chat event; other users may accept/join explicitly.
3. Popups appear **only** with installed/compatible game and user setting enabled. With popups disabled, **Matchmaking still listens**.
4. Host may **resend** lobby announcement because there is no shared persistent online lobby log.
5. Observers allowed **only with complete local chat-history record from matchmaking onward**, never on incomplete reconstructed histories; observers never automatically become players.
6. Start with **Tic-Tac-Toe: solo vs CPU and online two-player**; complete its full architecture before increasing library size.
7. Quality-first progression: **Tic-Tac-Toe → Snake → Checkers → Battleship → Scorched Earth → Texas Hold'em → Creature Arena**; Creature Arena last, using detailed front-only monochrome sprites with runtime player mirroring, attack effects, level-99 six-creature roster and four chosen moves per creature.

