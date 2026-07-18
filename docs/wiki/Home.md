# Banana Pi RPi.GPIO Wiki

This repository adapts the familiar `RPi.GPIO` Python API to supported Banana Pi Linux boards. This Wiki is the long-form documentation entry point for Python users, board maintainers and hardware validation teams.

## Start here

- New user: [Build and Install](Build-and-Install) → [Quick Start](Quick-Start)
- Check a board: [Board Support Matrix](Board-Support-Matrix)
- Understand `BOARD` and `BCM`: [Numbering and Detection](Numbering-and-Detection)
- Add a board: [Porting a New Board](Porting-a-New-Board)
- Validate hardware: [Hardware Validation](Hardware-Validation)
- Known gaps: [Known Limitations and Roadmap](Known-Limitations-and-Roadmap)
- Maintain the docs: [Documentation Maintenance](Documentation-Maintenance)

## Support claims

The source currently contains 47 effective internal Banana Pi model IDs and 231 detection names. These numbers describe code coverage, not complete hardware validation. A board can import, detect and map pins while pull control, edge behavior, PWM or exact device permissions remain limited.

The public Banana Pi catalog also contains products that are not appropriate `RPi.GPIO` targets. Modules without an exact carrier, MCU boards, camera products, isolated industrial DI/DO and boards without an intended user GPIO connector are recorded separately instead of receiving guessed mappings.

## Companion project

C and command-line users should use the [BPI-WiringPi2 Wiki](https://github.com/BPI-SINOVOIP/BPI-WiringPi2/wiki). Detection and shared physical/BCM maps must stay aligned across both repositories, while Python behavior is documented here.

> Safety: never call `GPIO.setup(..., GPIO.OUT)` until the exact board, carrier, revision, voltage domain and pin map have been confirmed.
