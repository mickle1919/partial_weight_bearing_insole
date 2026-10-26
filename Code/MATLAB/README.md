# MATLAB

This is the code I used to analyze my lab data. All data is pulled from the `Lab Data` folder. The results from these scripts, and what I concluded from them, are in `Engineering Notebook/06 Prototyping 4 LAB.pdf`.

## The lab session

* `fit_calibration_curves.m` - fits `force = a * raw^b` to each sensor from the 24 bare-sensor presses and compares the result against the curves already in my firmware.
* `compare_curves_two_anchor.m` - asks whether swapping to the refitted curves would change anything, by solving each curve set's own anchors and scoring the same 15 trials under both.
* `analyze_standing_trials.m` - scores the 21 static holds against the plate and sorts them into clinical bands.
* `analyze_moving_trials.m` - overlays my device against the plate for the moving trials, aligns the two streams, and graphs them as a grid.

## Finding and fixing the anchor error

These came after the lab notebook was written, so their results are not in `06 Prototyping 4 LAB.pdf`.

* `apply_correction.m` - applies the correction factor I back-solved by hand to all 21 standing trials and re-bands them.
* `validate_median_fix.m` - rebuilds the anchor from the device's own logged forces, using no force-plate data, then scores the 18 trials that did not produce it.
* `test_limit_anchor.m` - asks whether the upper anchor has to sit at full body weight, by comparing it against a 30% and a 50% anchor on the same 12 trials.
