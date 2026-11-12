# Contributing to MeshCore Solo Žygis

Contributions and feedback in Lithuanian or English are welcome. Start with
[README.md](README.md) or [README.lt.md](README.lt.md).

## Questions, bugs and ideas

Use [Discussions](https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis/discussions)
for setup questions and general feedback. Use an
[issue template](https://github.com/WarDaddyLT/Meshcore-Solo-Zheegis/issues/new/choose)
for reproducible bugs or feature proposals. Include the exact board, build
environment, commit/version, steps, and expected versus observed behavior.
Remove channel secrets and personal locations from logs before sharing them.

Discuss architectural changes or new board support before implementing them.
Small documentation and bug fixes can go directly to a pull request.

## Pull requests

Branch from `main` using names such as `fix/gps-cycle`, `feature/board-support`,
or `docs/lithuanian-guide`, and target this fork's `main`. Keep one change per PR.
Describe the behavior, link related issues, list affected targets, and state
which checks ran. Distinguish host tests, firmware builds, and physical-device
results. Update user documentation when behavior or setup changes.

## Style and architecture

Follow the surrounding C++ and `.clang-format`: two spaces, attached braces,
`camelCase` functions/variables, `PascalCase` classes and `ALL_CAPS` macros. Avoid
unrelated reformatting and dynamic allocation outside setup/begin. Keep tracking
behavior under `PRIVATE_STATUS_REPORTER`, preserve ordinary Solo targets, and
reuse the existing keypad driver. Preserve Solo's `NodePrefs` schema; tracker
configuration has its own versioned record. See [AGENTS.md](AGENTS.md).

## Validation

```sh
pio test -e native -e native_kiss_modem -vv
python3 test/techo_status/test_status.py
python3 test/techo_status/test_cycle.py
python3 test/techo_status/test_dispatch.py
python3 test/techo_status/test_persistence.py
python3 scripts/check_project_docs.py
pio run -e LilyGo_T-Echo-Lite_keyshield_zygis -e ThinkNode_M5_zygis
```

Add meaningful regression cases for behavior changes. Device-specific changes
also need the relevant hardware observations; do not claim device testing from
a successful build alone.

## Attribution and upstream

Preserve copyright and dependency license notices. See
[upstream maintenance](docs/upstream-maintenance.md) before updating the Solo
base or submitting changes upstream. There is no special automated-PR merge path.
