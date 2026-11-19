# Veylith Arena CPU preview

Veylith Arena is an offline three-creature team game for the family firmware.
It uses the [v1.0 design](design/Veylith_Arena_GDD_v1.0.md) and the supplied
[v2 artwork](design/Veylith_Arena_Assets_v2/README.md). This increment implements
Trainer and CPU battles. PvP, matchmaking and spectators are not implemented.
The existing private-channel Tic-Tac-Toe multiplayer remains available.

## Build and open

```sh
pio run -e WioTrackerL1_zygis_veylith
```

The validated display target is the Wio Tracker L1 / L1 Pro 128×64 OLED.
Other boards and e-ink need their own layout/input validation. Open
**Tools → Games → Veylith Arena**, or pin it through **Favourites → + → Games**.
The environment extends the channel-only family profile and omits USB game
input, diagnostics, memory probes, screenshots and timing counters.

## Trainer and battle controls

- **Left/Right in Trainer:** choose one of three team slots. **Up/Down:** select
  Team, Train, Moves or Ready; **Enter** opens it. Context opens short help.
- **Team:** select any of 20 families. The same family cannot occupy two slots.
  All 60 forms have front art mirrored at render time for the player; forms evolve at levels 15 and 35. Right or Context in Team opens a creature card; Left/Right previews forms and Enter opens its short description.
- **Train:** choose HP, ATK, DEF, FOC, WARD or SPD, review the cost and confirm
  Yes. Every three purchases advance one level, up to 99. No automatic spending.
- **Moves:** choose a slot, inspect an eligible move and press Enter to equip.
  Swaps are free. The available pool requires matching elements, minimum level
  and permanent trained stats; four is the equipped-slot limit. An asterisk marks
  a locked affiliated move; duplicates are rejected. Left/Right in a move card
  cycles overview, short description, exact effect and the level/stat checks.
  There are 80 moves across elemental attacks, healing, guards and support.
  Temporary battle effects never grant or remove move access.
- **Ready:** freezes the team and discloses the generated CPU lineup and
  difficulty. Left/Right inspects both teams; Enter cycles stat/loadout pages,
  Up/Down changes member, Back returns. Enter on Ready starts the battle.
- **Battle:** Enter opens Moves, free Plain Strike, Switch or Team status.
  Context in the battle move picker opens read-only move descriptions. Targeted support asks for a teammate. Unavailable actions do not spend a turn.
  A fainted active creature needs a free replacement. Back opens pause/abandon.
- **Result:** all three participating families earn XP, including on defeat.
  Enter returns to Trainer for another match. Abandoning earns no XP.

The CPU chooses from public state before the player's action. Its levels follow
the median trained team level within two levels; three consecutive losses use
Novice behavior. Bench regeneration, energy/cooldowns, shields, statuses,
once-per-team revival tokens and the round-41 pressure/round-60 cap are bounded
by the rules engine. CPU games send no game radio packets.

## Storage, assets and validation

Progress uses a 437-byte, versioned, little-endian CRC record at `/veylith1`,
written to a temporary file before atomic replacement on the L1. It is separate
from identity, channel keys, radio preferences, tracking and GPS trail storage.
Save errors keep the current profile intact; an unreadable save requires explicit
backup-and-reset confirmation. Results use monotonic IDs to prevent duplicate
awards on save retry. Closing Arena frees its profile, screens and battle state;
tracking and the GPS trail remain resident.

The complete sprite/UI data is 10,280 bytes of const flash, with no extra
framebuffer. `tools/veylith/generate_assets.py` checks every PNG, species mapping,
front-only storage and generated C++ export. It preserves MSB-first data for the
OLED driver's bitmap function. The asset README's front-crop warnings still apply: this is prototype artwork, not a hand-approved final asset set.

```sh
python3 tools/veylith/generate_assets.py --check
python3 test/veylith/test_veylith.py
```

Host checks cover progression/evolution, all 80 execution paths, save corruption,
screen bounds, failure recovery and 10,000 deterministic CPU-vs-CPU matches.
These do not replace the GDD's longer physical soak, human balance review or
future multi-device PvP gates. Developer-only
`WioTrackerL1_zygis_veylith_test` uses `/veytest1` instead of the real profile and
retains temporary USB verification tools; do not distribute it as production.
