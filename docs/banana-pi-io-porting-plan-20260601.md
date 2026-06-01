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
| BPI-F3 | SpacemiT K1 | done | Added K1 GPIO mmap backend from Dangku F3/SpacemiT reference; 40-pin map is from Dangku `bananapif3`; local extension build passed. Hardware PWM remains guarded pending hardware validation. |
| BPI-AI2N | Renesas RZ/V2N | done | Added RZ/V2N GPIO mmap backend from Dangku AI2N/Renesas reference; 40-pin map is from Dangku `bananapiai2n`; local extension build passed. Hardware PWM is out of scope for RPi.GPIO. |

### Batch C: Rockchip boards

| Board group | SoC family | Status | Notes |
| --- | --- | --- | --- |
| BPI-R2 Pro | RK3568 | done | Added RK3568 GPIO v2 mmap backend and 40-pin CON2 map from official Banana Pi R2 Pro GPIO table plus Armbian DTS. Basic GPIO input/output/read/write build-tested; pull remains no-op pending RK3568 GRF pinctrl hardware validation. |
| BPI-CM2 | RK3568 module | blocked | Official CM2 page describes a Raspberry Pi CM4-compatible module and says the dedicated CM2 base board was still being designed; Armbian currently uses an R2 Pro-derived DTS. Need a confirmed CM2 carrier/base-board 40-pin map before adding an alias. |
| BPI-M5 Pro / BPI-CM5 Pro | RK3576 | done | Added RK3576 selection to the Rockchip GPIO v2 mmap backend, M5 Pro aliases/map from the official Banana Pi M5 Pro 40-pin table plus Armbian `rk3576-bananapi-m5-pro`, and CM5 Pro aliases/map from the official Banana Pi CM5 Pro IO board 40-pin table plus Armbian `rk3576-armsom-cm5-io`. Basic GPIO input/output/read/write extension build-tested; pull remains no-op pending RK3576 GRF pinctrl hardware validation. M5 Pro pin 28/40 and CM5 Pro pin 16 use GPIO numbers derived from explicit function names where the official number cell is blank; CM5 Pro pin 28 follows the official GPIO number 109 despite a mixed function-cell label. |
| BPI-M7 | RK3588 | done | Added RK3588 selection to the Rockchip GPIO v2 mmap backend and 40-pin map from the official Banana Pi M7 GPIO table plus Armbian `rk3588-bananapi-m7`. Basic GPIO input/output/read/write extension build-tested; pull remains no-op pending RK3588 GRF pinctrl hardware validation. Pin 28 uses GPIO 149 derived from the explicit `GPIO4_C5` function because the official number cell is missing/misaligned. |
| BPI-W3 | RK3588 | done | Added W3 aliases/model detection and a W3-specific RK3588 40-pin map from the official Banana Pi W3 GPIO table plus Armbian `rk3588-bananapi-w3`. The W3 table matches M7 for most pins but leaves physical pin 37 unassigned, so this repo keeps pin 37 as non-GPIO instead of aliasing W3 to M7. Basic GPIO input/output/read/write extension build-tested; pull remains no-op pending RK3588 GRF pinctrl hardware validation. |
| BPI-AIM7 IO | RK3588 | alias | Added AIM7 aliases/model detection using the M7-compatible 40-pin map from the official BPI-AIM7 development kit table plus Armbian `rk3588-armsom-aim7-io`. Basic GPIO input/output/read/write extension build-tested; pull remains no-op pending RK3588 GRF pinctrl hardware validation. |
| BPI-LM7 | RK3588 module | blocked | Official LM7 documentation describes the LGA core module, not a fixed 40-pin header. Needs an exact carrier/baseboard GPIO map before adding an alias. |
| BPI-M4 Super | RK3568 | done | Added M4 Super aliases/model detection using the existing RK3568 Rockchip GPIO v2 mmap backend. The 40-pin map is from the official Banana Pi M4 Super GPIO table plus Armbian `rk3568-armsom-sige3`. Basic GPIO input/output/read/write extension build-tested; pull remains no-op pending RK3568 GRF pinctrl hardware validation. |
| BPI-M1 Super | RK3528 | done | Added RK3528 selection to the Rockchip GPIO v2 mmap backend and M1 Super aliases/model detection. The 40-pin map is from the ArmSoM Sige1-compatible official table plus Armbian `rk3528-armsom-sige1`; this avoids malformed GPIO number cells seen in the Banana Pi BPI-M1S page. Basic GPIO input/output/read/write extension build-tested; pull remains no-op pending RK3528 GRF pinctrl hardware validation. |
| BPI-Forge1 | RK3506J | done | Added RK3506 selection to the Rockchip GPIO v2 mmap backend and Forge1 aliases/model detection. The 40-pin map is compatible with the M1 Super/Sige1 map and comes from the official Banana Pi Forge1 GPIO table plus Armbian `rk3506b-armsom-forge1`. Basic GPIO input/output/read/write extension build-tested; pull remains no-op pending RK3506 GRF pinctrl hardware validation. |
| BPI-P2 Pro | RK3308 | done | Added RK3308 Rockchip GPIO v1 mmap backend using kernel/U-Boot v1 offsets (`DR=0x00`, `DDR=0x04`, `EXT=0x50`) and GPIO bank bases from Armbian `rk3308-bpi-p2-pro`. The 40-pin map is deliberately limited to official GPIO-capable pins 5-18; power/GND/ADC/audio/mic pins remain non-GPIO. Pull control remains no-op pending RK3308 pinctrl hardware validation. |

### Batch D: vendor or WIP boards

| Board group | SoC family | Status | Notes |
| --- | --- | --- | --- |
| BPI-W2 | Realtek RTD1296 | done | Added RTD129x mmap backend using Armbian/vendor GPIO bases (`MISC=0x9801b100`, `ISO=0x98007100`) and the official BPI-W2 40-pin GPIO table; IGPIO pins map to Linux GPIO `101 + N`. Pull control remains no-op pending Realtek pinctrl validation. |
| BPI-M4 plain | Realtek RTD1395 | done | Extended the Realtek backend for RTD1395 single ISO GPIO group (`0x98007100`, GPIO 0-56) and added the official BPI-M4 40-pin table. Pull control remains no-op pending Realtek pinctrl validation. |
| BPI-M6 | Synaptics VS680 | done | Added VS680 DW APB GPIO mmap backend from Armbian/vendor DTS (`gpio0/1/2` and `sm_gpio0`) and the official BPI-M6 CON3 40-pin table. SoC GPIO and SM_GPIO header pins are supported; header pins routed through the FXL6408 I2C expander remain non-GPIO in this mmap backend pending a separate gpiod/sysfs expander path. Local extension build passed. |
| BPI-F2S / BPI-F2P | Sunplus SP7021 | done | Added SP7021 GPIO mmap backend from Armbian/vendor pinctrl registers (`pctl@0x9C000100`, base0/base1/base2 register banks) and the F2S/F2P 40-pin map from the official schematics. Pull control remains no-op pending Sunplus pinctrl hardware validation. Local extension build passed. |
| BPI-CM6 | SpacemiT K1 | done | Added CM6-specific 26-pin IO board map from the official CM6 docs; reuses the K1 SpacemiT mmap backend from F3. Physical pin 12 is GPIO44, so CM6 is not aliased to F3. Local extension build passed. |
| BPI-SM10 | SpacemiT K3 | blocked | Official docs and local K3 DTS identify the board as `spacemit/k3_com260.dtb`, but no authoritative 40-pin expansion header map or schematic has been found in docs or SDK yet. Do not guess. |
| BPI-M2C | UniSoC UIS7885 | blocked | Armbian path is PAC/hybrid; userspace GPIO support depends on usable kernel GPIO exposure. |

### Batch E: routers, app products, and blocked families

| Board group | Status | Notes |
| --- | --- | --- |
| BPI-R4 | done | Added MT7988 GPIO v2 mmap backend from Armbian/kernel pinctrl registers (`pio@1001f000`) and the official CON2 26-pin GPIO map. Basic setup/input/output/read/write extension build-tested. Pull control remains no-op pending hardware validation. |
| BPI-R3 | done | Added MT7986 GPIO support by reusing the MTK GPIO v2 mmap backend (`pio@1001f000`) and the official R3 26-pin GPIO image (`r3_gpio_40.jpg`). Basic setup/input/output/read/write extension build-tested. Pull control remains no-op pending hardware validation. |
| BPI-R3 Mini | blocked | Official docs only show generic sysfs GPIO examples and board interfaces; no authoritative 26/40-pin expansion header map was found. Do not alias it to R3 without a confirmed carrier/header map. |
| BPI-R64 | done | Added MT7622 GPIO support using the official R64 40-pin GPIO image (`r64_gpio_40.jpg`) and mainline `pinctrl-mt7622` register ranges (`pinctrl@10211000`). Basic setup/input/output/read/write and pull-up/down extension build-tested. |
| BPI-R4 Lite | done | Added MT7987 GPIO v2 support by reusing the MTK v2 mmap backend (`pio@1001f000`) and the Armbian 6.17 `mt7987a-bananapi-bpi-r4-lite-mikrobus.dtsi` map. BOARD mode follows the 2x8 MikroBUS physical pins 1-16; GPIO-capable pins are 5/6/7/8/10/11/12/13/14 only. Pull control remains no-op pending hardware validation. |
| BPI-R4 Pro | done | Added MT7988 GPIO v2 support using the Armbian 6.17 `mt7988a-bananapi-bpi-r4-pro.dtsi` 26-pin map. The physical header matches the existing R4 map, but R4 Pro has its own board aliases/header so later 4e/8x differences can be adjusted independently. Detection now checks `/proc/device-tree/compatible` before the model string because the 4e/8x DTS model string is still generic `Bananapi BPI-R4`. Pull control remains no-op pending hardware validation. |
| BPI-R2 Mini / R4 Mini / OpenWrt One | blocked | Reviewed local Armbian tree on 2026-06-02: no R2 Mini/R4 Mini board target or DTS was found. OpenWrt One has MT7981B DTS files, but 6.17 disables the DTB and the available DTS only exposes board internals such as memory/LED/flash/UART; no authoritative external IO header map is present. Do not add aliases until board targets and header policy are available. |
| BPI-WiFi5 / WiFi6 / RT2 / RV2 | deferred | Router/OpenWrt image flows; no generic RPi.GPIO target until IO header requirement is defined. |
| BPI-F2 / F4 / F5 / S64 / Secure-Pi / SM9 / AI2H / Loongson boards | blocked | Need vendor BSP, kernel DTS, pinout, and/or new SoC backend before implementation. |

## Current Next Item

Continue Batch E router/service-header review from `BPI-WiFi5 / WiFi6 / RT2 / RV2`.

Resume sequence:

1. Add one board or exact carrier alias set at a time in both repos,
   build-test, commit, and push.
2. If a carrier map is not reliable from available sources,
   mark the row blocked with the exact missing item instead of guessing.
