# Hardware Validation

## Identity capture

Run these read-only commands on the exact board and carrier:

```sh
date -Is
uname -a
cat /etc/os-release
cat /proc/device-tree/model; echo
tr '\0' '\n' < /proc/device-tree/compatible
gpiodetect
gpioinfo
python3 -c 'import RPi.GPIO as GPIO; print(GPIO.VERSION); print(GPIO.RPI_INFO)'
```

Archive stdout/stderr together with the image filename/SHA256, kernel commit, DTB, board revision and carrier revision.

## Electrical approval

A hardware owner must identify safe pins before any switching test. Do not use pins connected to power, boot straps, storage, LEDs, buttons, radios, regulators, level shifters of unknown direction, expanders or 1.8 V domains unless the test plan explicitly covers them.

## Minimum test set

For at least one safe output and one safe input:

- `GPIO.BOARD` and `GPIO.BCM` select the intended connector pin;
- output LOW/HIGH is verified with a meter or logic analyzer;
- input LOW/HIGH is read correctly;
- rising/falling/BOTH edge behavior is tested if supported;
- pull-up/down is tested only if the backend implements it;
- software PWM start/change/stop is tested when claimed;
- `GPIO.cleanup()` releases the tested channels cleanly.

## Evidence levels

| Level | Meaning |
| --- | --- |
| Source-backed | Mapping is supported by schematic/DTS/source, but not run on hardware. |
| Build-tested | Native extension compiles and imports locally. |
| Boot-tested | Exact image boots and detects the intended target. |
| GPIO-tested | Safe digital input/output passed on hardware. |
| Capability-tested | Pull, edge and software PWM have individual results. |

Do not use `implemented` as a synonym for the final capability-tested level. Record failures and unsupported functions as carefully as passes.
