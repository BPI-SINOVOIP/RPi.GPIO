# Complete Banana Pi Board Catalog

Status date: 2026-07-19

<!-- catalog-status-counts: implemented=14 alias=9 limited=23 carrier-only=6 base-covered=3 blocked=11 deferred=9 needs-review=1 out-of-scope=34 -->

This is the normalized public product inventory used by both Banana Pi GPIO repositories. It records every reviewed product even when this library is not the correct interface. Customer-private products are intentionally excluded.

The catalog contains 110 products: 14 `implemented`, 9 `alias`, 23 `limited`, 6 `carrier-only`, 3 `base-covered`, 11 `blocked`, 9 `deferred`, 1 `needs-review` and 34 `out-of-scope`. See [Board Support Matrix](Board-Support-Matrix) for status definitions and engineering limits.

IDs `L01`-`L82` cover Linux boards, modules, routers and application products. IDs `O01`-`O28` preserve reviewed MCU, control, camera, radio and STEAM products that do not belong in these Pi-style Linux GPIO libraries.

| ID | Product | SoC / platform / type | Status |
| --- | --- | --- | --- |
| L01 | BPI-M1 | Allwinner A20 | `implemented` |
| L02 | BPI-M1 Plus / BPI-M1+ | Allwinner A20 | `implemented` |
| L03 | Banana Pro / BPI-Pro | Allwinner A20 | `alias` |
| L04 | BPI-M2 | Allwinner A31s | `implemented` |
| L05 | BPI-M2 Plus (H3/H2+/H5) | Allwinner H3/H2+/H5 | `implemented` |
| L06 | BPI-M2 Zero | Allwinner H2+/H3 | `implemented` |
| L07 | BPI-M2 Magic / BPI-M2M | Allwinner A33/R16 | `implemented` |
| L08 | BPI-M2 Berry / BPI-M2B | Allwinner R40/V40/A40i | `alias` |
| L09 | BPI-M2 Ultra / BPI-M2U | Allwinner R40/V40/A40i | `implemented` |
| L10 | BPI-M2 Pro | Amlogic S905X3 / SM1 | `alias` |
| L11 | BPI-M2S | Amlogic A311D/S922X / G12B | `implemented` |
| L12 | BPI-M2C | UNISOC UIS7885 | `blocked` |
| L13 | BPI-M3 | Allwinner A83T | `implemented` |
| L14 | BPI-M4 | Realtek RTD1395 | `limited` |
| L15 | BPI-M4 Berry | Allwinner H618 | `limited` |
| L16 | BPI-M4 Zero | Allwinner H618 | `limited` |
| L17 | BPI-M4 Super / BPI-M4S | Rockchip RK3568 | `limited` |
| L18 | BPI-M5 | Amlogic S905X3 / SM1 | `implemented` |
| L19 | BPI-M5 Pro | Rockchip RK3576 | `limited` |
| L20 | BPI-M6 | Synaptics VS680/SL1680 | `limited` |
| L21 | BPI-M7 | Rockchip RK3588 | `limited` |
| L22 | BPI-M64 | Allwinner A64 | `implemented` |
| L23 | BPI-M1 Super / BPI-M1S | Rockchip RK3528 | `limited` |
| L24 | BPI-P2 Zero | Allwinner H2+/H3/H5 | `alias` |
| L25 | BPI-P2 Maker | Allwinner H2+/H3 | `out-of-scope` |
| L26 | BPI-P2 Pro | Rockchip RK3308 | `limited` |
| L27 | BPI-F2 | NXP/Freescale i.MX6 | `blocked` |
| L28 | BPI-F2S | Sunplus SP7021 | `limited` |
| L29 | BPI-F2P | Sunplus SP7021 | `alias` |
| L30 | BPI-F3 | SpacemiT K1 | `limited` |
| L31 | BPI-F4 | Sunplus SP7350 | `limited` |
| L32 | BPI-F5 | Allwinner T527 | `blocked` |
| L33 | BPI-Forge1 | Rockchip RK3506J | `alias` |
| L34 | BPI-CanMV-K230D Zero | Canaan/Kendryte K230D | `limited` |
| L35 | BPI-2K0300 | Loongson 2K0300 | `blocked` |
| L36 | BPI-R1 | Allwinner A20 | `alias` |
| L37 | BPI-R2 | MediaTek MT7623N | `implemented` |
| L38 | BPI-R2 Pro | Rockchip RK3568 | `limited` |
| L39 | BPI-R2 Mini | MediaTek MT7981B | `out-of-scope` |
| L40 | BPI-R3 | MediaTek MT7986 | `limited` |
| L41 | BPI-R3 Mini | MediaTek MT7986 | `out-of-scope` |
| L42 | BPI-R4 | MediaTek MT7988A | `limited` |
| L43 | BPI-R4 Lite | MediaTek MT7987 | `limited` |
| L44 | BPI-R4 Pro (4E/8X) | MediaTek MT7988A | `limited` |
| L45 | BPI-R4 Mini | MediaTek MT7987 | `out-of-scope` |
| L46 | BPI-R64 | MediaTek MT7622 | `implemented` |
| L47 | BPI-W2 | Realtek RTD1296 | `limited` |
| L48 | BPI-W3 | Rockchip RK3588 | `limited` |
| L49 | BPI-WiFi5 Router | Siflower SF19A2890S | `deferred` |
| L50 | BPI-WiFi6 Router | Triductor TR6560/TR5220 | `deferred` |
| L51 | BPI-WiFi6 Mini | Triductor TR6560/TR5220 | `deferred` |
| L52 | BPI-RT2 (AX3000) | Realtek RTL8198D | `deferred` |
| L53 | BPI-RV2 Gateway | Siflower SF21H8898 RISC-V | `deferred` |
| L54 | OpenWrt One / AP-24.XY | MediaTek MT7981B | `limited` |
| L55 | BPI-MNF | MediaTek MT7622E | `deferred` |
| L56 | BPI-CM2 | Rockchip RK3568 | `blocked` |
| L57 | BPI-CM4 | Amlogic A311D / G12B | `carrier-only` |
| L58 | BPI-CM4IO | Amlogic A311D / G12B | `implemented` |
| L59 | BPI-CM5 | Amlogic A311D2 | `needs-review` |
| L60 | BPI-CM5 Pro | Rockchip RK3576 | `carrier-only` |
| L61 | BPI-CM6 | SpacemiT K1 | `carrier-only` |
| L62 | BPI-RK3588 LGA Core Board and Development Kit | Rockchip RK3588 | `base-covered` |
| L63 | BPI-RK3588 Stamp-hole Core Board | Rockchip RK3588 | `out-of-scope` |
| L64 | BPI-RK3588 Gold-finger Core Board | Rockchip RK3588 | `out-of-scope` |
| L65 | BPI-LM7 Core Board | Rockchip RK3588 | `base-covered` |
| L66 | BPI-AIM7 | Rockchip RK3588 | `carrier-only` |
| L67 | BPI-S64 Core | Actions S700 | `blocked` |
| L68 | BPI-Secure-Pi | MegaHunt SP2302/MH1905 | `blocked` |
| L69 | BPI-SM9 | SOPHGO BM1688 | `blocked` |
| L70 | BPI-SM10 | SpacemiT K3 / K3-CoM260 | `limited` |
| L71 | BPI-AI2N | Renesas RZ/V2N | `carrier-only` |
| L72 | BPI-AI2N-Carrier | Renesas RZ/V2N | `carrier-only` |
| L73 | BPI-AI2H | Renesas RZ/V2H | `blocked` |
| L74 | K3 Pico-ITX | SpacemiT K3 | `limited` |
| L75 | BPI-6202 / BPI-CS6202 | Allwinner A40i-H/R40 | `alias` |
| L76 | BPI-6204 / BPI-CS6204 | Allwinner A40i-H/R40 | `alias` |
| L77 | BPI-KVM | Rockchip RK3568 | `deferred` |
| L78 | BPI-OM7 3D Camera | RK3588 + Orbbec Gemini 2 | `base-covered` |
| L79 | BPI-5202 | Loongson 2K1000LA | `blocked` |
| L80 | BPI-2K3000 | Loongson 2K3000 | `blocked` |
| L81 | BPI-3A5000 | Loongson 3A5000 | `deferred` |
| L82 | BPI-3A6000 | Loongson 3A6000 | `deferred` |
| O01 | BPI-FSM1819D / FSM8191D on product page | Fortior servo controller | `out-of-scope` |
| O02 | BPI-VP10 | Fortior FU7512L servo controller | `out-of-scope` |
| O03 | BPI-D1 | Anyka AK3918 camera board | `out-of-scope` |
| O04 | BPI-D2 | Rockchip RV1126 AI camera | `out-of-scope` |
| O05 | BPI-G1 | STM32F103 + CC3200/CC2530/CC2540 | `out-of-scope` |
| O06 | BPI-EAI80 | Edgeless/Gree EAI80 MCU/NPU | `out-of-scope` |
| O07 | BPI-R18-AI | Allwinner R18 voice dev kit | `out-of-scope` |
| O08 | BPI-AI-Voice | Microsemi voice kit | `out-of-scope` |
| O09 | AIWorld P1 | AI development product | `out-of-scope` |
| O10 | BPI-AI / BPI-K210 | Kendryte K210 RISC-V MCU/AI | `out-of-scope` |
| O11 | BPI-Centi-S3 | ESP32-S3 | `out-of-scope` |
| O12 | BPI-PicoW-S3 | ESP32-S3 | `out-of-scope` |
| O13 | BPI-Leaf-S3 | ESP32-S3 | `out-of-scope` |
| O14 | BPI-Pico-RP2040 | RP2040 | `out-of-scope` |
| O15 | BPI-Bit | ESP32 | `out-of-scope` |
| O16 | BPI-Bit-S2 | ESP32-S2 | `out-of-scope` |
| O17 | BPI-Smart | ESP8266 | `out-of-scope` |
| O18 | BPI-NANO | ATmega328P | `out-of-scope` |
| O19 | BPI-UNO | ATmega328P | `out-of-scope` |
| O20 | BPI-UNO32 | ESP32 | `out-of-scope` |
| O21 | BPI-QCar | STEAM robot board/kit | `out-of-scope` |
| O22 | BPI-LoRA | Arduino/LoRa board | `out-of-scope` |
| O23 | BPI NB-IoT Linaro 96Boards | NB-IoT board | `out-of-scope` |
| O24 | BPI-CC2650 Zigbee BT 96Boards | TI CC2650 | `out-of-scope` |
| O25 | BPI NB-BC95 | NB-IoT board | `out-of-scope` |
| O26 | BPI Z-Wave Gateway | Z-Wave IoT board/module | `out-of-scope` |
| O27 | BPI NB-IOT Arduino | Arduino/NB-IoT board | `out-of-scope` |
| O28 | BPI-Nano Robot Board | Arduino Nano carrier/control board | `out-of-scope` |

## Maintenance and safety

Catalog coverage, code coverage and hardware validation are separate claims. A code-backed board may remain `limited`; a module may be usable only through one named carrier; and a product stays listed when it is blocked or out of scope.

Do not infer a map from a sibling board or SoC. New support requires an exact board/carrier identity, authoritative connector map, Device Tree or equivalent GPIO-controller evidence, and safe real-hardware validation.

Maintainers should run:

```sh
python3 tools/audit-bpi-support.py
python3 tools/audit-bpi-support.py --peer ../BPI-WiringPi2
```

The first command validates the local model table, display names, detection ordering, all referenced 64-entry maps and this 110-row catalog. The second also requires both repositories to have identical models, aliases, detection patterns and shared map values.
