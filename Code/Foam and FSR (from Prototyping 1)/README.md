# Prototyping 1 - Foam and FSR

Code from the conductive foam and first FSR testing. See
`Engineering Notebook/03 Prototyping 1.pdf`.

- `01_basic_read.ino` - reads one sensor and prints the number. Used for the conductive foam and the conductive pressure sensor sheets, to see whether pressing either one actually changed the reading.
- `02_averaged_read.ino` - same thing, but averages 20 readings in a row so the number stops jumping around. This is the version used for the resistor testing, where I tried every resistor I had to find which one gave the biggest usable range on the foam.
- `03_wheatstone_two_channel.ino` - for the FSR, not the foam. Reads both midpoints of the Wheatstone bridge and prints the difference between them, which cancels out drift and noise.
- `04_trial_recorder.ino` - for the foam calibration. Adds s/g/c commands so I could record a full 5 second trial instead of just watching the live number. I needed this because the foam slowly sinks under a steady weight, so a single reading does not describe what it is doing.

Since these were all written in the Arduino interface, I sadly was not able to recover the original files for 01-03. I went back through my old engineering notebook and transcribed them from the pictures in it.
