# Numbering and Detection

## Public numbering modes

| Mode | Meaning |
| --- | --- |
| `GPIO.BOARD` | Physical connector position on the exact supported carrier/header. |
| `GPIO.BCM` | Compatibility identifier from the board map. It is not necessarily the SoC's native GPIO line. |

Internally, the backend also needs the native SoC GPIO bank/line or Linux gpiochip offset. Do not expose or document that controller identifier as though it were universally interchangeable with `GPIO.BCM`.

BPI-F4 is a documented exception to the usual single-header layout: `GPIO.BOARD` numbers 1-29 are rows in the official combined terminal table across CN8/CN3/CN7/CN6/CN5/CN1/CN4, not connector-local pin labels. `GPIO.BCM` channels 0-19 follow the GPIO-bearing table rows in order and map to native SP7350 lines `84,85,71,70,81,80,83,82,60,61,69,68,72,73,74,75,59,58,56,57`.

## Board detection

Detection aliases and Banana Pi model selection are maintained in `source/c_gpio_bpi.c`. Board-specific maps are included through `source/bpi_gpio.h` and the relevant `source/bpi-*.h` headers.

For a new board, collect both values from the exact production image:

```sh
cat /proc/device-tree/model; echo
tr '\0' '\n' < /proc/device-tree/compatible
```

Prefer exact `compatible` strings over broad marketing names. Generic model strings shared by multiple revisions must not silently select the wrong carrier map.

## Modules and carriers

A compute module does not define a 26/40-pin user header by itself. `GPIO.BOARD` can only be promised for the exact carrier/development kit.

Examples:

- LM7 uses W3 as its documented development kit.
- CM4, CM5, CM5 Pro, CM6, AIM7 and AI2N support claims must name the exact IO board/carrier.

## Detection bug report data

Attach:

```sh
uname -a
cat /etc/os-release
cat /proc/device-tree/model; echo
tr '\0' '\n' < /proc/device-tree/compatible
python3 -c 'import RPi.GPIO as GPIO; print(GPIO.VERSION); print(GPIO.RPI_INFO)'
```

Also provide board, module and carrier revisions and a clear header photograph.
