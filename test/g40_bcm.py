#!/usr/bin/python
import RPi.GPIO as GPIO
import time

phy_led = [8, 10, 12, 16, 18, 22, 24, 26, 28, 32, 36, 38, 40,
	37, 35, 33, 31, 29, 27, 23, 21, 19, 15, 13, 11, 7, 5, 3];


bcm_led = [2, 3, 4, 17, 27, 22, 10, 9, 11, 0, 5, 6, 13, 19, 26,
	21, 20, 16, 12, 1, 7, 8, 25, 24, 23, 18, 15, 14];

led = bcm_led
#led = phy_led

stime = 0.1
#stime = 0.5

GPIO.setmode(GPIO.BCM)
#GPIO.setmode(GPIO.BOARD)

for pin in led:
  print pin, "GPIO.setup GPIO.OUT"
  GPIO.setup(pin, GPIO.OUT)


while True:
  for pin in led:
    GPIO.output(pin, True)
    print 'on ', pin
    time.sleep(stime)
  for pin in led:
    GPIO.output(pin, False)
    print 'off ',pin
    time.sleep(stime)

