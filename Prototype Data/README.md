# Prototype Data

Data from the prototyping phases:

- `Engineering Notebook/03 Prototyping 1.pdf`
- `Engineering Notebook/04 Prototyping 2.pdf`

**`foam_calibration_data.csv`** - Prototyping 1. Known weights placed on the
conductive foam sensor, from 10 lb up to 70 lb in 5 lb steps, plus my friend
Jack (163 lb) and me (176 lb) standing on it. Three trials each, recorded
every 100 ms for 5 seconds. Each weight is its own block of columns across
the sheet. The downward slope within each trial is the foam creeping under
sustained load, which is what Prototyping 2 investigated.

**`prototype_2_data.csv`** - Prototyping 2, in three sections:

- **Section A: foam on glass** - the drift test. Repeating the 20 lb and
  25 lb trials on glass instead of wood to check whether the drift was real
  foam creep or current leaking through the table.
- **Section B: bare FSR** - the FSR on its own, 5 lb to 176 lb.
- **Section C: foam sandwich FSR** - the same weights with the FSR wrapped
  in foam, testing whether spreading the load would raise its ceiling.

Comparing B against C is what showed the foam sandwich made saturation
*worse*, not better.
