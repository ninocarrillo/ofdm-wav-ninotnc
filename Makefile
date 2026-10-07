all: ofdm-tx ofdm-rx

ofdm-tx:
	gcc model/src/ofdm-tx.c model/src/textfile.c model/src/wavfile.c edac/src/*.c ofdm/src/ofdm.c ofdm/src/fft_int.c ofdm/src/dsp.c -O3 -g -o ofdm-tx -lm -I model/inc -I ofdm/inc -I edac/inc
ofdm-rx:
	gcc model/src/ofdm-rx.c model/src/textfile.c model/src/wavfile.c edac/src/*.c model/src/ofdm-dev.c ofdm/src/ofdm.c ofdm/src/fft_int.c ofdm/src/dsp.c -O3 -g -o ofdm-rx -lm -I model/inc -I ofdm/inc -I edac/inc
clean:
	-rm -f ./ofdm-tx.exe
	-rm -f ./ofdm-tx
	-rm -f ./ofdm-rx.exe
	-rm -f ./ofdm-rx
	-rm -f svg/*
	-rmdir svg
	-mkdir svg
