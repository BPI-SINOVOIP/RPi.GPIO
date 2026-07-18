# Board Support Matrix

Status date: 2026-07-18

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

The normalized Banana Pi catalog has 110 products: 14 `implemented`, 9 `alias`, 18 `limited`, 6 `carrier-only`, 3 `base-covered`, 15 `blocked`, 9 `deferred`, 5 `needs-review` and 31 `out-of-scope`. Customer-private boards are excluded.

## Code-backed families

The repository contains Banana Pi model paths for these major families:

- Legacy Allwinner: M1/M1 Plus/R1, M2, M2 Plus variants, M2 Ultra/Berry, M2 Magic, M2 Zero/P2 Zero, M3, M64.
- Newer Allwinner: M4 Berry and M4 Zero.
- Amlogic: M2S, CM4IO carrier, M5 and M2 Pro.
- SpacemiT K1: F3 and CM6 IO carrier.
- Renesas RZ/V2N: AI2N carrier path.
- Rockchip: R2 Pro, M5 Pro/CM5 Pro IO, M7, W3, AIM7, M4 Super, M1 Super, Forge1 and P2 Pro.
- Realtek/Synaptics/Sunplus: W2, M4, M6, F2S and F2P.
- MediaTek: R2, R3, R4, R4 Lite, R4 Pro, R64.

This list says a code path exists. Read the per-board row in the [porting plan](https://github.com/BPI-SINOVOIP/RPi.GPIO/blob/bpi-legacy-io-porting/docs/banana-pi-io-porting-plan-20260601.md) for capability and validation limits.

## Current software-startable work

1. BPI-F4 — implement SP7350 detection, mapping and Python backend selection using the official terminal map/Q654 source; then obtain exact DTS/gpioinfo/hardware logs.
2. BPI-SM10 — finish signal-to-controller mapping for the official 40-pin carrier and exact K3 DTS, then implement/test the Python path.
3. BPI-CanMV-K230D Zero — derive Linux gpiochip/offset from the official 40-pin/SDK evidence and implement without guessing runtime numbering.

## Still blocked by exact internal evidence

BPI-M2C, BPI-F2, BPI-F5, BPI-2K0300, BPI-CM2+CM4IO, BPI-CM5+CM4IO, RK3588 Stamp-hole/Gold-finger carriers, BPI-S64 Core kit and BPI-AI2H carrier require an exact board package or runtime evidence.

## Scope decisions

- BPI-LM7 is covered through the W3 development kit; only exact LM7+W3 detection evidence is needed.
- BPI-R2 Mini and BPI-R3 Mini have no Pi-style library target under the current evidence.
- K3 Pico-ITX, OpenWrt One, BPI-R4 Mini, BPI-5202 and BPI-2K3000 require an explicit connector/library policy before support is promised.

Never infer support from the SoC name, physical dimensions or a sibling-board DTS.
