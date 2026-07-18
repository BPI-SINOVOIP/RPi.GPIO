# Porting a New Board

## Evidence gate

Before editing code, obtain all of the following:

1. Exact product/module/carrier names and PCB revisions.
2. Authoritative schematic or machine-readable pin table.
3. Exact kernel DTS/DTB and board `model`/`compatible` strings.
4. Physical pin → net → SoC pad/bank/line → Linux gpiochip/offset mapping.
5. Voltage, boot-strap, reserved, board-owned and output-unsafe pin annotations.

If any mapping is ambiguous, record a `blocked` row instead of guessing.

## Decide alias or backend

- Use an alias only when official evidence proves identical carrier wiring and backend behavior.
- Add a new internal model when the header map, backend, voltage or limitations differ.
- Treat module + carrier as the target. Never alias modules solely because they share a connector form factor.

## Implementation locations

- Detection/model/backend selection: `source/c_gpio_bpi.c`
- Board map headers: `source/bpi-*.h`
- Map inclusion: `source/bpi_gpio.h`
- GPIO setup/register access: `source/c_gpio.c` and the selected backend helpers
- Python API/mode/setup logic: `source/py_gpio.c`, `source/common.c`
- Edge/event path: `source/event_gpio.c`
- Software PWM: `source/soft_pwm.c`, `source/py_pwm.c`

Keep model IDs, detection aliases and shared physical/BCM arrays aligned with BPI-WiringPi2.

## Build gate

```sh
python3 setup.py build_ext --inplace
python3 -c 'import RPi.GPIO as GPIO; print(GPIO.VERSION); print(GPIO.RPI_INFO)'
```

## Review checklist

- Canonical product name plus aliases are documented.
- Detection tests include real Device Tree strings.
- `GPIO.BOARD` power/GND/non-GPIO positions match the exact header.
- Unsafe pins stay unavailable.
- Unsupported pull/edge/PWM behavior raises or documents a clear limitation.
- The companion BPI-WiringPi2 map is updated in the same work unit.
- Hardware evidence names image, kernel, DTB, board and carrier revision.

Continue with [Hardware Validation](Hardware-Validation).
