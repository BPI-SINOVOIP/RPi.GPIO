# Documentation Maintenance

## Information architecture

- `README.md`: short project identity, install pointer and Wiki link.
- GitHub Wiki: user guides, Python examples, concepts, support policy, porting and validation workflows.
- `docs/banana-pi-io-porting-plan-20260601.md`: detailed implementation evidence and active engineering queue.
- Code: final truth for detection aliases, maps and backend selection.
- Hardware logs: final truth for runtime validation; never replace them with documentation claims.

## Update workflow

For every new board or support change:

1. Add/normalize the public product name and aliases.
2. Decide `support`, `carrier-only`, `base-covered`, `scope-review` or `out-of-scope` before coding.
3. Link official schematic/pinout/DTS/BSP evidence.
4. Update the implementation plan and both repositories together.
5. Build/import and capture exact hardware evidence.
6. Update `Board-Support-Matrix.md`, limitations and roadmap.
7. Review files under `docs/wiki/`, then publish the same commit set to the Wiki repository.

## Drift controls

- The two repositories must have the same effective Banana Pi model IDs, detection aliases and shared physical/BCM maps unless an API-specific difference is explicitly documented.
- Run a set comparison after any detection change.
- Date support snapshots; do not silently overwrite the meaning of an old test.
- Use canonical product ID as the key and keep marketing names/Device Tree strings as aliases.
- Never remove a blocked or out-of-scope board from the catalog merely because it has no implementation.

## Wiki publishing

GitHub Wikis are separate Git repositories. Only the default branch is rendered. The reviewed files in `docs/wiki/` are copied to `RPi.GPIO.wiki.git`; `docs/wiki/README.md` stays in the main repository as maintainer instructions.

Before publishing:

- verify every relative Wiki link;
- execute Python examples that do not switch hardware and syntax-check the rest;
- confirm no internal filesystem paths, confidential artifacts or private board names are present;
- preview `Home`, `_Sidebar`, tables and code blocks on GitHub;
- keep the companion Wiki cross-links valid.

## Review cadence

- On every board-port PR.
- On every release or default-image change.
- After the official Banana Pi product catalog changes.
- At least quarterly for broken links, stale support claims and deferred/out-of-scope decisions.
