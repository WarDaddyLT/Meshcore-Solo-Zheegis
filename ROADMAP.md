# Žygis project roadmap

This roadmap describes project status, not promised release dates.

## Community preview — available

- Standalone Solo messaging/navigation foundation.
- Private position/battery reports and persistent channel binding.
- GPS acquisition/sleep scheduling, targeted channel commands and alerts.
- Direct-repeater discovery and route acknowledgements.
- Tracking-enabled build configurations; current adapters are listed in [hardware integration](docs/hardware.md).
- English and Lithuanian introductions, setup guides, and contribution templates.
- Shared tracking regression tests and CI for the integrated build targets.

## Before a field-tested release

- Record physical-device results for both targets: GPS, audio, keypad, LEDs,
  reboot persistence, and recovery from missing GPS fixes.
- Measure power use during acquisition and sleep; document the results.
- Verify navigation and GPX workflows with the tracker GPS policy.
- Verify the M5 full-flash partition migration on a device.
- Resolve dependency licensing requirements before distributing binary builds.
- Publish reproducible build instructions and release notes with an explicit
  tested-device matrix.

## Community input wanted

- Clearer setup flows and Lithuanian wording.
- Field reports with board, build SHA, radio preset, and reproducible observations.
- Desired navigation/tracking behavior and useful on-device controls.
- Additional boards, proposed individually with a maintainer/tester.

Ask questions in [Discussions](https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis/discussions)
or propose work through [Issues](https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis/issues).
