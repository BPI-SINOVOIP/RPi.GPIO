#!/usr/bin/python
import RPi.GPIO as GPIO

print 'Pi Board Information'
print '---------------------'
for key,val in GPIO.RPI_INFO.items():
    print '%s => %s'%(key,val)
