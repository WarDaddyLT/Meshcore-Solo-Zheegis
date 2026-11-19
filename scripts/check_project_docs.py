"""Check local links and neutral tracker examples in the public project guides."""
from pathlib import Path
import re
import sys
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
GUIDES = [
    'README.md', 'README.lt.md', 'README-Solo.md', 'AGENTS.md',
    'CONTRIBUTING.md', 'SECURITY.md', 'ROADMAP.md', 'CHANGELOG.md', 'RELEASE.md',
    'docs/tracking.md', 'docs/hardware.md', 'docs/tracking-cli.md',
    'docs/zygis_migration.md',
    'docs/upstream-maintenance.md', 'docs/community-intro.lt.md',
    'docs/upstream-funding.md',
    'docs/games.md', 'docs/family-profile.md', 'docs/veylith-arena.md',
]
NEUTRAL_HANDLES = {'Tracker01', 'DeviceName', 'Handle'}


def anchors(text):
    found, counts = set(), {}
    for heading in re.findall(r'^#{1,6}\s+(.+?)\s*#*$', text, re.M):
        slug = re.sub(r'[^\w\- ]', '', heading.lower()).replace(' ', '-')
        count = counts.get(slug, 0)
        counts[slug] = count + 1
        found.add(slug if count == 0 else f'{slug}-{count}')
    return found


def main():
    errors = []
    for name in GUIDES:
        path = ROOT / name
        if not path.is_file():
            errors.append(f'{name}: missing guide')
            continue
        text = path.read_text()
        for target in re.findall(r'!?\[[^\]\n]*\]\(([^)\s]+)\)', text):
            parts = urlsplit(target)
            if parts.scheme or parts.netloc:
                continue
            linked = (path.parent / unquote(parts.path)).resolve() if parts.path else path
            if not linked.exists():
                errors.append(f'{name}: missing link target {target}')
            elif parts.fragment and linked.suffix == '.md':
                if unquote(parts.fragment) not in anchors(linked.read_text()):
                    errors.append(f'{name}: missing heading {target}')
        # The inherited hardware reference can contain upstream syntax examples.
        if name != 'README-Solo.md':
            for handle in re.findall(r'@\[([^]\n]+)\]', text):
                if handle not in NEUTRAL_HANDLES:
                    errors.append(f'{name}: use a neutral device name in tracker examples')
    if errors:
        print('\n'.join(errors), file=sys.stderr)
        return 1
    print(f'Checked {len(GUIDES)} public guides: local links, headings and neutral examples pass.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
