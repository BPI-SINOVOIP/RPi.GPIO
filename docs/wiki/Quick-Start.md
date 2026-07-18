# Quick Start

## Inspect before switching a pin

```sh
python3 - <<'PY'
import RPi.GPIO as GPIO
print(GPIO.VERSION)
print(GPIO.RPI_INFO)
PY
```

Confirm all of the following before output testing:

- the detected board and carrier are correct;
- the physical header orientation is known;
- the selected pin is a 3.3 V GPIO, not power, ground, a boot strap or a board-owned peripheral;
- the selected pin is documented as output-safe for the exact board revision.

## Safe template

Ask the hardware owner for an approved physical pin, then replace `SAFE_BOARD_PIN`:

```python
import RPi.GPIO as GPIO
import time

SAFE_BOARD_PIN = 0  # Replace with an approved physical header pin.

try:
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(SAFE_BOARD_PIN, GPIO.OUT, initial=GPIO.LOW)
    GPIO.output(SAFE_BOARD_PIN, GPIO.HIGH)
    time.sleep(0.1)
    GPIO.output(SAFE_BOARD_PIN, GPIO.LOW)
finally:
    GPIO.cleanup()
```

Do not run this template with `0`; it is deliberately invalid until replaced. Do not copy a pin number from another Banana Pi model.

## `BOARD` versus `BCM`

- `GPIO.BOARD` selects the physical connector position for the exact supported carrier.
- `GPIO.BCM` selects the compatibility number defined by this project's board map.
- The BCM-style number is not necessarily the native Linux gpiochip offset or SoC bank/line on a non-Raspberry-Pi SoC.

Never mix numbering modes in one process. Call `GPIO.setmode()` once and use the matching documented pin values.

## Input and cleanup

After a hardware owner approves a safe input and pull support is confirmed:

```python
GPIO.setmode(GPIO.BOARD)
GPIO.setup(SAFE_BOARD_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
print(GPIO.input(SAFE_BOARD_PIN))
GPIO.cleanup()
```

On a backend without verified pull control, use an external resistor and do not claim `PUD_UP`/`PUD_DOWN` support.
