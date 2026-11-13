# Žygis migration to MeshCore Solo

The tracking changes were ported from MeshCore commit
`66e7ff37ced46b40c8243a01606817e35a157401` onto Solo commit
`3d70c515` (the fork's original `Faketec_st7789_solo` branch).
The project repository is
[WarDaddyLT/Meshcore-Solo-Zheegis](https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis).

## Integration choices

- New opt-in environments are in `solo/zygis/platformio.ini`. Existing board and
  Solo environment settings are retained.
- Tracking invokes Solo's `AbstractUITask` instead of the removed upstream
  listener interface. Incoming channel history remains available.
- Solo's TCA8418 keypad driver handles alerts and normal navigation. A second
  keyboard library instance would consume the same hardware FIFO.
- `/zygis1` stores a magic/version marker, interval, bound slot/name/key, and
  known-position flag, independently of Solo's preference schema. Saves use
  `DataStore::commitFile` after a successful complete temporary write.
- CLI rescue owns the USB terminal during configuration. Reading raw `Serial`
  alongside Solo's dual-interface frame decoder would corrupt binary traffic.
  Use `exit` to return to the companion protocol.
- The tracker scheduler owns GPS acquisition and sleep. It requires a fresh fix
  per wake and has no timeout. Solo navigation can use those fixes, but live
  navigation does not keep GPS awake in tracker builds.
- Automatic public location adverts, remote location telemetry, and automatic
  Solo live shares are suppressed on tracking targets. Intentional manual
  navigation/waypoint shares remain available.
- M5 uses the Arduino `min_spiffs.csv` partition table: two 1.875 MiB application
  slots, 128 KiB filesystem, and 64 KiB core dump region. Switching layouts needs
  a data backup and full USB flash, not an application-only OTA update.

## Validation

Run both tracker builds, the two PlatformIO native suites, and all four Python
checks in `test/tracking/`. Keep physical-device verification separate:
GPS cold start and reacquisition, paused listening, reboot persistence,
private-channel rebinding, alarm/message dismissal, audio, LEDs and current draw.
No firmware was flashed as part of the source migration.

The migration checks passed: 69 native test cases, tracker privacy/dispatch/GPS
cycle/persistence checks, both tracking firmware builds, and the unmodified Solo
simulator build. T-Echo Lite uses about 76% RAM / 67% flash; M5 uses about 38% RAM
/ 73% application flash with its tracking partition table.

## Local recovery snapshot

The original changes were archived before porting. A sibling backup directory
named `MeshCore-Zheegis-backup-20261113-010711` contains a verified
`working-files.tar.gz`, SHA-256 `manifest.json`, binary `changes.patch`, base SHA,
and a complete `history.bundle`. This local backup is not published to GitHub.
To recover independently, clone the bundle, check out the SHA in `base-commit.txt`,
and overlay the archived working files. The patch is an alternative for tracked
changes; do not apply it again after overlaying the archive.
