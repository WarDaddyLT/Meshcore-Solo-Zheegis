# Veylith Arena — front-only 1-bit assets

This is the supplied v2 pack, revised to **v2.1** following the project's
front-only artwork decision. Each of the 60 forms stores one **32×32**, 1-bit
front PNG. The player uses a horizontal mirror of that image at render time;
there are no separate rear sprites, mirrored copies or flip scratch buffers.

## Canonical source assets

- `creatures/*_front.png`: 60 front sprites in GDD species order.
- `manifest.json`: family/stage/name mappings, front crop metadata and warnings.
- `trainer/trainer_{idle,train,level_up,ready}_24x32.png`: four Trainer poses.
- `trainer/species_01_icon_16px.png` through `species_60_icon_16px.png`: team portraits.
- `trainer/level_progress_{0,1,2,3}of3_32x7.png`: progress glyphs.
- `ui_icons/`: 23 icons at 8×8.
- `trainer_screen_layouts.json` and `previews/`: Trainer layout concepts and
  128×64 reference screens; dynamic text uses the Solo font at runtime.

Pixel format is row-major, **MSB first**, white=1, black=0. For a 32×32 sprite,
pixel `(x,y)` is bit `7-x%8` of byte `y*4+x//8`.

## Firmware export

From the repository root, run:

```sh
python3 tools/veylith/generate_assets.py
python3 tools/veylith/generate_assets.py --check
```

The generator validates PNG dimensions/format and GDD names, then writes the
single compiled export, `src/games/veylith/Assets.{h,cpp}`. Creature art uses
**7,680 bytes**; all creature/Trainer/icon/progress data uses **10,280 bytes** of
const flash. Runtime mirroring emits horizontal pixel runs into the existing
framebuffer. It stores no additional sprite or framebuffer.

The old 120-view atlas, raw binary, duplicate C++ exports, icon atlas, separate
rear PNGs and front/rear QA sheet were removed. Keep the canonical PNGs and
regenerate the firmware export instead of maintaining parallel copies.

## Quality notes

This remains prototype artwork. Some front source crops touch their gutters;
see `manifest.json/source_crop_warning_ids/front` and review all 60 forms at
true OLED size before declaring the artwork final. The GDD is authoritative
for team selection, XP training, levels, evolutions and free move swaps.
