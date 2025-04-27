#!/bin/sh
arduino-cli compile --fqbn arduino:avr:mega ./eeprom_flasher/eeprom_flasher.ino && arduino-cli upload --fqbn arduino:avr:mega ./eeprom_flasher/eeprom_flasher.ino -p $1
