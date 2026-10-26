# Analysis

Outputs from the scripts in `code/analysis_matlab`, all from the session at Dr. Jenkyn's lab. What I concluded from them is in `Engineering Notebook/06 Prototyping 4 LAB.pdf` and `08 Lessons Learned.pdf`.

`sensor_calibration_trials.csv` - The 24 bare-sensor presses, three trials at each of four load levels for both sensors, with the raw reading and the true load from the plate. This is what `fit_calibration_curves.m` fits

`standing_trials_matlab.csv` - The 21 static holds: what the plate measured, what my device displayed, and whether the two landed in the same clinical band. These are the readings **before** the calibration anchor was fixed, which is why the device only matches on 7 of 21 and reads high on every single one.

`moving_trials_matlab.csv` - The moving trials, with peak loads and how closely my device tracked the plate through each stance. 

`median_fix_summary.csv` - One row per calibration hold, from `validate_median_fix.m`. For each of the three "stand on test foot only" holds: the anchor the device derived from its own readings alone, how far that is from the correct value, and how the other 18 standing trials scored under it.

`median_fix_trials.csv` - The per-trial detail behind that summary. All 18 held-out trials scored under each of the three anchors, with the true percentage, what the device would have shown, the error in percentage points, and both clinical bands.
