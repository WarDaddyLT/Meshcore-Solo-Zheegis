# Games preview

The first six GDD games are available offline: Tic-Tac-Toe, Snake, Checkers,
Battleship, Scorched Earth and Texas Hold’em. Tic-Tac-Toe also supports two
human players over an existing private MeshCore channel. The current hardware target is the Wio Tracker L1 /
L1 Pro OLED; other boards still need display, input and memory validation.
This is an experimental build, with host simulation coverage for multiplayer;
a match between two physical radios remains a validation requirement.

## Veylith Arena CPU preview

The separate `WioTrackerL1_zygis_veylith` candidate adds the 20-family, 60-form
Trainer and three-creature CPU battles, with manual XP training and 80 moves.
It uses the supplied v2 prototype artwork. See [Arena controls and limits](veylith-arena.md).
PvP Arena is not implemented. The six-game environments below remain available.

## Lightweight production build

For the Wio Tracker L1 / L1 Pro OLED, build:

```sh
pio run -e WioTrackerL1_zygis_games_production
```

This release profile retains all six offline games, multiplayer Tic-Tac-Toe,
tracking, BLE/USB companion support and on-demand screen allocation. USB game
input (`0x79`), game diagnostics (`0x7A`), raw memory probes (`0x7D`), game timing
counters and USB screenshots are omitted. Solo Tools screenshot capture is
therefore unavailable in this profile. No preferences, identity or channels
are embedded in the downloadable firmware.

The build produces `.pio/build/WioTrackerL1_zygis_games_production/firmware.uf2`
for the board's UF2 bootloader and `firmware.zip` for serial DFU. Use the L1 OLED
images for this board; other boards require their own environments. For an
existing L1 with the supported UF2 bootloader, double-press Reset and copy the
UF2 to its boot drive. Application-only updates do not intentionally erase
settings; see [migration notes](zygis_migration.md) when changing firmware families.

Keep `WioTrackerL1_zygis_games_test` for development: it retains the input,
read-only diagnostic, screenshot and timing tools used for game validation.

## Open and play

Build `pio run -e WioTrackerL1_zygis_games`. On the device, open **Tools → Games**,
expand the category and select **Tic-Tac-Toe**. Choose Easy, Normal or Hard,
choose X (first) or O (second), then select **Start game**.

Use Up/Down to select setup options and Left/Right to change them. During play,
use the joystick to select a square and press Enter to place your mark. Menu/Back
opens the pause menu; Enter after a result starts another round. Hard uses an
optimal search; Easy chooses random legal moves, while Normal wins or blocks
immediate threats before choosing a square.

## Snake

Open **Tools → Games → Snake**, select Slow, Normal or Fast with Left/Right,
then select **Start game**. Steer with the joystick. The game rejects immediate
reverse turns; collecting food grows the snake and gradually increases speed.
Walls and body collisions end the round. Filling the board wins.

Menu or Enter pauses. The pause menu offers **Resume**, **Restart** and
**Exit Snake**. Enter after a result restarts; Menu exits. Add Snake through
**Favourites → + → Games → Snake** for quick launch.

Snake has a fixed 30×12 board and a 788-byte rules state. Its separate screen
is allocated only when opened and freed when exited. USB play checks collected
food, paused/resumed, restarted after collision and completed 100 load/unload
cycles. Active game objects occupied 948 bytes; free heap returned from 32,896
to the 33,860-byte home baseline after exit. It uses the shared
framebuffer, respects sound settings and sends no game radio traffic.

## Checkers

Open **Tools → Games → Checkers**, choose Easy or Normal CPU and start. You
control filled pieces; the CPU uses outlined pieces. Select your piece with
Enter, move the cursor to a marked destination and press Enter again. Menu
opens **Resume / Restart / Exit Checkers**. A quick-launch shortcut is available
through **Favourites → + → Games → Checkers**.

This build follows the requested 8×8 rules, overriding the original GDD's English
variant: men move one square forward but capture in all four diagonal directions;
kings move and capture along the entire unobstructed diagonal. Captures and
continuations are mandatory. Reaching the last row promotes immediately, and
capture continues as a flying king. Choose freely among available capture paths;
there is no longest-capture requirement. Already captured pieces remain blockers
until the capture turn ends, then are removed. See the
[IDF capture rules](https://idf64.org/rules-of-the-game/).

No pieces or no legal moves loses. The bounded local draw rule is 80 completed
turns without a capture or promotion. Easy chooses a random legal move. Normal
examines one reply per candidate in small nonblocking steps. No Hard level or
networked Checkers is advertised. Its rules plus CPU occupy 468 bytes, allocated
with the screen only while Checkers is open. A complete on-device CPU game,
pause/restart and 100 load/unload cycles passed. Game objects occupied 632 bytes;
free heap returned from 33,212 to 33,860 bytes after exit. The largest measured
CPU polling step was 977 microseconds.

## Battleship vs CPU

Open **Tools → Games → Battleship**. Choose **Manual** to place ships with the
joystick, hold Enter to rotate, and press Enter to confirm each ship. The fleet
lengths are 5, 4, 3, 3 and 2. Alternatively choose **Auto** to generate a valid
fleet. Review your fleet, then choose **Start**.

Move the cursor on the target grid and press Enter to fire. Each valid shot
changes turn, including hits; repeated shots do nothing. The CPU uses a
hunt/target strategy based only on its known hit/miss grid. Hold Enter to switch
between your fleet and the target grid. Enter on the fleet view returns to the
target grid without firing. Menu offers **Resume / Restart / Exit Battleship**.
The last shot needed to sink all five enemy ships wins.

This is an offline CPU game. Opponent ship positions are hidden until the result;
no fleets, shots or other game data are transmitted. The rules state occupies
412 bytes and is released with the screen when you leave. Battleship can also
be pinned through **Favourites → + → Games → Battleship**. Manual placement,
rotation, a complete CPU game, pause/restart and 100 load/unload cycles passed
on the device. Active game objects occupied 588 bytes; free heap returned from
33,256 to the 33,860-byte home baseline after exit.

## Scorched Earth vs CPU

Open **Tools → Games → Scorched Earth**, choose Easy or Normal CPU, then start.
Up/Down selects angle, power, weapon or Fire. Left/Right adjusts the selected
value; Enter also increments/cycles a value, or fires when Fire is selected.
Standard shells are unlimited; each tank starts with three heavy shells.

Wind affects each shot. Shells follow deterministic fixed-point trajectories;
impacts damage tanks and deform terrain. Falling into a crater adds damage,
and a close shot can damage your own tank. The CPU considers one candidate
trajectory per poll and uses only public terrain, wind and health. Flight is
animated locally without blocking tracking. Menu pauses both flight and aiming.

Reduce the CPU tank's health to zero to win. If neither tank is destroyed after
40 shots, the match is drawn. Enter after the result restarts; Menu exits. It
has a quick-launch entry and sends no game radio traffic. Physics and the CPU
planner occupy 340 bytes, owned only by the active screen. A complete on-device
match, pause during flight, restart and 100 load/unload cycles passed. Active
game objects used 528 bytes; free heap returned from 33,316 to 33,860 bytes after
exit. The largest observed CPU step was 977 microseconds.

## Texas Hold’em vs CPU

Open **Tools → Games → Texas Hold’em** and press Enter to start a heads-up
match with 500 virtual chips each and 5/10 blinds. Up/Down selects Fold,
Check/Call or Raise; Enter performs the selected action. On Raise, Left/Right
changes the total street bet in steps of 10; hold Enter to select the effective
all-in amount, then press Enter to confirm. Raises respect the minimum and both
players’ remaining chips. Uncalled excess is returned; split pots conserve chips.

Your two cards are on the left, the CPU’s hidden cards on the right, and the
community cards below. The top line shows your chips, CPU chips and pot. CPU
cards are revealed only at showdown. The best five of seven cards decide the
winner, including ace-low straights and all kickers. Enter after a result deals
the next hand; after either stack reaches zero it resets the match. Menu opens
**Resume / New match / Exit Holdem**. A quick-launch shortcut is available.

This is a local virtual-chip game, with a simple CPU using only its own cards
and public information. No poker messages or cards are sent over the radio.
The 100-byte rules state and screen are released on exit. All 2,598,960 five-card
hands, seven-card selection, betting streets, heads-up order, all-ins, ties and
hidden-card CPU decisions have host regression coverage. See
[standard hand comparisons](https://www.pokerstars.com/poker/games/rules/hand-rankings/comparing-poker-hands/).

## Play over the mesh

Both devices need this games build and the same existing random-key private
channel. Games does not create, import or edit channels or keys.

The channel picker lists all configured channels that pass the same private-
channel check used by tracking. No extra flag, tracking binding or confirmation
is required. Public, hashtag and known name-derived keys are excluded. Select
which private channel to use; even a single available channel is not selected
automatically. Configure channels through Solo or the companion app.

1. Open **Tools → Games → Multiplayer** on both devices.
2. On one device select **Host game**, then the private channel. The host is X.
   The open lobby announces automatically every second while waiting.
3. On the other select **Find game**. The list shows the host’s configured handle;
   select it and press Enter. Joining and starting are automatic. There are no
   Accept, Ready or Start confirmations. Unrefreshed lobbies expire after five seconds.
4. Move the cursor and press Enter on your turn. Guest moves appear only after
   the host commits them. Menu opens **Resume / Leave match**.
5. After victory, loss or draw, select **Play again** with Enter to play again.
   Either player can request the next round; both boards reset automatically.

The host orders all moves. Session IDs, round numbers and increasing move
sequences reject duplicates and delayed events from earlier rounds. Retries keep
their original IDs; a missing move is never replaced with a guessed board.
Lobby negotiation retries automatically at one-second intervals while open.
Announcements stop when a player joins, and there is no in-match heartbeat.
Gameplay retries remain bounded at three attempts, approximately ten seconds
apart; **Radio timeout → Enter** retries explicitly. Leaving clears the session
and stops all lobby traffic. A lobby supports 256 rounds before a fresh lobby
is needed. Host handles are display labels, not authenticated sender identities.

**Both players need this updated firmware.** The packet format now carries a
round number and uses protocol version 2; earlier game builds are incompatible.

Each event is 52 text characters total, starting with `~G1:`, and is visible
as text to other clients. Group members can impersonate public player IDs;
this is trusted-group play, without signed sender authentication or anti-cheat.
Spectators, persistent replay and background invitations are not implemented.

## Add to quick launch

On the home **Favourites** page, select an empty **+** tile, then **Games**.
Choose **Games menu**, any of the six single-player games, or **Tic-Tac-Toe
mesh**. The shortcut survives reboot. To replace
an occupied tile, hold Enter on it and select **Replace**. The same menu offers
**Unpin**. Pinning a game already on the dial moves its existing shortcut.

Games use the existing six typed favourite slots without extending the
preferences file. This version retains game pins if built without games and
shows them as unavailable. Older firmware predating game pins does not know
the new slot type and may clear them.

## Final six-game build measurements

Direct USB allocator counters on the connected L1 with all six games enabled:

| Visible game | Free heap (bytes) |
| --- | ---: |
| Tic-Tac-Toe | 33,652 |
| TTT multiplayer | 32,684 |
| Snake | 32,840 |
| Checkers | 33,160 |
| Battleship | 33,208 |
| Scorched Earth | 33,272 |
| Texas Holdem | 33,520 |

Home had **33,820 bytes free** and returned to that value after every game exit.
The loop stack watermark remained **2,064 bytes unused** during these checks.
Hold’em used 284 bytes of game objects; fold/raise, showdown, pause, a complete
nine-hand CPU match, reset and 100 allocation/free cycles passed over USB.
CPU timing samples were below the device timer’s approximately 977 µs resolution;
a zero sample is not zero work. The debug firmware uses 535,308 / 708,608 bytes
of application flash. These are measured samples, not worst-case guarantees.

## Implementation and checks

With `SOLO_UI_ON_DEMAND`, optional screens are allocated on entry and destroyed
at the next loop boundary after leaving, once their callbacks have returned.
Only the visible game owns its state; multiplayer storage is allocated only
when Multiplayer is opened. CPU search and network polling allocate nothing.
The display framebuffer is shared and offline play sends no game radio traffic.

Žygis tracking, mesh communications, identity and channel configuration remain
resident. Local chat histories are disposable RAM buffers; leaving Messages
clears them and stops its UI-managed DM retries. The phone's mesh message queue
is separate and continues working without loading a local chat screen. A BLE
connection closes Games and gives the companion app priority. Local stopwatch
state resets when its screen is unloaded; alarms/countdown use the existing
background timer engine. No persistent chat storage is added.

The first cleanup measurement showed **33,860 bytes free heap**, versus **3,868
bytes** before cleanup, with identity/radio/channel/tracking fingerprints
unchanged. These are runtime samples, not a worst-case guarantee.

Run the `test/games/test_*.py` scripts for exhaustive rules/AI checks,
two-screen multiplayer simulation, packet loss/deduplication, 128×64 layout,
quick-launch slots, private-channel filtering and screen lifecycle regressions.
The `WioTrackerL1_zygis_games_test` profile additionally enables USB game-input
debugging; it remains available while the library is being validated. Native
firmware suites and tracking regressions pass. USB input/screen checks supplement
physical joystick, audio and radio coexistence testing; those field checks are
still required. Creature Arena and multiplayer for the other games are excluded.
