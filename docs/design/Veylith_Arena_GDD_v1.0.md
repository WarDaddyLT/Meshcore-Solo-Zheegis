# VEYLITH ARENA — Original 1-Bit Creature Combat Game

**Game Design Document & Codex Implementation Specification · v1.0 · 15 November 2026 · front-only artwork amendment**

**Product:** an original, battle-only, three-monster tactical arena RPG embedded in **MeshCore Solo**. **No names, artwork, audio, story, monsters or other proprietary assets.** First-generation handheld *presentation* is a visual benchmark; the actual display is **strict black/white 1-bit**, not the four-shade palette.

**Primary target:** Seeed Studio **Wio Tracker L1 Pro OLED** (128×64, nRF52840). The L1 Pro **E-Ink variant is a separate optional port**: its 122×250/250×122 display and slow refresh are not to be treated as a 128×64 OLED.

**Scope:** single-player versus adaptive CPU first; then 2-player private-channel PvP with optional complete-history spectator replay, using the pre-existing Solo game framework and channel manager.

**This document supersedes the old ‘Creature Arena’ section in `MeshCore_Solo_Game_Library_GDD_v1.1.md`**, which assumed six monsters, all level 99, and no levelling. Do not let the older section silently override this document.

**Codex instruction:** Inspect the checked-out MeshCore Solo tree, platform definitions, existing Games framework, actual channel/chat APIs, assets layout, persistence and linker map before implementing; do not invent classes or pin mappings. Implement in milestones and leave production build flags disabled until tests and memory headroom pass.

**Artwork amendment (15 November 2026):** store front art only. The player uses
a horizontal mirror at render time. This overrides all earlier separate-rear
requirements and saves 7,680 raw bitmap bytes. Canonical PNGs live in
`Veylith_Arena_Assets_v2`; duplicate atlases/binary/header exports are removed.

## 1. Locked player requirements (authoritative)

1. Exactly **5 elements**, with **20 distinct ordered two-element combinations** (`5×4`; never repeat an element within a creature). **Primary and secondary order matters.**
2. Exactly **20 original evolution families, each with 3 stages = 60 original monsters**. Two evolutions per family (stage I→II→III). No world, catching, items or campaign.
3. Each player builds **one team of 3 monsters**, with **one active at a time** and two bench members. Every battle is preceded by a **Trainer** screen.
4. Each monster has **4 equipped move slots**, picked from the larger pool eligible by **level, elements and permanent trained stats**; **move swapping is always free** in Trainer.
5. Battles award **XP on both wins and losses**. XP is a **spendable per-monster currency**; choosing a stat in Trainer **consumes XP and buys a permanent stat-training rank**.
6. **Three stat purchases advance one creature level**, with explicit **0/3, 1/3, 2/3** level-progress UI. Levels unlock more move choices; evolution occurs automatically at milestones. Level cap **99**.
7. Combat must have real tactical choices: **physical/mystic offense, guard, shield, heal self/other/all, revive self/other/all, buffs, debuffs, status cleansing, resource restoration, counters, switching and elemental synergy**. No usable inventory items, even during battle.
8. CPU strength tracks the player's **actual trained team**, while CPU behaviour improves within readable/fair limits (no peeking at unrevealed choices; no hidden stat cheating).
9. On-device visuals: readable **front sprites for all 60 forms, mirrored horizontally for the player**, tactical menus, HP/energy bars, and purposeful local attack/hit/faint/evolution animations; no networked animation frames.
10. All game-related multiplayer packets, from lobby opening through final result, go through **existing eligible random-key private MeshCore channels only**; Games never creates, imports, edits, or deletes channels.
11. Host announces an open lobby as a recognizable **special chat message**; compatible devices can show an optional popup. With popups disabled, **Games → Matchmaking** still discovers repeated announcements. Invitations do not equal acceptance or automatic joining.
12. Observer/spectator may render a match **only if local private-channel chat history contains the complete contiguous authoritative event log including the original matchmaking/lobby-opening event**. No incomplete-history bypass via snapshot.

## 2. Player loop and game identity

**Loop:** Games → Veylith Arena → Select CPU Battle / Host PvP / Matchmaking → **Trainer** → Ready → Battle → Outcome (win or loss) → XP granted to the three participants → Trainer → spend XP on chosen stats → level up / evolve / unlock new moves → freely revise four moves → next battle.

There are **20 base forms available from day one** (one per ordered elemental pair). The player may select any three distinct family slots; additional families are not hidden behind chance, purchases, capture, or inventory. Training belongs to the individual family; evolving replaces its displayed species but **does not reset its stats, XP, unlocked moves or training history**. Stages II and III are earned by growth; there are only three forms per family.

Save-slot scope for v1: **one player profile** with progress for 20 families, one current three-family lineup, and four equipped moves per family. Optional preset teams are a later addition. Solo and PvP use the same trained rosters; PvP lobbies show an approximate level-range warning before READY if levels are very different.

**Non-goals:** elemental collection gacha, exploration, NPC dialogue, breeding, IV/EV lotteries, equipment, consumables, monetization, live-service backend, streaming animation over LoRa, six-monster battle parties.

## 3. Hardware, performance and 1-bit visual direction

- **nRF52840 Cortex-M4F @ 64 MHz**, **1 MB internal flash / 256 KB RAM total** including Solo and platform reserves. Do **not** treat all flash/RAM as available to games.
- Primary panel **1.3-inch 128×64 monochrome OLED**; 1,024 bytes for a 1-bit framebuffer. Reuse Solo's screen buffer/renderer; avoid allocating another full frame without profiling. Four-direction joystick + enter/back are mapped via existing Solo input APIs, not board-specific hard-coded GPIO.
- A Game Boy-like visual *language* means **strong creature silhouettes, bold 1-pixel contours, pixel patterns, depth by stippling or hatching**, expressive front silhouettes with horizontal player mirroring and deliberate motion. No grayscale/color reliance; avoid dense dithering on the physical OLED.
- **Sprite master:** one front sprite for each of 60 creatures, design at **32×32px** (128 B per 1-bit bitmap), test 28×28 and cropped dynamic windows on hardware. 60 raw bitmaps ×128 B = **7,680 B**, before animation accents/compression. Mirror the front horizontally at render time for the player; do not store a second view or allocate a flip buffer. **No placeholder sprites in a production release**.
- Sprite design: stage I cute compact recognizable silhouette; stage II readable adaptation; stage III striking anatomy. The mirrored player view must preserve the exact front silhouette. Create a **20-family visual-style bible** from §5 descriptions. Creatures must be visibly distinguishable in 1-bit at true physical size.
- Attack animations share a small effect library: frame-shift/recoil, inverse-flash, 4–8 particle templates, projectile travel, slash lines, circular pulse, short screen rumble, health bar stepping, send-out/faint (mask/wipe), automatic evolution reveal. **Only the host/owner locally animates; RF carries game events, not frames.**
- Target local game render cadence **20–30 FPS where practical**, nonblocking; maintain radio/USB/BLE event service. The actual panel update rate is implementation-dependent; profile on board.
- E-Ink optional port: 122×250 portrait or 250×122 landscape, slow partial refresh, text-first battle screens and sparse updates; disable animated frames there, do not waste time optimizing for it before the OLED version ships.
- Audio: short optional buzzer chimes via Solo sound preferences; no samples or melodies from existing games.

### 3.1 Pixel-exact OLED layout rules

**Battle VIEW** (128×64 canvas):

| Region | Rectangle `x,y,w,h` | Content |
|---|---|---|
| Opponent header | `0,0,90,9` | 10–12-char name truncation and level |
| Opponent HP bar | `43,10,45,4` | Solid HP fill + outline |
| Enemy front sprite | `94,10,32,32` | Front/side facing left; clip to region |
| Your mirrored front sprite | `2,22,32,32` | Front mirrored horizontally to face enemy right; no rear asset |
| Your status | `38,42,88,13` | Name, HP, energy or status icon; 2 compact rows |
| Message strip | `0,56,128,8` | Action result / turn indicator, paginated |

Full-screen screens instead of overlapping HUD: **MOVE (2×2 slots with name/cost), TEAM (three entries, HP/status), TRAINER (roster, stats, XP, level progress), MOVE POOL (scroll), SUMMARY/RESULTS, HELP**. The position table is a design target; if real 6×8 font clipping makes it illegible, adjust layout and document measured change.

### 3.2 Input map

| Input | Battle | Trainer | Dialog |
|---|---|---|---|
| Joystick Up/Down | Select menu action / scroll | Select family/stat/move | Select option |
| Left/Right | Switch focus / four-slot grid | Switch team slot or tab | Select Yes/No |
| Select/Enter | Confirm action | Purchase stat (confirm) / equip move | Confirm |
| Back | Back one page; no lost turn | Discard *uncommitted* menu choice only | Dismiss/cancel |
| Long Select (if supported) | Quick status/info | Inspect unlock preview | Details |

Purchases become durable **only after a confirmation**; move swaps require no XP confirmation. Every screen must be usable with a four-way joystick and current Solo Back/Enter behaviour.

### 3.3 Sample 128×64 battle flow (design text, not final artwork)

```text
[VEY LITH ARENA] [CPU LV 16]     (splash / menu)
ENEMY Lv16    HP [=====---]       (battle header)
            [32x32 FRONT]        (opponent region)
[32x32 MIRRORED FRONT]                      (player region)
YOUR Lv15     HP 64/87  E 8      (bottom status)
[MOVE] [TEAM] [STATUS]           (actions page)
```

The ASCII example is thematic, **not** a claim that all those long strings fit simultaneously. Rendering must be verified against the 128×64 pixel rectangles above.

## 4. Five-element system, all 20 ordered combinations

Elements: **E Ember**, **T Tide**, **N Thorn**, **S Stone**, **K Spark**. Each monster has **two different elements** with an explicit **primary** and **secondary** order. The primary affects move affinity; secondary affects defense and move access. Both elements are used for defense.

For a balanced five-type metagame, use a cyclic wheel: each element is strong against the next two on the wheel, weak against the previous two. **Wheel order: `E → N → S → K → T → E`.** This deliberately chooses symmetry and game balance over literal real-world chemistry.

- Attacker strength versus a single defending element: **×1.20**. Weakness: **×0.8333**. There are no completely neutral different-element pairings on the five-element ring.
- Against two defending elements, multiply both factors, then clamp total to **0.70–1.44**; calculate in **fixed-point integers**, not float.
- Move of attacker's **primary** element: ×1.15 affinity; **secondary** element: ×1.08; universal neutral attacks ×1.00. No same-element double-type exists.
- Status/heal/guard moves do not gain raw type damage affinity unless their individual effect explicitly says so.

### Elemental attack multiplier chart (attacking row → defending column)

| Attacker \ Defender | Ember | Thorn | Stone | Spark | Tide |
| --- | --- | --- | --- | --- | --- |
| Ember | 1.00 | 1.20 | 1.20 | 0.8333 | 0.8333 |
| Thorn | 0.8333 | 1.00 | 1.20 | 1.20 | 0.8333 |
| Stone | 0.8333 | 0.8333 | 1.00 | 1.20 | 1.20 |
| Spark | 1.20 | 0.8333 | 0.8333 | 1.00 | 1.20 |
| Tide | 1.20 | 1.20 | 0.8333 | 0.8333 | 1.00 |

Example: Ember move versus Thorn/Stone has multiplier `1.20 × 1.20 = 1.44`; Ember versus Tide/Spark = `0.8333 × 0.8333 ≈ 0.694`, clamped to 0.70. Ember versus Thorn/Tide ≈ 1.00. Primary/secondary **order creates 20 distinct strategic family identities**, even when defending dual-element pairs are the same unordered combination.

## 5. Complete original creature roster: 20 families / 60 forms

All names and silhouettes below are **original game-design proposals**; review for name collisions/trademarks before public release. The first form is selectable initially; stage II appears automatically at **Level 15**, stage III at **Level 35**. **Every one of the 20 ordered pairs occurs exactly once; no self-pair exists.**

Base stats listed here are **Stage I, Level 1, zero training**, before staged bonuses or purchased training. Column meanings: `HP/ATK/DEF/FOC/WARD/SPD`. Individual species are deliberately differentiated by their ordered type pair and intended role.

| ID | Primary/Secondary | Stage I | Stage II | Stage III | Base stats | Playstyle |
| --- | --- | --- | --- | --- | --- | --- |
| 01 | Ember/Tide | Vapkit | Vaprowl | Thermwyrm | 37/14/10/13/10/12 | Balanced caster / delayed burn |
| 02 | Ember/Thorn | Cindseed | Briarsmoke | Pyrocanopy | 35/14/10/15/10/12 | Damage-over-time / ally sustain |
| 03 | Ember/Stone | Flintub | Kilnscar | Furnacore | 34/14/13/13/10/11 | Armoured striker / counter |
| 04 | Ember/Spark | Flickit | Arcfang | Voltflare | 34/15/10/14/9/15 | Fast burst / disruption |
| 05 | Tide/Ember | Brinlet | Scaldusk | Geyzard | 37/13/11/14/13/10 | Healing / pressure |
| 06 | Tide/Thorn | Kelprit | Reefveil | Thalorin | 38/11/11/15/14/9 | Team healer / cleansing |
| 07 | Tide/Stone | Siltclaw | Breakshell | Tidebastion | 37/11/14/13/14/8 | Tank / shield transfer |
| 08 | Tide/Spark | Pipvolt | Surgewake | Tempestide | 37/12/11/14/13/12 | Speed control / siphon |
| 09 | Thorn/Ember | Spriglow | Briarfox | Wildpyre | 35/13/12/15/12/11 | Support / counter-pressure |
| 10 | Thorn/Tide | Reedpip | Mirehowl | Fenmantle | 38/11/12/14/13/10 | Regeneration / endurance |
| 11 | Thorn/Stone | Rootling | Barkhorn | Groveantler | 35/11/15/14/13/9 | Bulwark / status |
| 12 | Thorn/Spark | Vinetip | Vezclaw | Stormthicket | 35/12/12/15/12/13 | Tempo / debuffs |
| 13 | Stone/Ember | Ashnub | Moltusk | Basaltrum | 40/15/15/10/12/8 | Heavy offense / grit |
| 14 | Stone/Tide | Shellit | Reefram | Tectavern | 43/13/15/9/13/7 | Barrier / recovery |
| 15 | Stone/Thorn | Gravseed | Mossolith | Verdolith | 41/13/15/11/13/7 | Defensive team anchor |
| 16 | Stone/Spark | Ferrichip | Ferrogeist | Magnetern | 40/14/15/10/12/10 | Counter / control |
| 17 | Spark/Ember | Zippup | Voltcrest | Aetheroc | 32/14/9/16/10/16 | Fast offense / burn |
| 18 | Spark/Tide | Luminip | Arcshoal | Stormskate | 35/12/9/15/11/15 | Siphon / tactical switch |
| 19 | Spark/Thorn | Blinkbud | Staticole | Voltloom | 33/12/9/17/11/15 | Battery / healing |
| 20 | Spark/Stone | Coillet | Sparkplate | Dynacarapax | 32/12/12/15/11/14 | Guard-breaking / toughness |

### 5.1 Visual silhouette bible (mandatory for original sprite production)

Sprites must differ between forms in silhouette, anatomy and pose, not merely by adding spikes. Below is the visual direction **for every family**; create one front view of each named stage, 60 stored images total; mirror at render time for the player. Stage I = small/simple; Stage II = transitional; Stage III = full dramatic form, preserving recognizable family features.

| Family | All-stage silhouette direction |
| --- | --- |
| 01 — Vapkit → Vaprowl → Thermwyrm | Steam salamander; back fins and spiralling condensation horns |
| 02 — Cindseed → Briarsmoke → Pyrocanopy | Ember-seed moth becoming a blazing antlered tree-moth |
| 03 — Flintub → Kilnscar → Furnacore | Forge beetle with obsidian plates and a glowing furnace carapace |
| 04 — Flickit → Arcfang → Voltflare | Plasma jackal with branching lightning tails |
| 05 — Brinlet → Scaldusk → Geyzard | Boiling marsh lizard with vertical steam fins |
| 06 — Kelprit → Reefveil → Thalorin | Kelp-winged ray with dangling seed tendrils |
| 07 — Siltclaw → Breakshell → Tidebastion | Coral crab expanding into a walking reef citadel |
| 08 — Pipvolt → Surgewake → Tempestide | Electric eel with a lightning-braid dorsal crest |
| 09 — Spriglow → Briarfox → Wildpyre | Seed-tailed fox whose mane becomes a burning vine crown |
| 10 — Reedpip → Mirehowl → Fenmantle | Pond frog becoming a reed-cloaked bog guardian |
| 11 — Rootling → Barkhorn → Groveantler | Bark-shelled deer with branching stone-root antlers |
| 12 — Vinetip → Vezclaw → Stormthicket | Charged vine mantis becoming a thunder-garden sentinel |
| 13 — Ashnub → Moltusk → Basaltrum | Lava ram with strata horns and cooling armour |
| 14 — Shellit → Reefram → Tectavern | Pebble-shelled amphibian becoming a tidal mesa turtle |
| 15 — Gravseed → Mossolith → Verdolith | Rolling mineral seed growing into an enormous rooted monolith |
| 16 — Ferrichip → Ferrogeist → Magnetern | Iron-feathered owl with ring-shaped magnetic wings |
| 17 — Zippup → Voltcrest → Aetheroc | Lightning fledgling growing into a storm raptor |
| 18 — Luminip → Arcshoal → Stormskate | Electric lantern fish developing glider fins |
| 19 — Blinkbud → Staticole → Voltloom | Luminous pollinating insect with sparking petal wings |
| 20 — Coillet → Sparkplate → Dynacarapax | Spiral-armoured armadillo with conductive mineral plates |

### 5.2 Stats and evolution definitions

- Six trainable stats: **HP**, **ATK** physical offense, **DEF** physical mitigation, **FOC** mystic offense/healing, **WARD** mystic mitigation, **SPD** initiative.
- At Stage I, Level 1, zero training, use base table above. For stage II add **`+14 HP, +4 ATK/DEF/FOC/WARD, +3 SPD`**; for stage III (versus stage I) add **`+30 HP, +9 ATK/DEF/FOC/WARD, +7 SPD`**. Bonuses are absolute, not compounded, and do not alter training ranks.
- A purchased training rank adds: **HP +3 max HP**, each other stat **+1**. HP gained from training/evolution is added to current HP in battle only when constructing battle state; out-of-battle there is no injured roster.
- Each stat has a **training rank 0–80** cap. Full Level 99 requires 294 total rank purchases, which is achievable across six stats (capacity 480). Unspent XP never changes levels or stats by itself.
- Stage is **derived from level**; the eligible move pool is derived from **level, ordered elements and permanent trained stats**. Do not store redundant editable level/stage. Clamp all arithmetic, avoid integer wrap, and validate migrated saves.

### 5.3 Creature field notes

Each named form has its own short description, shown in the in-game creature card.

| Species | Monster | Description |
| --- | --- | --- |
| S01 | Vapkit | A little salamander puffing warm steam rings. |
| S02 | Vaprowl | A finned hunter hiding its strikes in hot mist. |
| S03 | Thermwyrm | A steam wyrm crowned with spiralling horns. |
| S04 | Cindseed | A glowing seed-moth carrying a tiny ember. |
| S05 | Briarsmoke | A smoky moth trailing thorny branches. |
| S06 | Pyrocanopy | An antlered tree-moth sheltering a living flame. |
| S07 | Flintub | A forge beetle wrapped in fresh obsidian. |
| S08 | Kilnscar | A scarred beetle hardening its furnace shell. |
| S09 | Furnacore | A walking furnace behind thick volcanic plates. |
| S10 | Flickit | A restless jackal cub with a crackling tail. |
| S11 | Arcfang | A swift jackal snapping with charged fangs. |
| S12 | Voltflare | A plasma hunter with branching lightning tails. |
| S13 | Brinlet | A marsh lizard warming its shallow pool. |
| S14 | Scaldusk | A sturdy lizard venting scalding steam. |
| S15 | Geyzard | A geyser guardian with towering steam fins. |
| S16 | Kelprit | A tiny ray drifting beneath kelp fronds. |
| S17 | Reefveil | A veiled ray tending its seed tendrils. |
| S18 | Thalorin | A great kelp-winged ray that shelters its team. |
| S19 | Siltclaw | A coral crab carrying a coat of silt. |
| S20 | Breakshell | A broad crab bracing its living reef shell. |
| S21 | Tidebastion | A walking reef citadel built to protect allies. |
| S22 | Pipvolt | A bright eel storing sparks in its crest. |
| S23 | Surgewake | A quick eel riding its own charged wake. |
| S24 | Tempestide | A storm eel crowned by braided lightning. |
| S25 | Spriglow | A seed-tailed fox warming new shoots. |
| S26 | Briarfox | A thorn-maned fox guarding a hidden ember. |
| S27 | Wildpyre | A wild fox wearing a crown of burning vines. |
| S28 | Reedpip | A pond frog hiding among young reeds. |
| S29 | Mirehowl | A reed-cloaked frog calling through the mire. |
| S30 | Fenmantle | A patient bog guardian beneath a living cloak. |
| S31 | Rootling | A young deer sheltered by rough bark. |
| S32 | Barkhorn | A sure-footed deer growing stony antlers. |
| S33 | Groveantler | A forest guardian with branching root antlers. |
| S34 | Vinetip | A small mantis testing its charged vines. |
| S35 | Vezclaw | A vine mantis striking with electric claws. |
| S36 | Stormthicket | A thunder-garden sentinel with living blades. |
| S37 | Ashnub | A young lava ram with cooling stone horns. |
| S38 | Moltusk | A fierce ram wrapped in hardened magma. |
| S39 | Basaltrum | A heavy ram armoured in volcanic strata. |
| S40 | Shellit | A shy amphibian beneath a pebble shell. |
| S41 | Reefram | A reef-backed turtle holding firm in the tide. |
| S42 | Tectavern | A tidal mesa turtle carrying a rocky refuge. |
| S43 | Gravseed | A rolling mineral seed seeking soft earth. |
| S44 | Mossolith | A mossy monolith beginning to take root. |
| S45 | Verdolith | An ancient rooted stone guarding its allies. |
| S46 | Ferrichip | A small owl shedding bright iron feathers. |
| S47 | Ferrogeist | A watchful owl shaping magnetic wing rings. |
| S48 | Magnetern | A silent iron owl commanding a magnetic halo. |
| S49 | Zippup | A restless fledgling sparking at the wings. |
| S50 | Voltcrest | A keen storm bird with a blazing crest. |
| S51 | Aetheroc | A mighty raptor diving through charged clouds. |
| S52 | Luminip | A little lantern fish flashing in the dark. |
| S53 | Arcshoal | A bright fish learning to glide on charged fins. |
| S54 | Stormskate | A storm glider skimming above the waves. |
| S55 | Blinkbud | A glowing pollinator resting in a closed bud. |
| S56 | Staticole | A darting insect scattering static pollen. |
| S57 | Voltloom | A bright flower-winged insect sharing energy. |
| S58 | Coillet | A tiny armadillo curled into a charged coil. |
| S59 | Sparkplate | A hardy armadillo growing conductive plates. |
| S60 | Dynacarapax | A living dynamo behind a spiral mineral shell. |

## 6. Trainer, XP economy and level unlocks (CORE REQUIREMENT)

**Trainer must appear before *every* battle**, CPU or PvP. It is not a one-time tutorial. The player may change the three-monster lineup; inspect species/role/type; view detailed stats; **spend available XP to train one stat rank**; and freely replace any of the four equipped skills from the currently unlocked eligible move pool.

### 6.1 XP storage and spending

Per evolution **family**, store: `xp_wallet` (unspent), `xp_lifetime_earned` (analytics), `train_rank[6]`, `equipped_move_id[4]`, and optional progress flags. A family only evolves; it is not duplicated as three simultaneous roster entries. XP is never shared globally, and losing a battle never removes XP.

Training purchase cost in XP for selected stat `s` (rank `r`) and current creature `L`:

`costXP(s) = 30 + 4×r + 2×(L−1)`  (integer, bounded; rank up from r to r+1).

On confirmation: if **all** conditions hold (`xp_wallet >= cost`, `r < 80`, `L < 99`, `sum(train_rank) < 294`), subtract XP and increment that stat rank once; persist atomically. **No auto-training.** Show exact cost and resulting stat before confirmation.

**Level is calculated as** `L = min(99, 1 + floor(sum(train_rank[6]) / 3))`. Show level progress `sum(ranks) mod 3` towards the next level (e.g., `LV 12 • 2/3`). Level increments exactly when the third, sixth, ninth ... cumulative stat training purchase is made. At level 99, further training is disabled for v1 to avoid divergent cap behavior; XP earned thereafter updates lifetime-XP only, not an unspendable wallet: **maximum 294 purchased ranks total**; choose purchases deliberately.

**Rationale:** The player spends XP *on stats* and levels are *unlocked by actual stat investment*. There is no separate hidden level-XP bar. Levels expand the move pool and trigger evolutions, so training choices matter. The move list itself is always free to rearrange.

### 6.2 Battle XP awards to all three selected monsters

Base per-monster XP for a completed battle (no XP for aborted/desynced matches):

`XP_base = 60 + 8 × clamp(opponent_average_level, 1, 99)`

- **Winner:** each selected family gains `floor(XP_base × 1.00)`.
- **Loser:** each selected family gains `floor(XP_base × 0.45)` (minimum 20); this includes fainted members so players always progress after a fair loss.
- **Draw at hard turn cap:** each family gains `floor(XP_base × 0.60)`.
- **CPU battles:** same as PvP, no hidden reduction. If player level is much higher than opponent average, apply `XP_base *= max(0.25, 1 − 0.08 × (player_avg_level−opponent_avg_level))` with integer fixed-point to discourage intentionally weak CPU farming.
- **PvP:** same earning rules; prevent obvious repeat-loop farming by a bounded last-8 match/opponent cache, decreasing rewards after repeated immediate rematches. This is a **casual local profile**, not cheat-proof rankings or secure server validation. Do not penalize normal fair PvP without clear rules on device.
- XP is awarded **once** per confirmed `MATCH_END`/CPU result ID; idempotent after reboot/retry. No XP from mere lobby joins or disconnected uncompleted games.

**Example:** Opponent mean level 10 → `XP_base=140`. A victory yields +140 XP to each of your three families; a defeat yields +63 XP each. A Level 1 family with no training can buy the first HP rank for 30 XP (+3 HP), or ATK rank for 30 XP (+1 ATK); after any total three purchased ranks it becomes Level 2 and unlocks level-2 options if defined. A move change from one unlocked slot to another always costs **0 XP**.

### 6.3 Trainer UI contract

```text
TRAINER      TEAM 1/3
VAPKIT         LV 6
XP 00480  NEXT 1/3
HP 45   ATK 18
[STATS] [MOVES]
[TEAM]  [READY]
```

**Screens:** ① Select 3 family slots; ② Monster detail (front/mirrored-front preview, elemental glyphs); ③ Stat Training (6 rows, current effective stat, rank, next value, XP cost, progress `0/3`); ④ Move Loadout (4 equipped slots); ⑤ Move Library (eligible unlocked moves, sorted by role and level); ⑥ Battle Ready (team, levels, move summaries, channel/lobby rules).

**Trainer usability tests:** first-time player can buy +HP with XP, see wallet decrease and `1/3`; buy two more ranks, see Level 2 and move-unlock notification if any; equip an unlocked move at 0 cost; swap bench lineup; exit/re-enter and see exactly the same progression. At Level 15/35, verify correct form/sprites and new move choice.

### 6.4 Move pool versus four equipped slots

**Every monster has exactly four equipped move slots.** Four is the loadout
limit, not the size of its available move pool. That larger pool is derived
from all three conditions below, joined with **AND**:

1. **Element:** the move is universal or matches the creature's primary or
   secondary element. Primary/secondary order affects affinity, not eligibility.
2. **Level:** the creature has reached the move's minimum level.
3. **Permanent stats:** every stat gate in the move table is met. Compare the
   creature's base + purchased ranks + current evolution bonus. Ignore battle
   buffs/debuffs, status, current HP and energy; they cannot unlock or unequip moves.

`ATK>=16 & FOC>=20` means both thresholds are required. `None` means no additional
stat gate; level and element still apply. Training HP alone cannot unlock a
high-level attack without its offensive stat. High ATK cannot bypass level or
an unrelated element. Two creatures of the same level can have different pools.

- Choose any four **distinct** currently eligible moves; swapping costs zero XP.
  There is no four-move lifetime learning cap and no permanent forgetting system.
- The two elemental starters and universal Plain Strike, Mend Self, Guard and
  Steady Breath have no stat gates. Every fresh creature has six choices for four
  slots. Plain Strike is also always offered in battle, outside those four slots.
- Preview affiliated locked moves with their exact level/stat requirements;
  mark them locked and reject equipping until all conditions pass. Sort by role,
  then level and ID. The move card shows description, effect and a requirements page.
- Stat training can unlock a move even without gaining a level; evolution bonuses
  can also meet a gate. Training feedback lists newly unlocked moves. No auto-equip.
- READY freezes ranks, derived stats and the four chosen move IDs. The same gate
  validator applies to save/load, CPU builds and later PvP snapshots.
- Family cards show every form's brief description and front sprite; locked future
  forms are previews. They cannot modify a creature's actual stage.

### 6.5 Initial balance policy

These are the first tuning values, not a claim of completed human balance testing.
Elemental attack tiers share an energy/cooldown budget: starters 22 power/E1;
level-4 proc attacks 32/E3; level-9 armour piercers 50/E6/CD1; late finishers
64-66/E9/CD2. Guaranteed status, drain, shield pressure and priority trade raw
power for utility. No paid attack replaces the free fallback in every matchup.

Physical access uses ATK, mystic damage/healing uses FOC, shielding uses DEF or
WARD, and tempo uses SPD. High-impact revival, silence and counters may require
two stats. Gates are attainable by every affiliated family at the listed minimum
level within the `3*(level-1)` rank budget; none requires a rank above 80. They
create deliberate builds without spending extra XP to learn or change a move.

Revival remains bounded by its team/creature tokens, energy and four-slot
opportunity cost. Passive Anchor occupies a slot and cannot be manually cast.
Round-41 healing pressure and the round-60 cap prevent indefinite support loops.
Validate all families and gates, then run seeded role/level sweeps and human
matches before claiming final competitive balance.

## 7. Full move pool: exactly 80 choices

Each move has a stable numeric ID, primary required element (`U` means universal), role, attack power, ENERGY cost, cooldown in *that monster's activations*, target, minimum level, permanent-stat requirements, a short description and an effect. Gameplay and serialization refer to **IDs**, not localized move strings. Names should fit a scrolling 128px menu; longer names may truncate on the battle move-picker without changing ID.

**Targets:** `EN` active enemy; `SELF`; `ALLY` one living bench ally (not self); `FAINT` one fainted bench ally; `TEAM` all three allies subject to alive/faint semantics; `TEAM_FAINT` all fainted allied members.

**Attack classes:** `PHY` uses ATK vs DEF; `ARC` uses FOC vs WARD. `HEAL`, `GUARD`, `BUFF`, `DEBUFF`, `CLEAN`, `REVIVE` use their stated effect, not attack power. Accuracy is **100%** for v1 except explicit status rolls, reducing needless RNG frustration on a limited screen. `0` in power means non-damage effect. Move cost always paid on accepted action, cooldown begins on activation.

### 7.1 Elemental offensive moves: 40 (8 per element)

#### Ember (E)

| ID | Move | Class | Pow | EN | CD | Lv | Effect | Stat gate | Description |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Cinder Tap | PHY | 22 | 1 | 0 | 1 | Plain hit | None | Flick a small ember to test a rival. |
| 2 | Coal Lash | PHY | 32 | 3 | 0 | 4 | 10% Burn | ATK>=16 | Lash the foe with a burning coal. |
| 3 | Kiln Crash | PHY | 50 | 6 | 1 | 9 | Pierce 20% DEF | ATK>=20 | Drive a furnace-hot blow through armour. |
| 4 | Ember Lance | ARC | 34 | 4 | 0 | 15 | 20% Burn | FOC>=24 | Hurl a lance of searing embers. |
| 5 | Ash Spiral | ARC | 27 | 5 | 1 | 22 | Hit and 2-turn -ATK 15% | FOC>=28 | Whirl ash to weaken an enemy striker. |
| 6 | Searing Brand | ARC | 45 | 6 | 1 | 31 | Burn if target unshielded | FOC>=33 | Brand an exposed rival with lasting heat. |
| 7 | Scorch Siphon | ARC | 33 | 5 | 1 | 45 | Heal user 35% damage dealt | FOC>=43 | Draw warmth from a hit to mend yourself. |
| 8 | Pyre Cascade | ARC | 66 | 9 | 2 | 60 | Extra 20% to target shield | FOC>=52 | Cascade flames over an enemy barrier. |

#### Tide (T)

| ID | Move | Class | Pow | EN | CD | Lv | Effect | Stat gate | Description |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 9 | Ripple Jab | PHY | 22 | 1 | 0 | 1 | Plain hit | None | Jab with a sharp ripple of water. |
| 10 | Brine Slice | PHY | 32 | 3 | 0 | 4 | 10% -SPD 20% for 2 turns | ATK>=16 | Slice with brine to hamper enemy speed. |
| 11 | Surge Hammer | PHY | 50 | 6 | 1 | 9 | Pierce 20% DEF | ATK>=20 | Crash a wave through physical armour. |
| 12 | Current Spear | ARC | 36 | 4 | 0 | 15 | Heal user 10% of damage dealt | FOC>=24 | Launch a current that restores a little health. |
| 13 | Undertow | ARC | 28 | 5 | 1 | 22 | Apply -SPD 20% for 2 turns | FOC>=28 | Drag a rival back with a heavy undertow. |
| 14 | Drench Mark | ARC | 45 | 6 | 1 | 31 | Next Tide hit against target +15% | FOC>=33 | Soak a foe to prime your next Tide hit. |
| 15 | Tidal Siphon | ARC | 34 | 5 | 1 | 45 | Heal user 40% damage dealt | FOC>=43 | Siphon health through a rolling tide. |
| 16 | Deepwave | ARC | 64 | 9 | 2 | 60 | Pierce 30% WARD | FOC>=52 | Send a deep wave through mystic defences. |

#### Thorn (N)

| ID | Move | Class | Pow | EN | CD | Lv | Effect | Stat gate | Description |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 17 | Thorn Prick | PHY | 22 | 1 | 0 | 1 | Plain hit | None | Prick a rival with a quick thorn. |
| 18 | Briar Hook | PHY | 32 | 3 | 0 | 4 | 10% Root | ATK>=16 | Hook a foe with a clutching briar. |
| 19 | Root Crush | PHY | 50 | 6 | 1 | 9 | Pierce 20% DEF | ATK>=20 | Crush armour beneath a heavy root. |
| 20 | Pollen Dart | ARC | 34 | 4 | 0 | 15 | 20% Poison | FOC>=24 | Scatter a dart of poisonous pollen. |
| 21 | Spore Veil | ARC | 24 | 5 | 1 | 22 | Poison if target unshielded | FOC>=28 | Seed poison on an unshielded rival. |
| 22 | Sap Rend | ARC | 46 | 6 | 1 | 31 | Heal user 15% damage dealt | FOC>=33 | Rend a foe and reclaim a little health. |
| 23 | Verdant Needle | ARC | 34 | 5 | 1 | 45 | Heal lowest-HP ally 25% damage dealt | FOC>=43 | Feed a wounded bench ally with stolen health. |
| 24 | Wildthicket | ARC | 64 | 9 | 2 | 60 | 30% Root | FOC>=52 | Entangle a rival in a wild thorn burst. |

#### Stone (S)

| ID | Move | Class | Pow | EN | CD | Lv | Effect | Stat gate | Description |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 25 | Gravel Flick | PHY | 22 | 1 | 0 | 1 | Plain hit | None | Flick a small but forceful piece of gravel. |
| 26 | Slate Knuckle | PHY | 32 | 3 | 0 | 4 | 10% -DEF 15% for 2 turns | ATK>=16 | Strike with slate to crack physical defence. |
| 27 | Boulder Ram | PHY | 50 | 6 | 1 | 9 | Pierce 20% DEF | ATK>=20 | Ram a boulder through enemy armour. |
| 28 | Shard Pike | ARC | 36 | 4 | 0 | 15 | 25% pierce WARD | FOC>=24 | Drive sharp stone through mystic protection. |
| 29 | Fault Pulse | ARC | 28 | 5 | 1 | 22 | Apply -DEF 15% for 2 turns | FOC>=28 | Shake the foe to expose physical weak points. |
| 30 | Dust Lock | ARC | 43 | 6 | 1 | 31 | Root for 1 round | FOC>=33 | Pin a rival beneath a cloud of stone dust. |
| 31 | Ore Bite | PHY | 36 | 5 | 1 | 45 | Generate self shield = 25% damage dealt | ATK>=43 | Turn a biting strike into your own barrier. |
| 32 | Mountainfall | PHY | 66 | 9 | 2 | 60 | Ignores up to 30% shield | ATK>=52 | Send a crushing blow around an enemy shield. |

#### Spark (K)

| ID | Move | Class | Pow | EN | CD | Lv | Effect | Stat gate | Description |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 33 | Spark Ping | ARC | 22 | 1 | 0 | 1 | Plain hit | None | Ping a rival with a focused electric spark. |
| 34 | Arc Slash | PHY | 32 | 3 | 0 | 4 | 10% Shock | ATK>=16 | Slash with an arc that may shock the foe. |
| 35 | Coil Smash | PHY | 50 | 6 | 1 | 9 | Pierce 20% DEF | ATK>=20 | Smash armour with a tightly coiled charge. |
| 36 | Volt Spear | ARC | 34 | 4 | 0 | 15 | 20% Shock | FOC>=24 | Cast a spear of shocking energy. |
| 37 | Static Pulse | ARC | 28 | 5 | 1 | 22 | Apply -SPD 20% for 2 turns | FOC>=28 | Disrupt the enemy rhythm with a static pulse. |
| 38 | Charge Mark | ARC | 45 | 6 | 1 | 31 | 20% Shock; 10% bonus versus slowed | FOC>=33 | Punish a slowed rival with a loaded charge. |
| 39 | Ion Siphon | ARC | 35 | 5 | 1 | 45 | Restore 2 ENERGY on hit | FOC>=43 | Recover energy from a clean electric hit. |
| 40 | Thunderline | ARC | 64 | 9 | 2 | 60 | First move next turn gains +1 priority | FOC>=52 & SPD>=44 | Strike hard and seize the next initiative. |

### 7.2 Universal tactical and support moves: 25

All five elements can eventually use every universal move below. This is the primary vehicle for deliberate team roles and no-item self/ally/all healing and resurrection.

| ID | Move | Role | Pow | EN | CD | Target | Lv | Effect | Stat gate | Description |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 41 | Plain Strike | PHY | 24 | 0 | 0 | EN | 1 | Neutral physical attack | None | A reliable basic attack when energy runs low. |
| 42 | Mend Self | HEAL | 0 | 4 | 1 | SELF | 1 | Restore 25% max HP, Focus scaled | None | Focus inward to repair your own wounds. |
| 43 | Guard | GUARD | 0 | 1 | 0 | SELF | 1 | Halve next incoming hit this round. | None | Brace against the next incoming hit. |
| 44 | Steady Breath | BUFF | 0 | 0 | 2 | SELF | 1 | Restore 3 ENERGY (cap 12); no damage. | None | Spend a quiet action gathering energy. |
| 45 | Aid Ally | HEAL | 0 | 5 | 1 | ALLY | 5 | Heal living bench ally 30% max HP; FOC scaled. | FOC>=16 | Restore health to one living bench ally. |
| 46 | Shield Ally | GUARD | 0 | 4 | 2 | ALLY | 5 | Shield living ally for 18% max HP | WARD>=16 | Place a protective shield on a bench ally. |
| 47 | War Signal | BUFF | 0 | 4 | 3 | TEAM | 9 | All living team +15% ATK for 2 rounds | ATK>=20 | Rally the team for stronger physical attacks. |
| 48 | Weakening Call | DEBUFF | 0 | 3 | 1 | EN | 9 | Enemy -20% ATK for 2 rounds | FOC>=20 | Unsettle the enemy physical attacker. |
| 49 | Cleanse Self | CLEAN | 0 | 3 | 1 | SELF | 10 | Remove one major status from self | WARD>=18 | Shake off one major status effect. |
| 50 | Brace Counter | GUARD | 0 | 4 | 2 | SELF | 12 | Next PHY hit -35%; reflect 30% post-guard damage. | DEF>=22 | Brace for a physical hit and strike back. |
| 51 | Group Mend | HEAL | 0 | 8 | 3 | TEAM | 15 | Heal all living allies 12% max HP | FOC>=24 | Spread a small healing pulse across your team. |
| 52 | Clear Signal | CLEAN | 0 | 5 | 2 | ALLY | 15 | Remove one status from living bench ally | WARD>=22 | Clear a major status from a bench ally. |
| 53 | Expose Guard | DEBUFF | 0 | 4 | 2 | EN | 18 | Enemy -20% DEF and WARD for 2 rounds | FOC>=26 | Expose the enemy physical and mystic guard. |
| 54 | Fleet Signal | BUFF | 0 | 4 | 3 | TEAM | 20 | All living team +15% SPD for 2 rounds | SPD>=26 | Give the whole team a burst of speed. |
| 55 | Life Transfer | HEAL | 0 | 4 | 2 | ALLY | 23 | Give bench ally up to 20% own max HP; keep 1 HP. | HP>=85 | Give a bench ally some of your own health. |
| 56 | Anchor Soul | REVIVE | 0 | 0 | 0 | SELF | 25 | Once: fatal at E8 revives self 20%; E=0; queued action lost. | WARD>=30 | Reserve energy for one automatic second chance. |
| 57 | Revive Ally | REVIVE | 0 | 9 | 3 | FAINT | 28 | Revive bench ally at 30% HP; shared single-revive token. | FOC>=32 & WARD>=28 | Bring one fallen bench ally back into battle. |
| 58 | Team Ward | GUARD | 0 | 7 | 3 | TEAM | 30 | All living allies gain 12% max HP shield | WARD>=32 | Raise a shared barrier around living allies. |
| 59 | Cleanse Team | CLEAN | 0 | 8 | 4 | TEAM | 35 | Remove major status from all living allies | WARD>=35 | Purge major statuses from your living team. |
| 60 | Focus Signal | BUFF | 0 | 5 | 3 | TEAM | 38 | All living team +15% FOCUS for 2 rounds | FOC>=38 | Sharpen the whole team mystic focus. |
| 61 | Tactical Drain | ARC | 30 | 5 | 1 | EN | 40 | Neutral ARC hit; user heals 35% dealt | FOC>=40 | Mend yourself through a neutral mystic strike. |
| 62 | Fading Step | BUFF | 0 | 3 | 2 | SELF | 45 | Next user action +1 priority; shield 8% max HP | SPD>=42 | Step lightly and prepare a faster next action. |
| 63 | Silence Hex | DEBUFF | 0 | 6 | 3 | EN | 55 | Enemy support costs +2 ENERGY for two turns. | FOC>=48 & SPD>=40 | Make enemy support actions harder to afford. |
| 64 | Revive Team | REVIVE | 0 | 12 | 0 | TEAM_FAINT | 70 | Revive all fainted allies at 15% HP; once per team. | FOC>=58 & WARD>=48 | Bring every fallen ally back for one last push. |
| 65 | Last Anthem | BUFF | 0 | 10 | 0 | TEAM | 80 | Living team: 20% HP shield and +10% damage, two turns; once. | ATK>=58 & FOC>=58 | Spend your final rally on shielding and power. |

### 7.3 Elemental defensive/support moves: 15 (3 per element)

| ID | Move | Type | Role | EN | CD | Target | Lv | Effect | Stat gate | Description |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 66 | Heat Mantle | Ember | GUARD | 4 | 2 | SELF | 10 | Shield self 22% max HP; next attacker has 20% Burn chance | DEF>=20 | Wrap yourself in a hot, punishing barrier. |
| 67 | Rallyflare | Ember | BUFF | 6 | 3 | TEAM | 26 | Team +20% ATK for 2 turns | ATK>=30 | Ignite the team physical fighting spirit. |
| 68 | Smolder Link | Ember | HEAL | 6 | 2 | ALLY | 50 | Heal bench ally 25%; remove Burn | FOC>=44 | Heal a bench ally and soothe its burns. |
| 69 | Flowguard | Tide | GUARD | 6 | 3 | TEAM | 10 | Shield team 10% max HP | WARD>=20 | Cover the team with a flowing water barrier. |
| 70 | Freshwater Rite | Tide | HEAL | 7 | 3 | TEAM | 26 | Heal all living allies 14% max HP; remove Burn | FOC>=30 | Wash wounds and burns from your whole team. |
| 71 | Tidal Return | Tide | REVIVE | 9 | 3 | FAINT | 50 | Revive bench ally at 25% HP; shared single-revive token. | FOC>=44 & WARD>=36 | Call one fallen ally back with the tide. |
| 72 | Root Shelter | Thorn | GUARD | 4 | 2 | ALLY | 10 | Shield living bench ally 22% max HP | DEF>=20 | Shelter a bench ally behind living roots. |
| 73 | Verdant Chorus | Thorn | HEAL | 7 | 3 | TEAM | 26 | Heal living team 8% HP now, 4% at next end-round; FOC scaled. | FOC>=30 | Mend the team now and again after the round. |
| 74 | Cleansing Bloom | Thorn | CLEAN | 7 | 3 | TEAM | 50 | Remove major statuses from all living allies | WARD>=44 | Cleanse the team with a restorative bloom. |
| 75 | Iron Mantle | Stone | GUARD | 4 | 2 | SELF | 10 | Shield self 25% max HP | DEF>=20 | Clothe yourself in thick mineral armour. |
| 76 | Bastion Kin | Stone | GUARD | 7 | 3 | TEAM | 26 | Team gains 15% max HP shield | DEF>=30 | Raise a stone bastion around the team. |
| 77 | Bedrock Echo | Stone | GUARD | 7 | 3 | SELF | 50 | Guard next hit 45%; reflect 35% blocked PHY damage. | DEF>=44 & ATK>=36 | Turn a guarded physical blow against its owner. |
| 78 | Quickstep | Spark | BUFF | 3 | 2 | SELF | 10 | Self +25% SPD for 2 rounds | SPD>=20 | Gather speed in a sharp electric step. |
| 79 | Flash Relay | Spark | BUFF | 4 | 2 | SELF | 26 | Priority +1 switch; incoming living ally gains 2 ENERGY. | SPD>=30 | Relay the lead and energise the incoming ally. |
| 80 | Arc Beacon | Spark | BUFF | 7 | 3 | TEAM | 50 | All living team +20% FOCUS for 2 rounds | FOC>=44 | Guide the team with a beacon of mystic power. |

### 7.4 Support, recovery and revival rules

- **Heal Self:** can only target a living user. **Heal Other:** targets another living team member, including a bench creature. **Heal All:** heals each living member of the current three-monster team. No items exist.
- **Revive Other:** usable by an *alive, active* creature to revive one fainted teammate. If no fainted target, grey out. A revived bench monster does not automatically become active; normal/forced switching applies.
- **Revive All:** usable by an *alive, active* creature, revives all fainted teammates at stated HP percentage; consumes its once-per-team-per-match **mass-revive token** and 12 ENERGY. Cannot be used when no one is fainted. **The user itself must be alive to cast this**, so there is no self-cast paradox.
- **Revive Self:** `Anchor Soul` is an **equipped reactive move**, not a manual action after KO. If it is equipped when the creature would faint from a hit or end-round damage, it has ≥8 ENERGY and has not used its personal self-revive this match, it automatically returns at 20% max HP at the end of the current resolved action, ENERGY→0, self-revive token spent; no extra attack that round. Render a distinct rising animation. It cannot trigger after match loss is already committed or against a later lethal tick in the same resolution; exactly once.
- A team has **one single-target revive token** usable by either `Revive Ally` or `Tidal Return` **(shared)**, and one mass-revive token for `Revive Team`; both max once per match. `Anchor Soul` has a separate **one-use-per-creature** counter; equipping the move is the opportunity cost. `Last Anthem` has an independent once-per-team token.
- **All team effects** update active and bench living allies (not fainted unless described). `Cleanse Team` must not remove KO. Shields cannot revive. Overheal is discarded.
- **Defense roles:** Guard damage reduction is a timed effect on incoming hit, Shields absorb a fixed amount of damage, Counter deals reflected fixed damage after surviving the hit. Shields cap at **35% of a target's max HP total** and expire after 2 end-of-round ticks.
- **Buff/debuff roles:** Percent modifications do not stack identically; refresh duration and replace weaker with stronger. Max two distinct positive and two distinct negative stat modifiers per creature. Types `Burn`, `Poison`, `Shock`, `Root` are major statuses; at most one at a time per target; newer status replaces only if no current status (never overwrite by luck).
- **Major status effects:** Burn deals `ceil(3% maxHP)` at end of round for 3 rounds and reduces PHY damage 10%; Poison deals `ceil(4% maxHP)` end-of-round for 3 rounds; Shock reduces SPD 25% for 2 rounds; Root prevents voluntary switching for 1 round. Status applies only if target survives hit and check succeeds; shields do not inherently block status unless move says *unshielded*.
- **No revive loops:** each revival has a finite token, and all fainted with no Anchor Soul trigger ends match immediately. High-level team healing has higher ENERGY opportunity cost; late-round pressure ends indefinite stall.

### 7.5 Exact effect resolution contract (no improvising missing mechanics)

The following definitions apply to the preceding 80 rows and must be codified as tested effect enums/parameters. Skill metadata may represent effects as compact typed fields, not free-form descriptions.

| Effect phrase | Exact interpretation |
|---|---|
| `X% Burn/Poison/Shock/Root` | After damage, one PRNG roll in `[0,999]`; apply if roll `<10×X`, if target survives and has no current major status. `Root` lasts 1 round; others §7.4 durations. |
| `Burn/Poison/Root if unshielded` | Guaranteed major status only if target had **zero shield immediately before damage** and survives; does not overwrite an existing major status. |
| `-ATK/-DEF/-SPD` | Timed percentage modifier, 2 rounds unless specified; use a typed stat-modifier slot, multiplicative and rounded down; do not accumulate multiple copies of same stat/sign. |
| `Pierce X% DEF/WARD` | During **this hit only**, replace applicable defending stat `D` with `D−floor(D×X/100)` before damage formula. Do not reduce target stat permanently. |
| `Ignores X% shield` | Calculate full damage; X% of that damage is applied directly to HP first, the remainder hits shield then HP. Total HP+shield loss never exceeds full damage. |
| `Extra X% to target shield` | If shield exists, increase amount deducted from shield by X% of base post-type hit, but **do not create extra HP damage** from the bonus. |
| `Next Tide hit +15%` | Add one `WET_MARK` marker to enemy for 3 end-round ticks. It is consumed by the **next qualifying Tide attack that damages that target**, applying ×1.15 damage before guard; one mark maximum, refresh only. |
| `Bonus against slowed` | Slowed means active -SPD debuff or Shock. Apply additive 10% damage multiplier for that attack only. |
| `First move next turn +1 priority` | A one-charge `NEXT_PRIORITY` flag on user consumed by next action within its next two activations; expires if unused; switch is an action too. |
| `Restore N ENERGY` | Modify user's per-creature ENERGY up to cap 12 after successful hit; if it misses because target fainted before action, no restoration. |
| `Heal N% of damage dealt` | Heal based on actual HP damage **not shield absorption**; can target self or stated bench ally; clamp to max HP. |
| `Remove Burn` | Remove Burn only, not other statuses; if target is not burned, remainder of move (e.g. healing) still applies. |
| `TEAM` buffs/shields | Snapshot currently living active and bench allies at activation; same team-wide duration and per-target caps. |
| `Team health regeneration` | Apply one delayed percentage heal at the next end-round after damage-over-time; do not revive KO allies. |
| `COUNTER` | If user survives the triggering PHY hit, deal stated percent of **post-guard damage prevented/dealt** as neutral retaliatory HP damage; no recursive counter. |
| `Self shield on attack` | Create a shield equal to specified percent of actual HP damage inflicted, subject to 35% max-HP cap and 2-round expiry. |
| `Quickstep/Flash Relay` | SPD buffs use normal bounded stat-modifier system; Flash Relay is a priority+1 **switch action** that moves in a living bench ally and grants it 2 ENERGY, and fails with no legal bench target. |
| `Support cost +2 ENERGY` | Applies only to non-damaging support moves (HEAL/GUARD/BUFF/DEBUFF/CLEAN/REVIVE), not PHY/ARC attacks or basic fallback; visible in move picker. |

**Ordering** on each action: validate cost/cooldown/target → debit energy → choose target and apply guard/shield → damage → permitted post-hit effects → mark cooldown/tokens → evaluate fatal trigger. At end-round: DOT → delayed regeneration → statuses/shield timers → fatal triggers/KO → victory. Fixed-point rounding must match host and client bit-for-bit; include golden action logs for every named effect.

**Known edge case:** A move with `ALLY` is unselectable when no other living ally exists. `FAINT` requires a fainted bench ally. `TEAM_FAINT` requires at least one fainted ally and an alive caster. A self-revival trigger checks once for that lethal event, never recursively triggers inside another revival, and cannot revive through match termination. This is deliberate: no infinite revival chains.

## 8. Battle system: explicit deterministic rules

### 8.1 Turn structure

- Battle: **three-versus-three team**, one active creature per side, two bench creatures. 1v1 actions (not six creatures simultaneously).
- At round start, living active creature receives **+2 ENERGY**, bench living creatures **+1 ENERGY** (max **12**); start each monster at **6 ENERGY**.
- Each side chooses one action: use equipped move OR switch to a living bench creature (switch consumes the action, priority +2). Forced switch after KO is free and occurs before next round, from living bench choices.
- In CPU mode the AI secretly chooses its action after evaluating visible state; in PvP both sides choose without seeing the other's selected action (commit/reveal in §11).
- Resolve order by priority (base attack=0, guard=+1, switch=+2; move-specific +/- modifier), then effective SPD; deterministic RNG breaks exact ties.
- Deduced `move cooldown` decrements **at the start of that creature's subsequent active turn**, not on a bench creature's unused rounds. The move used gets cooldown counter `CD+1` so `CD=1` prevents reuse on the next own activation.
- If a creature faints mid-round, its queued action is cancelled. If Anchor Soul is triggered, revival occurs at end of that one resolved action and does not retroactively restore the cancelled action. Once an entire team is fainted after all pending resolution/reactive revivals, commit the winner.
- Move unavailable if energy insufficient, invalid target, cooldown active or relevant once-per-battle token spent; **the UI greys it out**, and host rejects impossible remote actions. Player may use `Plain Strike` as an always-available fallback basic action **even if not equipped**, at 0 ENERGY (this action is not counted among the four equipped move slots), preventing dead turns.
- User can inspect own team, opposing revealed status and a compact battle-log summary at any time without spending a turn.
- Switching out does not clear persistent damage, status, cooldown, energy or shields; only a one-turn guard/counter stance ends when switched.

### 8.2 Damage and hit calculation (fixed-point integer)

For an attack class PHY or ARC:

1. `A = effective(ATK or FOC)`, `D = effective(DEF or WARD)` after bounded buffs/debuffs; use integer arithmetic.
2. `base = 2 + floor(POWER × A / (D + 20))` (POWER 22–70; applied to one target).
3. Multiply in this exact order, round down at each step: attacker move-affinity (primary 1.15, secondary 1.08, neutral 1.00); element effectiveness versus both defending elements (multiply the factors and clamp final 0.70..1.44); critical (one chance in 16, ×1.25); temporary boosts/penalties; enemy guarding/shield then HP.
4. `finalHit = max(1, bounded integer result)` if target HP>0 and no hard immunity. The move effect follows hit resolution; damage cap is current HP plus shield (avoid underflow).
5. Heal amount = `ceil(targetMaxHP × specifiedPercent × healingScale)`, where `healingScale=clamp(FOC / 30, 0.75, 1.30)` using fixed point. **For any heal caused by ‘percent of damage’ use actual HP damage dealt, not damage absorbed by shields.**
6. Critical RNG, status proc and speed tie use a committed, versioned **xorshift32** PRNG with defined state and byte-order; no platform `random()` calls in rules.

### 8.3 End-round updates and battle cap

After resolved actions: apply DOT → apply delayed regenerative effects → decrement/expire statuses, temporary stat modifiers and shields → evaluate KOs / self-revive → check result. Treat game engine phases as one deterministic ordered state machine with golden trace fixtures.

**Round 41+ arena pressure** reduces healing by 50% and deals environment damage to both active monsters: `2 + floor((round−41)/3)` each round, ignoring shields. All pressure events are shared, never personalized to punish one side. Hard cap **60 rounds**; if both teams still have surviving creatures, compare aggregate current HP percentage of full team; then surviving creature count; if still equal award draw.

### 8.4 Team synergies and strategic tradeoffs

- **No items** means a healer must use one of only four move slots; a tank can protect a wounded bench ally; a striker can opt for burst or status.
- Elemental counters require choosing which of three allies becomes active and when to switch. Dual-element order encourages meaningful lead and response selection.
- Healing one ally versus all has different energy thresholds; reviving costs a once-per-team token and forfeits a damaging action, while passive self-revival costs a move slot and reserved energy.
- `Plain Strike` at 0 ENERGY keeps desperate creatures actionable but loses to well-timed defense/elemental synergy.
- Role patterns to support in tests: **striker + healer + defender**, **debuffer + finisher + self-reviver**, **three balanced creatures**, **attrition/status squad**. Each should have realistic win conditions; none should be automatically dominant.

## 9. CPU opponents: fair, adaptive, no hidden cheating

### 9.1 Team construction

- Compute player team strength from three creatures' **actual** derived levels, stage stats, total invested training ranks and chosen primary/secondary elements.
- Generate a CPU team of **3 distinct families**. Choose levels near player **median** (normally within ±2 levels), do not grant fake hidden stat multipliers or impossible moves, and build 4 unlocked move slots per CPU creature using the same validation functions as players.
- Stat ranks for CPU are allocated according to its chosen level (exactly `3×(L−1)` ranks), six caps and a role template (striker/tank/support/hybrid). Stage, stats, cooldowns and move unlocks are derived identically.
- To prevent a one-sided elemental hard-counter, at most **one CPU creature** is selected primarily for direct countering of the player's strongest lead. Others should form a plausible balanced squad.

### 9.2 Decision logic

- AI score each legal action using only public battle state, current HP/energy/cooldowns, known types, observed enemy moves, own team state and simulated one-step outcomes; it **must not inspect unrevealed player move choice or peek at hidden PRNG future**.
- Sample heuristic terms: expected immediate damage +0.9, KO potential +2.0, own/team emergency heal +1.2, revive value +2.3 when down an ally, shield versus likely incoming pressure +1.0, elemental advantage on switch +0.8, wasted shield/overheal −1.4, energy inefficiency −0.3, late-round pressure risk +1.1. These are seed weights to tune from simulation.
- `Novice` (early levels or repeated losses): choose among 2–3 top moves with noise, tends not to perfect-predict; `Regular` (most matches): sensible action ranking; `Veteran` (higher levels with strong results): more accurate one-turn evaluation, opportunistic switch/revive, **never perfect information**.
- CPU may adjust tactics for last 5 results, but its disclosed level and trained stats remain within normal bounds; **after 3 losses**, reduce AI depth/noise before lowering roster level more than 2. No hidden rubber-banding during a running match.
- Off-line CPU decisions should complete within **10 ms target on nRF52840** (measure); no deep tree searches or heap explosions. When CPU takes longer, use a small state-machine work budget across frames without blocking radio.

### 9.3 Balance constraints

- Against a similarly levelled player using a competent balanced team, target **45–55% win rate** on Regular AI over a seeded corpus, without training advantages for CPU. Novice should be clearly beatable; Veteran should reward intentional team-building.
- Nonoptimal team builds remain viable; no single element/role dominates every other with 60-monster roster.
- CPU selection must use a deterministic seed stored with the battle log; reproduce every encounter in native host-side tests for bug reports.

## 10. Pre-battle, local flow, UX and save semantics

### 10.1 Single-player

`Open Veylith Arena → Select CPU → Trainer mandatory → select exactly 3, train stats, change any of 4 move slots for 0 XP → READY → CPU team generated/displayed with approximate levels and types → Fight → results → XP to each selected family → return Trainer (required again next battle)`. Do not persist temporary injuries between matches.

### 10.2 Multiplayer

`Host → choose an existing eligible private channel → LOBBY_OPEN special chat post → reannounce if desired → guest sees popup only if setting ON and game installed OR finds lobby in Games → Matchmaking → Join Request → host admits → each player enters Trainer → locks READY snapshot → MATCH_START → battle → result + XP on both sides → close lobby`. A host cannot start before both explicitly READY. No player is auto-joined by channel membership.

- Before every rematch, **both players revisit Trainer**; player loadout/stat changes between matches are allowed and free if move-only. Ready always commits a fresh immutable roster snapshot.
- `Invite popups [ON/OFF]` is an existing shared Games setting. OFF means no interruption but matchmaking screen and chat still show open games.
- Repeated announcements reuse same lobby/session ID; receiving duplicates refreshes a visible listing, no repeated popup. Manual host `Announce Again` is allowed (respect minimal send interval/airtime).
- If zero eligible private channels exist, show: **“No private channels configured. Add one in MeshCore Solo or the companion app.”** Games offers **Back**, never create/import/edit channel controls.
- Default public channel and predictable hashtag keys do not qualify as private. Classification must use trusted Solo channel metadata; if unknown, exclude (fail closed).
- All game packets, including challenges, moves, readiness, results and spectators, use that selected private channel. A shared channel is not a game roster; multiple independent lobby IDs may coexist.

### 10.3 Session failure behaviour

- Host crash, loss of private channel, message gaps or timeout suspend (not guess) the battle. Resume only with validated complete participant state and session IDs; otherwise cancel without XP.
- Input/menu navigation should never block MeshCore RX/TX, BLE, GNSS or normal chat. Rendering does not generate radio traffic.
- Older Solo firmware sees a marked chat message (e.g. `~G1:<base64url>`); newer Solo versions must parse it safely and display game-aware compact chat rows rather than raw gibberish wherever integration permits.

## 11. Multiplayer wire protocol, fairness and spectator history

**Use the Game Library master GDD lobby/session layer**, not a new competing networking stack. All packets remain marked, length-bounded **private-channel chat messages** (`~G1:` + Base64url packed binary is a suitable compatibility design). The text encoding is not cryptographic secrecy; MeshCore's existing private channel handles group encryption.

### 11.1 Minimum battle-specific events

| Event | Content | Committed/public replay? |
|---|---|---|
| `LOBBY_OPEN` | game ID, rules version, session, host, slots | Yes, first event required |
| `LOBBY_ADVERTISE` | repeat open message, same session ID, current occupancy | No new state commit |
| `PLAYER_JOINED`, `PLAYER_READY` | admitted identity, slot, ready flags | Yes |
| `TEAM_SNAPSHOT` | one locked team, 3 family IDs, 6 training ranks per family, 4 move IDs, level; split in bounded chunks only if needed | Yes, committed only after ready |
| `MATCH_START` | both roster hashes, public initial state, RNG seed, tie-break rules | Yes |
| `TURN_COMMITMENT` | actor, round, cryptographic digest of secret action+nonce | Yes |
| `TURN_REVEAL` | actor, round, chosen move/switch/target + 64-bit nonce | Yes after both commitments |
| `TURN_RESULT` | ordered actions, damage/heal/status/cooldown changes, checksum | Yes |
| `FORCED_SWITCH` | actor, replacement family slot | Yes |
| `MATCH_END` | winner/draw, result checksum, XP award receipt IDs | Yes |

**Simultaneous hidden decision PvP:** To stop a player simply waiting and reacting to the opponent's exposed move, each side sends `TURN_COMMITMENT = first 16 bytes SHA-256(session || round || actor || encodedAction || random64nonce)` before either action is revealed. Once both commitments exist, both send matching `TURN_REVEAL`; reveal mismatch rejects turn. That is a **fairness/commitment mechanism, not encryption**. Use vetted crypto already available or introduce the smallest tested hash implementation; never invent cryptography. Limit airtime; if the compact protocol doesn't fit reliably or is too slow, ship CPU mode first and gate PvP rather than downgrade fairness silently.

**RNG ownership:** host commits seed before match start; exactly one deterministic state transition function shared by both clients. Host broadcasts canonical result hashes to support log replay and diagnose mismatches; clients reject inconsistent outcomes rather than applying unknown state.

### 11.2 Full-history observer rule (non-negotiable)

A spectator may render live or replay **only if this device's local *normal private-channel chat history*** contains: initial `LOBBY_OPEN`; every authoritative slot/ready transition; both accepted `TEAM_SNAPSHOT` events; `MATCH_START`; complete ordered `TURN_COMMITMENT/REVEAL/RESULT` triples for **all previous rounds**; all forced switches; and `MATCH_END` if completed. Check sequence continuity and rules version. Otherwise display **“Spectator unavailable: complete match history including matchmaking is missing.”**

- Do not admit spectators merely because they are on the same channel or received a latest snapshot.
- Deduplicated lobby re-advertisements do not replace a missing original `LOBBY_OPEN` for complete-history qualification.
- Spectators have read-only state; no turn actions, no automatic game join, no player slot. They see revealed actions and complete rosters from committed start, not unrevealed precommit choices.
- Local chat retention may be too short for a long 3v3 match. **Do not claim spectating works if Solo has already evicted needed events.** Surface retention cap and missing-sequence error.
- Avoid storing raw sensitive channel secrets in game logs; honor Solo message persistence and storage constraints.

### 11.3 Reliability and airtime

- No animation frames, menu changes, rendering ticks, polling or CPU 'thinking' over the radio; only lobbies, readiness, moves and authoritative results.
- Host-ordered committed event sequence with CRC + bounded client action IDs, deduplication, timeouts/retries with randomized backoff, and no radio send storms. Spectator requires contiguous authoritative commit sequence; when missing, spectate disabled.
- Store session key `(channel identifier, session ID, game ID, rules version)` to isolate games sharing same channel. Public or hashtag-channel game messages are never accepted.
- Tune exact packet sizes based on checked-out Solo payload limit and duty-cycle settings; keep typical events well under platform chat text max; never assume RF supports unlimited history or bulk state.
- The game's over-the-air messages are parseable/visible as **special game chat messages** through Solo UI and traceable to local history without leaking channel keys.

## 12. Persistent progression and memory budgets

### 12.1 Save format (suggested packed v1)

```c
struct FamilySaveV1 {
    uint32_t xp_wallet;          // spendable XP
    uint32_t xp_lifetime_earned;
    uint8_t train_rank[6];       // each 0..80, sum max 294
    uint8_t equipped_move_id[4]; // 1..80, distinct, eligible by level/element/permanent stats
    uint8_t flags;               // migration/other low-bit indicators
}; // 19 bytes payload before explicit serializer
struct PlayerSaveV1 {
    uint8_t schema_version;
    FamilySaveV1 family[20];
    uint8_t selected_family[3];
    uint32_t recent_result_ids[8]; // bounded anti-duplicate XP
    uint32_t crc32;
};
```

Do **not** write packed C structs as raw persistence records: explicitly serialize fields with stable byte order and a version/length/CRC. Validated decoded save size ~**400–600 bytes** plus overhead. Save only on battle results, confirmed training purchases, equipped move changes and lineup changes, **not per animation frame or CPU reasoning step**. Atomic replace/write-through using Solo's current persistent backend; inspect wear-leveling and per-namespace quotas. Do not overwrite MeshCore settings, chat/channel keys or BLE state.

At first boot assign every family initial four moves from its two element starters plus universal move IDs 42 (`Mend Self`) and 43 (`Guard`); initial lineup defaults to 3 distinct families, or ask the user to pick. Support corruption repair only after a user-visible notice and safe backup path when feasible.

### 12.2 Additional game ROM/RAM estimates (PLANNING ONLY)

| Part | Flash budget | Active RAM budget |
|---|---:|---:|
| 60 front-only 32×32 bitmaps | 7,680 B raw; mirrored player view shares the same data | 0 B sprite scratch; draw mirrored runs into the shared framebuffer |
| 80 move definitions and names | 2–5 KB | Read-only in flash |
| 20 families, formulas, names | 2–5 KB | Read-only in flash |
| Rules, effects, validation, stat/XP engine | 18–40 KB | 1–3 KB |
| CPU AI, seeded encounter generation | 8–20 KB | 0.5–2 KB |
| UI/animation/effects | 12–30 KB | 0.5–2 KB |
| Network codec and battle adapters (reuse Games core) | 8–18 KB | 1–3 KB |
| Saves/session/team state | 1–4 KB | ~0.5–2 KB |
| **Total incremental budget target** | **~66–152 KB** (not measured) | **< 12 KB** incremental where practical |

**Important:** Solo and its platform/bootloader reserve unknown portions; **no 1 MB blanket game budget**. After baseline and after each milestone, parse linker map and static RAM and record peak runtime stack/heap for game and base firmware. If memory is short: compress sprites, deduplicate pixel motifs, prune uncommon moves only after balance tests, gate feature groups, or explore external QSPI only after verifying ownership. **Prioritize finished game quality over number of features**; don't shrink 60-species visual quality to ship an unfinished game.

### 12.3 Implementation architecture

```text
Solo existing platform (radio, BLE, UI, chat, channels, storage)
    ↳ Games registry / lobby manager / private-channel filtering
    ↳ Veylith menu & Trainer / XP save model
    ↳ 1-bit battle renderer + shared animation FX
    ↳ Pure battle rules + 80-move move-table
    ↳ Fair adaptive CPU decision engine
    ↳ PvP adapter / commit-reveal / host event ordering
    ↳ History-driven spectator replay (no shortcut via snapshots)
```

Never couple rules to display frame pacing. `BattleState` and `FamilySave` are fixed-size and validated. Use the existing Solo compiler, linker, display abstraction and input mapping; create unit-testable pure rules components for host-side C++ tests.

## 13. Content quality, balance and automated acceptance suite

### 13.1 Data integrity tests

- Exactly **20** family rows, exactly **60** unique species names, exactly **20** unique ordered dual-element pairs, and every family has **three** forms and no self-paired elements.
- Exactly **80** unique move IDs and names; **40 elemental attacks + 25 universal tactics + 15 element tactics**. All names referenced by loadouts resolve. Every family at level 1 has six legal choices for four distinct equipped slots. Every affiliated family can reach each listed stat gate at that move's minimum level within the normal rank budget. Every status/effect has an implemented deterministic handler.
- Type wheel: each element has exactly two strengths and two weaknesses; all dual-pair effectiveness values within 0.70–1.44.
- Every move role represented in playable unlocks: `PHY`, `ARC`, `HEAL`, `GUARD`, `BUFF`, `DEBUFF`, `CLEAN`, `REVIVE`; specifically Heal self/other/all; Revive self/other/all; defensive and offensive options.

### 13.2 Training and progression golden tests

- `L1, rankTotal=0 → (LV1, 0/3)`; purchasing one rank → `(LV1,1/3)`, two → `(LV1,2/3)`, third → `(LV2,0/3)`; XP cost deducted exactly once; no negative wallet.
- A Level 14 creature evolves to stage II when three further training ranks produce **Level 15**; Stage III only at **Level 35**. No duplicate species in lineup from the same family.
- Loss at opponent average Level 10 gives exactly **63 XP per selected family**; win gives **140 XP**, no XP for unselected families, and retries/replayed results do not double grant.
- `MOVE` swap in Trainer costs **0 XP** and becomes available as soon as all level, element and permanent-stat gates are met; locked/unrelated element IDs rejected. READY snapshot cannot mutate, even if normal profile is edited elsewhere.
- Level 99 maximum, 294 total training ranks, each stat max rank80; rejecting extra training after cap.

### 13.3 Battle mechanic tests

- Elemental pair ordering (primary/secondary) changes affinity while defense remains dual-element; all 5×4 combos covered.
- PHY versus DEF and ARC versus WARD damage; no floating point nondeterminism; speed/priority; status expiry; bench energy regen; KO cancels queued move; forced switch.
- Heal Self, Heal Other, Heal All target living creatures correctly; max HP cap; dead creatures not healed by ordinary skills.
- Revive Other, Revive All, Self-revive trigger once; cannot revive when all creatures are KO and no live caster/Anchor; token exhaustion prevents infinite loop.
- Shield cap/expiry, Guard effect, Counter effect, cleanse team, status uniqueness, pressure rounds 41–60 and hard draw tie-break.
- CPU never chooses illegal move or spends more energy than present; adaptive roster stage and legal trained ranks match visible CPU level.
- No single role or element exceeds 65% win rate against all peer-level balanced archetypes in seeded simulations; adjust moves or chart only with documented rules-version change.

### 13.4 UI and graphics tests

- True panel screenshots at **128×64**, including 3-stage front/mirrored-front views for all families: visible silhouette differences without grayscale; no clipping of critical HP/energy/turn/status.
- Trainer always appears before battle. Show XP wallet, selected stat rank, purchase price, after-buy value, level-progress bar, next move unlock and next evolution. Screens are usable without phone.
- Move list shows locked/unlocked with level, source type and exact permanent-stat requirements; exactly 4 equipped slots, no duplicate moves, swapping costs no XP.
- Animations localized, nonblocking, consistent with status after commit; can be skipped without changing simulation.
- No companion-app channel modifications. No public/hash-channel lobby picker.

### 13.5 Multiplayer and spectator tests

- Two players see one lobby advertisement, do not auto-join; popup ON only when installed+compatible+private channel. Popup OFF still discovers lobby via matching screen and chat. Reannounce same lobby ID refreshes not duplicates.
- Host confirms JOIN and READY; both see mandatory Trainer; one active match per device; older firmware treats unknown game packets safely.
- PvP both commit before reveal; out-of-turn, duplicate, malformed, changed nonce/action, incomplete snapshots rejected.
- Complete local chat history from `LOBBY_OPEN` through turn N permits spectator replay through N; missing any committed sequence event disables spectator.
- Wireless packets fit measured Solo limits; 1%/10% duty-cycle relevant to chosen radio band is enforced by existing Solo system; no UI frame traffic.
- Channel disappears/reclassified unknown midgame → stop multiplayer transmissions, preserve last verified local view; no attempt to manage channels.

### 13.6 Performance and release gates

- Capture baseline L1 Pro OLED build size before feature; record firmware flash delta, static RAM delta, peak stack/heap, render timing, LoRa airtime per battle and battery current proxies (if available).
- All 60 front sprites and their runtime-mirrored player views pass true-scale manual inspection, not only screenshot zoom. The 60 stored forms must be distinct; separate rear artwork is not required.
- Automated deterministic duel test corpus: ≥10,000 seeded simulated CPU matches across elements and archetypes, save/load/restore and history replay checks.
- Device soak test: ≥2 hours simultaneous idle-game browsing with normal MeshCore messaging and BLE enabled; at least 50 CPU matches, 10 PvP matches, deliberate packet loss/retries, power cycle during Trainer vs active match.
- If flash/RAM conflicts exist, *first* reduce nonessential cinematic animation, secondary UI features, or content not in this 60-creature core; preserve all five types, 20 lines, stat training and save reliability. Do not silently remove promised features.

## 14. Codex development roadmap (quality-first, easiest → hardest)

| Milestone | Deliverable | Do not proceed until… |
|---|---|---|
| **0** | Repo audit, L1 OLED baseline build, actual free flash/RAM, persistence/channel API, relevant Games core inventory | Baseline compiles and measured budgets recorded |
| **1** | Full static data tables: 5-type chart, 20 families/60 forms IDs, 80 moves, save/load validators, deterministic XP/stat/level/evolution functions | Static validations and XP golden tests pass |
| **2** | Trainer UI (team of 3, XP wallet, stat purchases/level progress, unlocked pool, 4 free-swappable moves) and migration-safe persistence | Power-cycle restores all 20 families and loadouts |
| **3** | Local deterministic battle engine for 1v1 active creature, statuses/support/revive/forced switching and 60-round end rule | All skill categories and golden battle traces pass |
| **4** | 60 polished 1-bit front sprites, runtime horizontal mirroring, animations, compact battle UI | True 128×64 physical OLED readability approval |
| **5** | Adaptive CPU arena, XP on victory/defeat, role-balanced matchmaking by team level | Fairness & balance seeded corpus passes |
| **6** | Host private-channel lobby, mandatory Trainer before READY, snapshot, PvP commit/reveal, sync/checksums, end receipts | Repeatable no-packet-loss & simulated loss PvP tests pass |
| **7** | Complete-history spectator replay, graceful recovery and field test; full polishing/balancing | Full chat event history gating and board soak test pass |

Implement milestone 0–2 first; **do not start coding all 80 move effects and 60 sprites before the display/flash/persistence baseline is understood**. Create small scripted asset/data validation generators in repository; ship generated `constexpr` content, not heavyweight runtime parsers. The game can be feature-gated if any milestone is unfinished.

## 15. Open technical decisions requiring measurement (NOT license to redesign the brief)

1. Exact Solo version/commit and game module API available in the checked-out branch.
2. Available linker flash/RAM after BLE/LoRa stack and current Solo features.
3. Current chat-history record size and retention: can it store a complete 3v3 match event stream including original lobby? Spectator MUST fail closed if not.
4. A safe persistence namespace/partition for per-family XP and skill loadouts, with versioning/atomic saves.
5. Actual payload limit on chosen channel messages and measured round duration/airtime with commit-reveal; whether two-team snapshots need chunks.
6. Existing validated SHA-256 support for fair turn commitment; no custom untested crypto.
7. Physical OLED font legibility, sprite contrast, frame pacing with radio activity; optional e-ink-specific rendering later.

The required **3-member teams, 60 forms, 20 ordered elemental pairs, 80 moves, 1-bit presentation, pre-battle Trainer, XP spending on stats, levels unlocking free move choices and all support categories** are product requirements, not open decisions.

## 16. Codex execution prompt (paste as-is after adding this file to the repository)

> Implement **Veylith Arena** from `Veylith_Arena_GDD_v1.0.md` as the final flagship game within the existing MeshCore Solo Games framework. This document overrides the old six-creature fixed-level Creature Arena outline. Start with Milestone 0: inspect actual repo/board definitions and existing Games, chat-history, rendering, channel filtering, settings and persistence APIs. Record flash/RAM baseline. Then implement Milestone 1 static content, deterministic progression, stat training and unit tests. Then Milestone 2 pre-battle Trainer with a three-creature team, XP wallet consumed on chosen stat upgrades, three training purchases per level, auto evolution at levels 15/35, levels 1–99 unlocking moves, four freely editable move slots drawn from a level/element/permanent-stat gated pool, short descriptions for all 60 forms and 80 moves, persistent data. Stop at the Milestone 2 quality gate and report changed files, tests, binary size delta, open risks and next phase. Do not invent APIs, change channel configuration, bypass the private-channel-only rule, or rewrite the MeshCore transport. Later add the 1-bit Gen-1-inspired battle interface, complete 80-move engine, adaptive CPU, fair private-channel PvP, and history-qualified spectators in milestone order.

## 17. Reference documentation

- Seeed Wio Tracker L1 hardware (processor, memory, display variants): https://wiki.seeedstudio.com/wio_tracker_l1_node/
- MeshCore Solo source and board targets: https://github.com/MarekZegare4/MeshCore-Solo
- MeshCore channel protocol / private versus hashtag keys: https://github.com/meshcore-dev/MeshCore/blob/main/docs/companion_protocol.md
- MeshCore Solo developer feature matrix (OLED vs portrait/landscape e-ink): https://support.ostrachhelp.de/Mirrors/MeshCore-Solo/src/branch/dev/docs/solo/developer/feature-matrix.md

**End — Veylith Arena GDD v1.0.**

