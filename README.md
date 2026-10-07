# Generate and process OFDM audio symbols.
# Requirements
- GCC
- Make
# Compilation
Clone the repository and navigate to the repository folder. Type `make` and the binaries will be generated. To rebuild, execute `make clean` and then `make` again.
## ofdm-tx
This binary takes any text-format file as its input and generates a 24kHz 1-channel .wav file that contains OFDM frames. The input text file is parsed and each line of text is treated as a frame. In the example below, I'm using the LICENSE file in this repository as the text input file, and specifying constellation 3 which is QAM16. Constellations implemented so far are 0-7, representing BPSK thru QAM256.
```
./ofdm-tx LICENSE output.wav 3
LICENSE contains 22 lines.
Wrote 195520 samples to output.wav.
```
## ofdm-rx
This binary takes a 1-channel 48kHz .wav file and searches for Schmidl-Cox sync symbols. Then it will attempt to decode the OFDM frame that follows each sync. Work in progess.
```
./ofdm-rx output.wav output.txt
output.wav contains 195520 samples.
```
Mike was here
