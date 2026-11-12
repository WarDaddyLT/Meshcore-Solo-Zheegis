# Security policy

MeshCore Solo Žygis is currently a community preview. Security work targets the
current `main`; there is no separately maintained stable Žygis release series.

## Reporting a vulnerability

Use this repository's **Security → Report a vulnerability** option for private
reports. Include the affected commit, board/build environment, impact, and
reproduction steps. Do not include real channel secrets or personal locations.
For general setup questions use Discussions; ordinary bugs belong in Issues.

Relevant reports include private-channel validation bypasses, unintended location
exposure, crafted-packet memory errors, denial of service, and unsafe command
handling. Everyone with the selected channel secret can control the tracker;
that shared-key access model is documented behavior, not individual authentication.

Preserve upstream attribution when reporting shared code issues and coordinate
fixes with the relevant upstream maintainers. This volunteer project does not
promise response deadlines or a supported-release SLA.
