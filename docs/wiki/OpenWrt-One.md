# OpenWrt One GPIO

## Support level

OpenWrt One (AP-24.XY, MediaTek MT7981B) is source-backed and build-tested, but remains `limited` until the production image and CN7 are tested on hardware. Detection accepts the exact Device Tree identity `model = "OpenWrt One"` and `compatible = "openwrt,one"`.

The implementation does not alias BPI-R4 Lite. Both boards expose mikroBUS, but their SoCs, physical routes and GPIO line numbers differ. OpenWrt One selects the MT7981 GPIO register base `0x11d00000`; the existing R3/R4-family paths continue to use `0x1001f000`.

## CN7 mapping

`GPIO.BOARD` follows the standard 16-pin mikroBUS numbering. `GPIO.BCM` uses the native MT7981 GPIO line shown below.

| CN7 / `BOARD` | Function/net | MT7981 GPIO / `BCM` | Current policy |
| ---: | --- | ---: | --- |
| 1 | GND | — | Power, unavailable |
| 2 | +5 V | — | Power, unavailable |
| 3 | SDA / UART2_RTS | 7 | Digital GPIO or I2C/UART mux |
| 4 | SCL / UART2_CTS | 6 | Digital GPIO or I2C/UART mux |
| 5 | UART2_TXD | 4 | Digital GPIO or UART mux |
| 6 | UART2_RXD | 5 | Digital GPIO or UART mux |
| 7 | INT / WO_JTDI | 10 | Digital GPIO |
| 8 | PWM / WO_JTCK | 12 | Digital GPIO; hardware PWM not claimed |
| 9 | AN / AUXIN2 | — | Analog input, unavailable to this backend |
| 10 | RST / mikrobus-reset | 2 | Kernel DTS exports this line; treat as board-owned until approved |
| 11 | SPI1_CS | 25 | Digital GPIO or SPI mux |
| 12 | SPI1_CLK | 22 | Digital GPIO or SPI mux |
| 13 | SPI1_MISO | 24 | Digital GPIO or SPI mux |
| 14 | SPI1_MOSI | 23 | Digital GPIO or SPI mux |
| 15 | +3.3 V | — | Power, unavailable |
| 16 | GND | — | Power, unavailable |

## Evidence chain

- [Official OpenWrt One hardware source directory](https://one.openwrt.org/hardware/)
- [Official CN7/M.2/RTC KiCad schematic](https://one.openwrt.org/hardware/07_M2_KEYM_mikroBUS_EEPROM_RTC.kicad_sch), SHA256 `26aa741396e076904e6e9a220b3e0bb45d47a65423f615829bacb0ddd901c8a3`
- [Exact OpenWrt One DTS at audited OpenWrt commit](https://github.com/openwrt/openwrt/blob/4f2dc5cc6497a6cef0a43bdefde522e734c1f40d/target/linux/mediatek/dts/mt7981b-openwrt-one.dts)
- [MT7981 pinctrl groups and line names](https://github.com/torvalds/linux/blob/f2ec6312bf711369561bdcb22f8a63c0b118c479/drivers/pinctrl/mediatek/pinctrl-mt7981.c)
- [MT7981B DTSI and `pinctrl@11d00000`](https://github.com/torvalds/linux/blob/f2ec6312bf711369561bdcb22f8a63c0b118c479/arch/arm64/boot/dts/mediatek/mt7981b.dtsi)

## Validation still required

Capture the exact OpenWrt image, kernel/DTS commit, `model`, complete `compatible`, `gpiodetect`, `gpioinfo` and device permissions. A hardware owner must approve safe, unoccupied CN7 lines before any `GPIO.setup()` or output test. In particular, do not overwrite an active I2C/SPI/UART pinmux or the DTS-exported reset line while a mikroBUS card is attached.

Pull control is currently a no-op. Edge behavior, PWM behavior, cleanup and non-root access are not hardware-validated claims.
