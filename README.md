# Noise Oscillator for [Korg NTS-1](https://www.korg.com/us/products/dj/nts_1/)

## Overview
This user oscillator implements several types of noise.  These can be useful as building blocks when doing sound design.  The shape knob will switch between the noise types.  For additional details on the noise types see [Colors of Noise](https://en.wikipedia.org/wiki/Colors_of_noise)

### 1. White Noise
This is a gaussian distribution

### 2. Pink Noise
-3db/octave, this is approximated using the Voss-McCartney algorithm with 8 rows.

### 3. Brownian Noise
White noise with a -6db/octive low pass first order filter.  The low pass filter is based on Korg's biquad implementation.

### 4. Blue Noise
The blue noise is created by applying a first difference operator on pink noise (generated via Voss-McCartney algorithm with 8 rows.)

### 5. Violet Noise
White noise with -6db/octave high pass first order filter.  The high pass filter is based on Korg's biquad implementation.

### 6. Grey Noise
Approximation of an equal loudness curve at 80db using a 2nd order low pass filter and a 6th order high pass filter in combination.  The low pass and high pass filters are based on Korg's biquad implementation.

### Notes
See the [logue-sdk](https://korginc.github.io/logue-sdk/) for details on:
1. How to setup a toolchain to build the project.
2. How to load the user oscillator into the keyboard using the [NTS-1 Sound Librarian](https://www.korg.com/us/products/dj/nts_1/librarian_contents.php).
3. Additional support documentation.