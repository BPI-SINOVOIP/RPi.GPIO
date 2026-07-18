# Build and Install

## Requirements

- A supported Banana Pi running Linux.
- Python 3, Python development headers, a C compiler and Git.
- Runtime access to the GPIO register/device path used by the selected backend.

Example Debian/Ubuntu preparation:

```sh
sudo apt update
sudo apt install python3 python3-dev python3-pip build-essential git
```

## Build from this repository

```sh
git clone https://github.com/BPI-SINOVOIP/RPi.GPIO.git
cd RPi.GPIO
python3 setup.py build_ext --inplace
```

Verify that the locally built extension imports:

```sh
python3 -c 'import RPi.GPIO as GPIO; print(GPIO.VERSION); print(GPIO.RPI_INFO)'
```

Run the command from the repository directory so Python loads the just-built extension.

## Install

For a system installation using the repository's legacy setuptools flow:

```sh
sudo python3 setup.py install
```

Distribution Python policies differ. On managed systems, package the module or use an environment appropriate for native extensions instead of bypassing the distribution package manager.

## Before using GPIO

Inspect detection first:

```sh
python3 - <<'PY'
import RPi.GPIO as GPIO
print(GPIO.VERSION)
print(GPIO.RPI_INFO)
PY
```

Stop if the detected model does not match the exact board/carrier. Read [Known Limitations and Roadmap](Known-Limitations-and-Roadmap) before assuming pull, edge or PWM support.
