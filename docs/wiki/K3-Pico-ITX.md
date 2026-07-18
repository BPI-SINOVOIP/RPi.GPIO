# K3 Pico-ITX GPIO

## Support level

K3 Pico-ITX is source-backed and extension-build-tested, but remains `limited` until an exact production image and the 26-pin FPC are tested on hardware. Detection accepts the exact Device Tree identity `model = "SpacemiT K3 Pico ITX"` and `compatible = "spacemit,k3-pico-itx"`.

This is intentionally not a general mapping of both FPC connectors. Only eight 3.3 V signals on the 26-pin connector are controlled by the application K3 GPIO block and exposed by the library. RT24-owned signals and the entire 1.8 V 36-pin connector remain unavailable.

## Supported 26-pin FPC subset

`GPIO.BOARD` uses the official 26-pin FPC positions. `GPIO.BCM` uses the native K3 global GPIO/pad number.

| BOARD | Official signal | Owner | Voltage | K3 GPIO / BCM | Current policy |
| ---: | --- | --- | ---: | ---: | --- |
| 1 | 3.3 V | Power | 3.3 V | — | Unavailable |
| 2 | 3.3 V | Power | 3.3 V | — | Unavailable |
| 3 | R_I2C1_SCL | RT24 | 3.3 V | — | Unavailable |
| 4 | R_I2C1_SDA | RT24 | 3.3 V | — | Unavailable |
| 5 | GND | Power | — | — | Unavailable |
| 6 | R_UART0_TX | RT24 | 3.3 V | — | Unavailable |
| 7 | R_UART0_RX | RT24 | 3.3 V | — | Unavailable |
| 8 | GND | Power | — | — | Unavailable |
| 9 | UART5_TX | Application K3 | 3.3 V | 21 | Digital GPIO or UART mux |
| 10 | UART5_RX | Application K3 | 3.3 V | 22 | Digital GPIO or UART mux |
| 11 | R-PWM1 | Application K3 | 3.3 V | 28 | Digital GPIO; hardware PWM not claimed |
| 12 | R-PWM2 | Application K3 | 3.3 V | 29 | Digital GPIO; hardware PWM not claimed |
| 13 | UART10_TX | Application K3 | 3.3 V | 31 | Digital GPIO or UART mux |
| 14 | UART10_RX | Application K3 | 3.3 V | 32 | Digital GPIO or UART mux |
| 15 | UART10_CTS | Application K3 | 3.3 V | 33 | Digital GPIO or UART mux |
| 16 | UART10_RTS | Application K3 | 3.3 V | 34 | Digital GPIO or UART mux |
| 17 | GND | Power | — | — | Unavailable |
| 18-25 | R_CAN signals | RT24 | 3.3 V | — | Unavailable |
| 26 | GND | Power | — | — | Unavailable |

## Evidence chain

- [Official K3 Pico-ITX user guide and connector tables](https://cdn-resource.spacemit.com/hardware/eco/docs-product/en/k3_pico/pico_user_guide.md)
- [Official K3 Pico-ITX hardware resources](https://cdn-resource.spacemit.com/hardware/eco/docs-product/en/k3_pico/pico_hw_resources.md)
- [Official schematic](https://cdn-resource.spacemit.com/file/product/K3/k3_pico_hw/SCH-146-V10_K3_DEB1_P1_LP5315B_PDF.pdf), SHA256 `7e37c4a9d119a81b389d88f238cad13d4a1a8a6f5f83de2dcb5e34280289cd19`
- [Exact K3 Pico-ITX DTS at the audited SpacemiT kernel commit](https://github.com/spacemit-com/linux-6.18/blob/27275ec8240cc49af3a525b8bc325d9b5029fb81/arch/riscv/boot/dts/spacemit/k3-pico-itx.dts)
- [K3 pinctrl definitions](https://github.com/spacemit-com/linux-6.18/blob/27275ec8240cc49af3a525b8bc325d9b5029fb81/arch/riscv/boot/dts/spacemit/k3-pinctrl.dtsi)
- [SpacemiT GPIO driver, including `spacemit,k3-gpio`](https://github.com/spacemit-com/linux-6.18/blob/27275ec8240cc49af3a525b8bc325d9b5029fb81/drivers/gpio/gpio-spacemit-k1.c)

The audited source is commit `27275ec8240cc49af3a525b8bc325d9b5029fb81`, tagged `k3-br-v1.0.0` on the `k3-br-v1.0.y` line.

## Validation still required

Capture the exact image, kernel/DTB commit, `model`, complete `compatible`, `gpiodetect`, `gpioinfo`, `/dev/mem` permissions and RPi.GPIO detection output. A hardware owner must approve an unoccupied 3.3 V pin before any output test, and must confirm the FPC orientation and breakout wiring.

Do not connect the 36-pin FPC to a 3.3 V adapter: its published IO domain is 1.8 V. Do not drive BOARD pins 3-7 or 18-25 through this library; they belong to RT24. Pull, edge/PWM behavior, cleanup restoration and production-image permissions remain unverified.
