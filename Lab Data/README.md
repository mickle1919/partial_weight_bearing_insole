# Lab Data

Every test run at the Wolf Orthopaedic Biomechanics Laboratory at Western
University. For a detailed breakdown
of what each test is and what the results meant, see
`Engineering Notebook/06 Prototyping 4 LAB.pdf`.

## Each test folder

Every folder above, except `analysis`, has two things in it:

**A CSV of my data** — what my device recorded, with all three trials in the
one file.

**A folder of the lab's data** — three `.forces` files, one per trial,
straight off the force plate.


## The analysis folder

`analysis/` holds the three outputs from my MATLAB analysis, including the standing
trials, the individual sensor calibration, and the moving trials.


Also, there is **My original data from google docs.pdf**. This is the file I
actually used at the lab, where I dumped every reading as it came in. I
cleaned it up into the separate CSVs once I got home, so this is the raw
version before any of that

The code that produced the analysis is in `code/analysis_matlab/`.
