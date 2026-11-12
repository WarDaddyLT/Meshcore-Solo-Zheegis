# Maintaining the upstream relationship

Žygis is a downstream project of
[MeshCore Solo](https://github.com/MarekZegare4/MeshCore-Solo), which builds on
[MeshCore](https://github.com/meshcore-dev/MeshCore). Keep their attribution and
license notices. Solo's broader hardware guide is retained in [README-Solo.md](../README-Solo.md).

## Current base

The original Solo base is `3d70c515`; the tracking port is `17bfaf9d`.
Record any later base updates in the changelog. `main` is this fork's integration
branch. Do not rename an upstream version as if it were a Žygis release.

## Updating Solo

1. Start a separate update branch from `main` with a clean working tree.
2. Fetch the Solo repository and inspect the chosen commit/tag before merging.
3. Resolve changes to GPS scheduling, channel handling, persistence, display,
   keypad and audio without overwriting the Žygis integration.
4. Run both tracking builds, native/regression checks, and the ordinary Solo
   simulator build. Record hardware results separately.
5. Open a PR documenting the new upstream SHA and behavioral changes.

Avoid importing an upstream release workflow or custom hosting domain without
review. This fork's default CI checks source and builds; it does not publish
firmware or deploy the upstream docs site.

## Version and release naming

Use `zygis-vMAJOR.MINOR.PATCH` for future Žygis tags, with prerelease suffixes
such as `zygis-v0.1.0-preview.1` when appropriate. No version tag is created by
repository preparation. Upstream firmware versions retain their own meaning.
See [RELEASE.md](../RELEASE.md) before publishing binaries.
