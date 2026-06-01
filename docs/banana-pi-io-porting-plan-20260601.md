# Banana Pi IO Porting Plan

Date: 2026-06-01

Branch: `bpi-legacy-io-porting`

Repository role: Python `RPi.GPIO` compatible support.

Reference matrix:

- Armbian board matrix: `/media/pi/SMCI/armbian/bpi-v26.2.1/docs/bananapi-board-support-priority-20260601.md`
- Companion C library repo: `/media/pi/SMCI/bpi/BPI-WiringPi2`

## Goal

Bring Banana Pi board IO support up to the same board coverage plan used by the
Armbian tree. The first pass covers board detection aliases and 40-pin header
maps. Later passes add new SoC GPIO backends where a simple alias is not enough.

The supported IO surface is:

- `GPIO.BOARD` and `GPIO.BCM` numbering where applicable.
- Runtime board detection and model naming.
- GPIO input/output, pull, edge, and software PWM behavior already exposed by
  this package.
- Runtime GPIO register access or a documented blocker when a SoC backend is
  not yet present.

## Resume Rules

1. Check `git status --short` in both `BPI-WiringPi2` and `RPi.GPIO`.
2. Open this file and the matching plan in `BPI-WiringPi2`.
3. Continue from the first row whose status is `todo` or `in-progress`.
4. Work one board or one exact board-family alias set at a time.
5. For each board, update the plan row, build-test the repo, commit, and push.
6. Mirror shared board detection changes in `BPI-WiringPi2` before moving to the
   next board.
7. If no authoritative pinout, DTS, or compatible carrier source exists, mark
   the row `blocked` instead of guessing.

## Status Values

- `done`: code is present in this repo, local build checks pass, and the row
  records the implementation basis.
- `alias`: code reuses an existing map because the board is known or documented
  to share the same header wiring.
- `todo`: not yet implemented.
- `in-progress`: current work item.
- `blocked`: exact board files, pinout, or GPIO backend are missing.
- `deferred`: board is application-only, router firmware-only, or has no clear
  40-pin header target for these libraries.

## Per-Board Checklist

1. Source the mapping from one of:
   - official schematic or pinout table,
   - kernel DTS pinctrl/header notes,
   - vendor BSP board file,
   - existing Banana Pi board with documented identical carrier wiring.
2. Decide whether this is an alias or a new GPIO backend.
3. For an alias:
   - add all common model strings to `bpiboard[]` in `source/c_gpio_bpi.c`,
   - reuse the existing `pinToGpio`, `physToGpio`, and `pinTobcm` arrays,
   - do not add a new model id unless behavior must differ.
4. For a new SoC backend:
   - add model id and model name in `source/c_gpio_bpi.c`,
   - add map arrays under `source/bpi-*.h`,
   - include the new map from `source/bpi_gpio.h`,
   - implement or select GPIO read/write/mode/pull/event backends,
   - confirm kernel GPIO numbering on Armbian.
5. Build checks:
   - `python3 setup.py build_ext --inplace`
6. Hardware checks when available:
   - `python3 -c 'import RPi.GPIO as GPIO; print(GPIO.RPI_INFO)'`
   - one safe output pin blink through `GPIO.BOARD`,
   - one safe output pin blink through `GPIO.BCM`,
   - one input pin read with pull-up/down if supported,
   - edge event test on a safe input pin.

## Implementation Order

### Batch A: legacy aliases and already-compatible boards

| Board or alias set | Basis | Status | Notes |
| --- | --- | --- | --- |
| BPI-M1 / BPI-M1 Plus / BPI-R1 | existing A20 map | done | Existing `BPI_MODEL_M1/M1P/R1` entries. |
| BPI-Pro | A20 Banana Pro, same GPIO layout as Banana Pi/Pro family | alias | Added `bpi-pro`, `banana-pro`, `bananapro`, `bananapi-pro`, and `bananapipro` aliases using the existing M1 Plus map; local build check passed. |
| BPI-M2 | existing A31s map | done | Existing `BPI_MODEL_M2`. |
| BPI-M2 Plus H3/H2+/H5 | existing M2 Plus map | done | Aliases already present. |
| BPI-M2 Ultra / BPI-M2 Berry | existing R40/V40 map | done | Aliases already present. |
| BPI-6204 / BPI-CS6204 | R40/M2 Ultra-compatible industrial BSP | alias | Added `bpi-6204`, `bpi-cs6204`, and `bpi-cs-6204` aliases using the existing M2 Ultra/V40 map; local build check passed. |
| BPI-6202 / BPI-CS6202 | CS6202/CS6204-compatible BSP direction | alias | Added `bpi-6202`, `bpi-cs6202`, and `bpi-cs-6202` aliases using the existing M2 Ultra/V40 map; no new model id; local build check passed. |
| BPI-M2 Magic / v1.1 | existing R16 maps | done | Separate v1.1 map exists. |
| BPI-M2 Zero / BPI-P2 Zero | existing H2+/H3 map | done | Aliases already present. |
| BPI-M3 | existing A83T map | done | Existing `BPI_MODEL_M3`. |
| BPI-M64 | existing A64 map | done | Existing `BPI_MODEL_M64`. |
| BPI-R2 | existing MT7623 backend | done | MTK GPIO path exists. |

### Batch B: Armbian normal boards without RPi.GPIO support yet

| Board group | SoC family | Status | Notes |
| --- | --- | --- | --- |
| BPI-M4 Berry | Allwinner H618 | done | Added initial H618 GPIO mmap path, board aliases, DT model detection, and 40-pin GPIO map from official BPI docs/Dangku reference; local extension build passed. |
| BPI-M4 Zero | Allwinner H618 | done | Added H618 GPIO map, board aliases, DT model detection, and local extension build checks. |
| BPI-M2S | Amlogic G12B | done | Added first Meson GPIO mmap backend, M2S aliases/model detection, and 40-pin map from Armbian DTS plus Dangku Amlogic reference maps; local extension build passed. |
| BPI-CM4IO | Amlogic G12B | done | Reuses Meson backend from M2S; added CM4IO-specific carrier map from Dangku `bananapicm4`/legacy BSP, separate from `BPI-RPICM4`; local extension build passed. |
| BPI-M5 | Amlogic SM1 | done | Reuses Meson backend; 40-pin map is from Dangku `bananapim5` and Armbian `meson-sm1-bananapi-m5`; local extension build passed. |
| BPI-M2 Pro | Amlogic SM1 | alias | M5-compatible alias per Dangku M5/M2Pro handling and Armbian SM1 overlays; uses separate model id/name; local extension build passed. |
| BPI-F3 | SpacemiT K1 | todo | Needs K1 GPIO backend and official header map. |
| BPI-AI2N | Renesas RZ/V2N | todo | Needs RZ/V2N GPIO backend and Banana Pi header map. |

### Batch C: Rockchip boards

| Board group | SoC family | Status | Notes |
| --- | --- | --- | --- |
| BPI-R2 Pro / BPI-CM2 | RK3568 | todo | Needs Rockchip GPIO backend and per-carrier maps. |
| BPI-M5 Pro / BPI-CM5 Pro | RK3576 | todo | Needs RK35xx backend and maps. |
| BPI-M7 / W3 / AIM7 / LM7 | RK3588 | todo | Needs RK3588 backend and carrier map separation. |
| BPI-M1 Super / M4 Super / Forge1 / P2 Pro | RK3528/RK3568/RK3506/RK3308 | todo | Needs per-SoC backend review. |

### Batch D: vendor or WIP boards

| Board group | SoC family | Status | Notes |
| --- | --- | --- | --- |
| BPI-W2 / BPI-M4 plain | Realtek RTD129x/RTD139x | todo | Needs legacy Realtek GPIO backend or sysfs-only policy. |
| BPI-M6 | Synaptics VS680 | todo | Needs VS680 GPIO backend and header map. |
| BPI-F2S / BPI-F2P | Sunplus SP7021 | todo | Needs SP7021 GPIO backend and header map. |
| BPI-CM6 / BPI-SM10 | SpacemiT K1/K3 | todo | Reuse K1/K3 work after F3/SM10 source validation. |
| BPI-M2C | UniSoC UIS7885 | blocked | Armbian path is PAC/hybrid; userspace GPIO support depends on usable kernel GPIO exposure. |

### Batch E: routers, app products, and blocked families

| Board group | Status | Notes |
| --- | --- | --- |
| BPI-R3 / R3 Mini / R64 / R4 / R4 Lite / R4 Pro | todo | Add only if the board exposes a documented 40-pin or service GPIO header useful to RPi.GPIO. |
| BPI-R2 Mini / R4 Mini / OpenWrt One | blocked | Armbian target is blocked/deferred; wait for board files and IO header policy. |
| BPI-WiFi5 / WiFi6 / RT2 / RV2 | deferred | Router/OpenWrt image flows; no generic RPi.GPIO target until IO header requirement is defined. |
| BPI-F2 / F4 / F5 / S64 / Secure-Pi / SM9 / AI2H / Loongson boards | blocked | Need vendor BSP, kernel DTS, pinout, and/or new SoC backend before implementation. |

## Current Next Item

Continue Batch B from `BPI-M5` / `BPI-M2 Pro`.

Resume sequence:

1. Confirm the `BPI-M5` and `BPI-M2 Pro` SM1 maps against Dangku and Armbian
   board strings.
2. Add `BPI-M5` in `BPI-WiringPi2`, build-test, commit, and push.
3. Add `BPI-M5` in this repo, build-test, commit, and push.
4. Repeat for `BPI-M2 Pro` if its carrier map differs; otherwise add it as a
   documented alias in both repos.
