# Prototyping 3 and 4 - Lab

Code for the two-sensor insole and the force plate session. See
`Engineering Notebook/05 Prototyping 3.pdf` and
`Engineering Notebook/06 Prototyping 4 LAB.pdf`.

Both files run on the Arduino Uno. Heel sensor on A1, ball sensor on A0.

- `01_lab_version.ino` - the full device. Reads both sensors, converts each raw reading to pounds using its calibration curve, and displays the total as a percentage of body weight. Type `z` with your foot off the ground to set 0%, then `f` standing on the sensor foot to set 100%, then `g` to record a 5 second trial. This is the code I took to the lab.

  **It contains the bug the lab session exposed.** The `f` command averages a 5 second hold while you shift your weight around, which stores a 100% anchor  too low and makes every reading come too high.
  
- `02_f_fix_mean_to_median.ino` - the fix. One change only: `f` now takes the median of a 3 second steady hold while standing on the sensor foot alone, instead of the mean of a 5 second hold while moving. 
