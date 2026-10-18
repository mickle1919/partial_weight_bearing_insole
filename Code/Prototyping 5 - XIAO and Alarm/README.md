# Prototyping 5 - XIAO and Alarm

Code for compacting the device: moving off the Arduino Uno onto the Seeed
XIAO nRF52840, going wireless, and adding the buzzer. See
`Engineering Notebook/07 Prototyping 5.pdf`.

Heel sensor on D1, ball sensor on D0.

## Firmware versions

These are just different versions of one program. Most differ from the one before by a few lines.

- `01_xiao_ble.ino` - the port to the XIAO, plus Bluetooth. Forces the ADC to 10-bit and to measure against the supply, so it returns the same raw numbers as the Uno and my existing calibration curves still apply.
- `02_xiao_ble_led.ino` - adds a status LED. The XIAO has no power indicator, so without this there is no way to tell whether the board is running.
- `03_xiao_ble_alarm.ino` - adds a buzzer
- `04_xiao_ble_alarm_direct_drive.ino` - same program, buzzer wired straight to the pin instead of through the transistor. This was part of chasing the alarm problem.
- `05_xiao_ble_alarm_limit_first.ino` - you type your prescribed limit before calibrating
- `06_xiao_ble_simple.ino` - what runs on the device now. It has a continuous buzz above the limit

## Various tests

These were just small programs written to test one question at a time

- `07_test_adc_divider_check.ino` - a 100k/100k divider read on both boards. The calibration curves are fitted against raw ADC numbers, so they only carry over if the XIAO returns the same number as the Uno for the same voltage. Both read 509 against an ideal of 511.
- `08_test_buzzer.ino` - is an active buzzer loud enough driven straight off a pin, or does it need the transistor?
- `09_test_buzzer_interactive.ino` - testing the buzzer. For the direct-drive wiring (buzzer sits between 3V3 and the pin), where LOW makes sound
- `10_test_buzzer_interactive_transistor.ino` - testing the buzzer. HIGH makes sound.
