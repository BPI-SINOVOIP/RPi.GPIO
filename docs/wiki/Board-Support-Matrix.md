# Board Support Matrix

Status date: 2026-07-19

## How to read this page

| Status | Meaning |
| --- | --- |
| `implemented` | Detection, pin map and a selectable backend exist; at least local extension-build evidence is recorded. |
| `alias` | A documented compatible board/carrier reuses an existing map/backend. |
| `limited` | Code exists, but hardware validation or capabilities such as pull/edge/PWM are incomplete. |
| `carrier-only` | The module has no fixed user header; support belongs to one exact carrier/development kit. |
| `base-covered` | The product uses an already supported base board and needs no independent pin map. |
| `blocked` | Authoritative mapping, exact carrier/DTS or backend evidence is missing. |
| `deferred` | The product is known but has no stable library target policy yet. |
| `needs-review` | The product exists but the two GPIO repositories still need an explicit support decision. |
| `out-of-scope` | `RPi.GPIO` is not the correct interface for the product. |

The normalized Banana Pi catalog has 110 products: 14 `implemented`, 9 `alias`, 22 `limited`, 6 `carrier-only`, 3 `base-covered`, 12 `blocked`, 9 `deferred`, 4 `needs-review` and 31 `out-of-scope`. Customer-private boards are excluded.

## Code-backed families

The repository contains Banana Pi model paths for these major families:

- Legacy Allwinner: M1/M1 Plus/R1, M2, M2 Plus variants, M2 Ultra/Berry, M2 Magic, M2 Zero/P2 Zero, M3, M64.
- Newer Allwinner: M4 Berry and M4 Zero.
- Amlogic: M2S, CM4IO carrier, M5 and M2 Pro.
- SpacemiT: F3/CM6 IO on K1 and SM10/K3-CoM260 on K3.
- Kendryte/Canaan K230D: BPI-CanMV-K230D Zero.
- Renesas RZ/V2N: AI2N carrier path.
- Rockchip: R2 Pro, M5 Pro/CM5 Pro IO, M7, W3, AIM7, M4 Super, M1 Super, Forge1 and P2 Pro.
- Realtek/Synaptics/Sunplus: W2, M4, M6, F2S, F2P and F4.
- MediaTek: R2, R3, R4, R4 Lite, R4 Pro, R64 and OpenWrt One.

This list says a code path exists. Read the per-board row in the [porting plan](https://github.com/BPI-SINOVOIP/RPi.GPIO/blob/bpi-legacy-io-porting/docs/banana-pi-io-porting-plan-20260601.md) for capability and validation limits.

## Newly code-backed, awaiting hardware validation

- BPI-SM10 now has four detection names, a 27-line J12 map traced through the official carrier schematic, and a K3 register variant of the SpacemiT backend. Published physical pin 18 remains unavailable. The native extension builds; exact image identity, gpioinfo and safe hardware tests are still required.
- BPI-CanMV-K230D Zero now has four detection names, a 27-line JP1 map from the official table, and a source-backed K230D two-bank GPIO/IOMUX backend. Physical pin 16 remains unavailable because the official row has no GPIO number. The native extension builds; real board behavior, edge/PWM and device permissions remain unverified.
- OpenWrt One now has exact model/compatible detection, an 11-line CN7 mikroBUS map closed through the official KiCad and upstream MT7981 pinctrl data, and board-specific selection of the exact DTS GPIO base `0x11d00000`. CN7 pin 9 (`AN`) remains outside the digital backend. The native extension builds; pull, edge/PWM, permissions and real hardware remain unverified.

## Hardware-validation work

BPI-F4, BPI-SM10, BPI-CanMV-K230D Zero and OpenWrt One all remain `limited` until named production images and safe real-hardware digital-I/O logs exist. F4 and OpenWrt One pull are no-ops; SM10 and K230D pull logic is source-backed but untested; edge and PWM behavior is not claimed as hardware-validated.

## Still blocked by exact internal evidence

BPI-M2C, BPI-F2, BPI-F5, BPI-2K0300, BPI-CM2+CM4IO, BPI-CM5+CM4IO, RK3588 Stamp-hole/Gold-finger carriers, BPI-S64 Core kit and BPI-AI2H carrier require an exact board package or runtime evidence.

## Scope decisions

- BPI-LM7 is covered through the W3 development kit; only exact LM7+W3 detection evidence is needed.
- BPI-R2 Mini and BPI-R3 Mini have no Pi-style library target under the current evidence.
- K3 Pico-ITX, BPI-R4 Mini, BPI-5202 and BPI-2K3000 require an explicit connector/library policy before support is promised.

Never infer support from the SoC name, physical dimensions or a sibling-board DTS.
