# Known Limitations and Roadmap

## General limitations

- Many newer backends have source-backed basic digital I/O but incomplete pull or edge validation.
- `RPi.GPIO` provides software PWM; do not describe it as verified hardware PWM.
- Direct register access depends on SoC register layout, page mapping and kernel/device permissions.
- GPIO expanders, EC/MCU/RT-core pins and isolated industrial IO may require libgpiod or a device-specific API.
- Carrier/module confusion is a high-risk failure mode; an incorrect `GPIO.BOARD` map can drive power, boot or board-owned signals.
- The upstream package metadata and legacy documentation still contain Raspberry Pi-specific wording; Banana Pi behavior must be checked against this Wiki and the porting plan.

## Current roadmap

### Software-startable

1. BPI-F4 / Sunplus SP7350.
2. BPI-SM10 / SpacemiT K3.
3. BPI-CanMV-K230D Zero / Canaan K230D.

### Needs exact internal board evidence

BPI-M2C, F2, F5, 2K0300, CM2+CM4IO, CM5+CM4IO, RK3588 Stamp-hole/Gold-finger carriers, S64 Core kit and AI2H carrier.

### Needs product scope decision

K3 Pico-ITX FPC/RT24, OpenWrt One mikroBUS, R4 Mini, 5202 module bus and 2K3000 isolated IO.

## Definition of done

A board is not complete until:

- exact board/carrier identity and authoritative mapping are recorded;
- detection and `BOARD`/`BCM` maps are consistent across both GPIO repositories;
- the native extension builds and imports;
- safe hardware digital I/O passes on a named image;
- every untested pull/edge/PWM function remains visibly limited;
- documentation and the support matrix are updated in the same change.
