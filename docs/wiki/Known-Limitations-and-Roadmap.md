# Known Limitations and Roadmap

## General limitations

- Many newer backends have source-backed basic digital I/O but incomplete pull or edge validation.
- `RPi.GPIO` provides software PWM; do not describe it as verified hardware PWM.
- Direct register access depends on SoC register layout, page mapping and kernel/device permissions.
- GPIO expanders, EC/MCU/RT-core pins and isolated industrial IO may require libgpiod or a device-specific API.
- Carrier/module confusion is a high-risk failure mode; an incorrect `GPIO.BOARD` map can drive power, boot or board-owned signals.
- The upstream package metadata and legacy documentation still contain Raspberry Pi-specific wording; Banana Pi behavior must be checked against this Wiki and the porting plan.

## Current roadmap

### Newly implemented, hardware validation required

- BPI-SM10 / SpacemiT K3: detection, J12 map and K3 GPIO direction/read/write/pull paths build in the native extension; physical pin 18 is deliberately unavailable.
- BPI-CanMV-K230D Zero / Kendryte K230D: detection, JP1 map, two-bank direction/read/write and IOMUX pull paths build in the native extension; physical pin 16 is deliberately unavailable.
- OpenWrt One / MediaTek MT7981B: exact detection, CN7 mikroBUS map and the board-specific `0x11d00000` GPIO-base selection build in the native extension; pull, edge/PWM, permissions and real hardware remain unverified.
- K3 Pico-ITX / SpacemiT K3: exact detection and the eight application-CPU GPIOs on the 3.3 V 26-pin FPC build in the native extension; RT24-owned pins and the complete 1.8 V 36-pin FPC remain unavailable by design. Pull, edge/PWM, permissions and real hardware remain unverified.

### Implemented but hardware validation required

BPI-F4 / Sunplus SP7350 has basic direction/read/write support and a terminal-row map. Pull, exact production detection identity and real hardware behavior remain unverified. SM10, K230D Zero, OpenWrt One and K3 Pico-ITX have stronger public source identity, but still require gpioinfo plus safe input/output tests on exact production images before their capability claims can be widened.

### Needs exact internal board evidence

BPI-M2C, F2, F5, 2K0300, CM2+CM4IO, CM5+CM4IO, S64 Core kit and AI2H carrier.

### Needs product scope decision

BPI-5202 module bus and BPI-2K3000 isolated IO.

R4 Mini and the standalone RK3588 Stamp-hole/Gold-finger core modules are now explicitly `out-of-scope`: the published products do not define a fixed raw user-GPIO connector for these Pi-style libraries. A separately named carrier with authoritative mapping can be evaluated as a future target.

## Definition of done

A board is not complete until:

- exact board/carrier identity and authoritative mapping are recorded;
- detection and `BOARD`/`BCM` maps are consistent across both GPIO repositories;
- the native extension builds and imports;
- safe hardware digital I/O passes on a named image;
- every untested pull/edge/PWM function remains visibly limited;
- documentation and the support matrix are updated in the same change.
