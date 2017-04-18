#!/usr/bin/python
import RPi.GPIO as GPIO
import time

phy_led2 = [8, 10, 12, 16, 18, 22, 24, 26, 28, 32, 36, 38, 40,
	37, 35, 33, 31, 29, 27, 23, 21, 19, 15, 13, 11, 7, 5, 3];

phy_led = [8, 10, 12, 16, 18, 22, 24, 26, 32, 36, 38, 40,
	37, 35, 33, 31, 29, 23, 21, 19, 15, 13, 11, 7, 5, 3];

print 'Pi Board Information'
print '---------------------'
for key,val in GPIO.RPI_INFO.items():
    print '%s => %s'%(key,val)
response = raw_input('\nIs this board info correct (y/n) ? ').upper()

GPIO.setmode(GPIO.BOARD)

for pin in phy_led:
  print pin, "GPIO.setup GPIO.OUT"
  GPIO.setup(pin, GPIO.OUT)


while True:
  for pin in phy_led:
    GPIO.output(pin, True)
    print 'on ', pin
    time.sleep(.1)
  for pin in phy_led:
    GPIO.output(pin, False)
    print 'off ',pin
    time.sleep(.1)

