#include "ofdm-dev.h"
#include "ofdm.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void OFDMWriteTitle(void *file) {
    fprintf(file, "P1, E, P1MA, SyncArm, SyncResult\r\n");
}

void OFDMDumpRX(OFDM_Rx_struct *rx, void *file) {
    fprintf(file, "%i, ", rx->P1);
    fprintf(file, "%i, ", rx->E);
    fprintf(file, "%i, ", rx->P1MA);
    fprintf(file, "%i, ", rx->SyncArm);
    fprintf(file, "%i\r\n", rx->SyncResult);
}

void OFDMDumpEq(OFDM_Rx_struct *rx, void *file) {
    fprintf(file, "Equalizer instance\r\n");
    fprintf(file, "subcarrier, real, imaginary\r\n");
    for (int i = 0; i < OFDM_FFT_N>>1; i++) {
        int j = i<<1;
        fprintf(file, "%i, %i, %i\r\n", i, rx->Eq[j], rx->Eq[j+1]);
    }
}


void OFDMDumpSync(OFDM_Rx_struct *rx, void *file) {
    double i = (double)rx->Eq[52]/8192.0;
    double q = (double)rx->Eq[53]/8192.0;
    fprintf(file, "Sync Field decode\r\n");
    fprintf(file, "Byte Count, %i\r\n", rx->Syncfield.Fields.PayloadByteCount);
    fprintf(file, "Constellation, %i\r\n", rx->Syncfield.Fields.Constellation);
    fprintf(file, "Randomizer, %i\r\n", rx->Syncfield.Fields.RandomizerIndex);
    fprintf(file, "Bandwidth, %i\r\n", rx->Syncfield.Fields.Bandwidth);
}

void OFDMDumpBaseband(OFDM_Rx_struct *rx, void *file) {
    fprintf(file, "Equalized Baseband Symbol:\r\n");
    fprintf(file, "subcarrier, real, imaginary\r\n");
    for (int i = 0; i < OFDM_FFT_N>>1; i++) {
        int j = i<<1;
        fprintf(file, "%i, %i, %i\r\n", i, rx->FFTBuf[j], rx->FFTBuf[j+1]);
    }
}

void OFDMSymbolSVG(OFDM_Rx_struct *rx, int sync_sequence, int symbol_sequence) {
    // Create an SVG file with the equalizer I/Q points
    // Create a text file for sync search development data
    FILE *svg;
    char work_string[100];
    snprintf(work_string, sizeof(work_string), "svg/sym-%04i-%02i.svg", sync_sequence, symbol_sequence);
    svg = fopen(work_string, "w");
    if (svg == NULL) {
        printf("Could not create %s.\r\n", work_string);
    } else {
        // Make an SVG viewbox
        int16_t uc_rad = 16384;
        int xdim = 65536;
        int ydim = 65536;
        snprintf(work_string, sizeof(work_string), "%i, %i, %i, %i", /* xmin */-(xdim/2), /* ymin */ -(ydim/2), /* width */ xdim, /* height */ ydim);
        fprintf(svg, "<svg xmlns='http://www.w3.org/2000/svg' viewBox='%s'>\r\n", work_string);
		// Add some text
        int text_size = 1000;
        int text_line_int = 1500;
        int text_x = -32767;
        int text_y = -32767 + text_size;
        double sto = (double)rx->AppliedPilotAngle/64;
        double tb_delta = -(1e6 * sto) / (((double)OFDM_CP_N + (double)OFDM_FFT_N) * ((double)symbol_sequence+1));
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i'", text_x, text_y);
        fprintf(svg, " <text %s font-family='Arial, sans-serif' font-size='1000' fill='black'>", work_string);
        fprintf(svg, "I/Q Plot, Sync Sequence %i, Symbol %i", sync_sequence, symbol_sequence);
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i'", text_x, text_y);
        fprintf(svg, " <text %s font-family='Arial, sans-serif' font-size='1000' fill='black'>", work_string);
        fprintf(svg, "Equalizer SNR %.1fdB", 10*log((double)rx->SignalE / (double)rx->NoiseE)/log(10));
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i'", text_x, text_y);
        fprintf(svg, " <text %s font-family='Arial, sans-serif' font-size='1000' fill='black'>", work_string);
        fprintf(svg, "Pilot Correction %.2f\u00B0 per bin", (double)rx->AppliedPilotAngle*180/32768);
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i'", text_x, text_y);
        fprintf(svg, " <text %s font-family='Arial, sans-serif' font-size='1000' fill='black'>", work_string);
        fprintf(svg, "Bin %i correction %.1f\u00B0", rx->FirstBin, (double)rx->FirstBin * (double)rx->AppliedPilotAngle*180/32768);
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i'", text_x, text_y);
        fprintf(svg, " <text %s font-family='Arial, sans-serif' font-size='1000' fill='black'>", work_string);
        fprintf(svg, "Bin %i correction %.1f\u00B0", rx->LastBin, (double)rx->LastBin * (double)rx->AppliedPilotAngle*180/32768);
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i'", text_x, text_y);
        fprintf(svg, " <text %s font-family='Arial, sans-serif' font-size='1000' fill='black'>", work_string);
        fprintf(svg, "STO: %.2f samples", sto);
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i'", text_x, text_y);
        fprintf(svg, " <text %s font-family='Arial, sans-serif' font-size='1000' fill='black'>", work_string);
        fprintf(svg, "Timebase delta: %.1f ppm", tb_delta);
        fprintf(svg, " </text>\r\n");
        // Draw the constellation unit circle
        fprintf(svg, " <circle stroke='gray' stroke-width='100' r='%i' fill='none'>\r\n", uc_rad);
		fprintf(svg, "  <title>Constellation Unit Circle Radius %i</title>\r\n", uc_rad);
		fprintf(svg, " </circle>\r\n");
        // Draw the real axis
        fprintf(svg, " <line stroke='gray' stroke-width='100' x1='%i' y1='%i' x2='%i' y2='%i' fill='none'>\r\n", -32768, 0, 32768, 0);
		fprintf(svg, "  <title>Constellation Real Axis Size %i</title>\r\n", 65536);
		fprintf(svg, " </line>\r\n");
        // Label the real axis
        text_x = (xdim/2) - (2*text_size);
        text_y = text_size;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "I");
        fprintf(svg, " </text>\r\n");

        // Draw the imag axis
        fprintf(svg, " <line stroke='gray' stroke-width='100' x1='%i' y1='%i' x2='%i' y2='%i' fill='none'>\r\n", 0, -32768, 0, 32768);
		fprintf(svg, "  <title>Constellation Imag Axis Size %i</title>\r\n", 65536);
		fprintf(svg, " </line>\r\n");
        // Label the imag axis
        text_x = text_size/2;
        text_y = text_size-(ydim/2);
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "Q");
        fprintf(svg, " </text>\r\n");


        int noise_rad = (int)((double)uc_rad * sqrt((double)rx->NoiseE / (double)rx->SignalE));
        if (noise_rad < 200) {
            noise_rad = 200;
        }
        for (int i = rx->FirstBin; i <= rx->LastBin; i++) {
            int ii = i<<1;
			// SVG graphics treat the y-axis as low values at top, so invert y axis for I/Q plot
            snprintf(work_string, sizeof(work_string), "fill='blue' stroke-width='0' cx='%i' cy='%i' r='%i' opacity='0.1'", /* cx */rx->FFTBuf[ii], /* cy */ -rx->FFTBuf[ii+1], noise_rad);
            fprintf(svg, "  <circle %s>\r\n", work_string);
			double tap_mag = sqrt(pow(rx->FFTBuf[ii], 2) + pow(rx->FFTBuf[ii+1], 2))/(double)uc_rad;
			double tap_ang = atan2(rx->FFTBuf[ii+1], rx->FFTBuf[ii]) * 180 /  M_PI;
            fprintf(svg, "   <title>FFT Bin %i (%i, %i)  %.2f\u2220%.1f\u00B0</title>\r\n", i, rx->FFTBuf[ii], rx->FFTBuf[ii+1], tap_mag, tap_ang);
			fprintf(svg, "  </circle>\r\n");
        }
        fprintf(svg, "</svg>\r\n");
    }
    fclose(svg);
}

void OFDMEqMagSVG(OFDM_Rx_struct *rx, int sequence) {
    // Create an SVG file with the equalizer I/Q points
    // Create a text file for sync search development data
    FILE *svg;
    char work_string[100];
    snprintf(work_string, sizeof(work_string), "svg/eqmag-%04i.svg", sequence);
    svg = fopen(work_string, "w");
    if (svg == NULL) {
        printf("Could not create %s.\r\n", work_string);
    } else {
        // Make an SVG viewbox
        int16_t uc_rad = 1<<rx->ConstShift;
		double eq_max_gain = 32767/(double)uc_rad;
		double xdim = 512;
		double ydim = 512;
		// Initialize an SVG viewbox
        snprintf(work_string, sizeof(work_string), "%i, %i, %i, %i", /* xmin */0, /* ymin */0, /* width */ (int)xdim, /* height */ (int)ydim);
        fprintf(svg, "<svg xmlns='http://www.w3.org/2000/svg' viewBox='%s'>\r\n", work_string);
		// Add some text
        int text_size = 10;
        int text_line_int = 15;
        int text_x = 5;
        int text_y = 0 + text_size;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='Arial, sans-serif' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "Equalizer Mag Plot");
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='Arial, sans-serif' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "Sync Sequence %i", sequence);
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='Arial, sans-serif' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "SNR %.1fdB", 10*log((double)rx->SignalE / (double)rx->NoiseE)/log(10));
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;

		// Draw graticule lines
		int x = 0;
        int bin;
        fprintf(svg, " <line stroke='gray' stroke-width='1' x1='%i' y1='%i' x2='%i' y2='%i' fill='gray'>\r\n", x, 0, x, (int)ydim);
		fprintf(svg, "  <title>Graticule %i</title>\r\n", 0);
		fprintf(svg, " </line>\r\n");
        // Put a label on this line
        text_x = 0;
        text_y = (int)ydim-(1+text_size);
        bin = rx->FirstBin;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "Bin %i", bin);
        fprintf(svg, " </text>\r\n");
        text_y = (int)ydim-1;
        text_x = 0;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "%.3fHz", (double)bin * OFDM_BIN_HZ);
        fprintf(svg, " </text>\r\n");

		x += ((int)xdim / 4);
        fprintf(svg, " <line stroke='gray' stroke-width='1' x1='%i' y1='%i' x2='%i' y2='%i' fill='gray'>\r\n", x, 0, x, (int)ydim-(2*text_size));
		fprintf(svg, "  <title>Graticule %i</title>\r\n", 1);
		fprintf(svg, " </line>\r\n");
        // Put a label on this line
        text_x = x-(3*text_size);
        text_y = (int)ydim-(1+text_size);
        bin = rx->FirstBin + ((rx->LastBin - rx->FirstBin) * 1 / 4);
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "Bin %i", bin);
        fprintf(svg, " </text>\r\n");
        text_y = (int)ydim-1;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "%.3fHz", (double)bin* OFDM_BIN_HZ);
        fprintf(svg, " </text>\r\n");

        x += ((int)xdim / 4);
        fprintf(svg, " <line stroke='gray' stroke-width='1' x1='%i' y1='%i' x2='%i' y2='%i' fill='gray'>\r\n", x, 0, x, (int)ydim-(2*text_size));
        fprintf(svg, "  <title>Graticule %i</title>\r\n", 1);
        fprintf(svg, " </line>\r\n");
        // Put a label on this line
        text_x = x-(3*text_size);
        text_y = (int)ydim-(1+text_size);
        bin = rx->FirstBin + ((rx->LastBin - rx->FirstBin) * 2 / 4);
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "Bin %i", bin);
        fprintf(svg, " </text>\r\n");
        text_y = (int)ydim-1;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "%.3fHz", ( (double)rx->FirstBin + (((double)rx->LastBin - (double)rx->FirstBin) * 2 / 4) ) * OFDM_BIN_HZ);
        fprintf(svg, " </text>\r\n");

        x += ((int)xdim / 4);
        fprintf(svg, " <line stroke='gray' stroke-width='1' x1='%i' y1='%i' x2='%i' y2='%i' fill='gray'>\r\n", x, 0, x, (int)ydim-(2*text_size));
        fprintf(svg, "  <title>Graticule %i</title>\r\n", 1);
        fprintf(svg, " </line>\r\n");
        // Put a label on this line
        text_x = x-(3*text_size);
        text_y = (int)ydim-(1+text_size);
        bin = rx->FirstBin + ((rx->LastBin - rx->FirstBin) * 3 / 4);
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "Bin %i", bin);
        fprintf(svg, " </text>\r\n");
        text_y = (int)ydim-1;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "%.3fHz", ( (double)rx->FirstBin + (((double)rx->LastBin - (double)rx->FirstBin) * 3 / 4) ) * OFDM_BIN_HZ);
        fprintf(svg, " </text>\r\n");

		x += ((int)xdim / 4);
        fprintf(svg, " <line stroke='gray' stroke-width='1' x1='%i' y1='%i' x2='%i' y2='%i' fill='gray'>\r\n", x, 0, x, (int)ydim);
		fprintf(svg, "  <title>Graticule %i</title>\r\n", 4);
		fprintf(svg, " </line>\r\n");
        // Put a label on this line
        text_x = x-(6*text_size);
        text_y = (int)ydim-(1+text_size);
        bin = rx->LastBin;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "Bin %i", bin);
        fprintf(svg, " </text>\r\n");
        text_y = (int)ydim-1;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "%.3fHz", ( (double)rx->FirstBin + (((double)rx->LastBin - (double)rx->FirstBin)) ) * OFDM_BIN_HZ);
        fprintf(svg, " </text>\r\n");

		int y = 0;
        fprintf(svg, " <line stroke='gray' stroke-width='1' x1='%i' y1='%i' x2='%i' y2='%i' fill='gray'>\r\n", 0, y, (int)xdim, y);
		fprintf(svg, "  <title>Graticule %i</title>\r\n", 5);
		fprintf(svg, " </line>\r\n");
        // Put a label on this line
        text_y = y+text_size;
        text_x = (int)xdim - (2*text_size);
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "%.0fx", eq_max_gain);
        fprintf(svg, " </text>\r\n");

		y += ydim/4;
        fprintf(svg, " <line stroke='gray' stroke-width='1' x1='%i' y1='%i' x2='%i' y2='%i' fill='gray'>\r\n", 0, y, (int)xdim, y);
		fprintf(svg, "  <title>Graticule %i</title>\r\n", 6);
		fprintf(svg, " </line>\r\n");
        // Put a label on this line
        text_y = y+text_size;
        text_x = (int)xdim - (2*text_size);
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "%.0fx", eq_max_gain*3/4);
        fprintf(svg, " </text>\r\n");

		y += ydim/4;
        fprintf(svg, " <line stroke='gray' stroke-width='1' x1='%i' y1='%i' x2='%i' y2='%i' fill='gray'>\r\n", 0, y, (int)xdim, y);
		fprintf(svg, "  <title>Graticule %i</title>\r\n", 7);
		fprintf(svg, " </line>\r\n");
        // Put a label on this line
        text_y = y+text_size;
        text_x = (int)xdim - (2*text_size);
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "%.0fx", eq_max_gain*2/4);
        fprintf(svg, " </text>\r\n");

		y += ydim/4;
        fprintf(svg, " <line stroke='gray' stroke-width='1' x1='%i' y1='%i' x2='%i' y2='%i' fill='gray'>\r\n", 0, y, (int)xdim, y);
		fprintf(svg, "  <title>Graticule %i</title>\r\n", 8);
		fprintf(svg, " </line>\r\n");
        // Put a label on this line
        text_y = y+text_size;
        text_x = (int)xdim - (2*text_size);
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "%.0fx", eq_max_gain*1/4);
        fprintf(svg, " </text>\r\n");

		y += ydim/4;
        fprintf(svg, " <line stroke='gray' stroke-width='1' x1='%i' y1='%i' x2='%i' y2='%i' fill='gray'>\r\n", 0, y, (int)xdim, y);
		fprintf(svg, "  <title>Graticule %i</title>\r\n", 9);
		fprintf(svg, " </line>\r\n");
		
        for (int i = rx->FirstBin; i <= rx->LastBin; i++) {
            int ii = i<<1;
			double tap_mag = sqrt(pow((double)rx->Eq[ii+1], 2) + pow((double)rx->Eq[ii], 2)) ;
			double tap_ang = atan2(rx->Eq[ii], rx->Eq[ii+1]) * 180 /  M_PI;
			// SVG graphics treat the y-axis as low values at top, so invert y axis for I/Q plot
            snprintf(work_string, sizeof(work_string), "cx='%i' cy='%i'", /* cx */(int)((i-rx->FirstBin) * xdim / (rx->LastBin - rx->FirstBin)), /* cy */ (int)ydim - (int)(tap_mag * ydim / 32768));
            fprintf(svg, "  <circle %s r='2' fill='green'>\r\n", work_string);
            fprintf(svg, "   <title>FFT Bin %i (%i, %i)  %.2f\u2220%.1f\u00B0</title>\r\n", i, rx->Eq[ii], rx->Eq[ii+1], tap_mag*eq_max_gain/32768, tap_ang);
			fprintf(svg, "  </circle>\r\n");
        }

        fprintf(svg, "</svg>\r\n");
    }
    fclose(svg);
}

void OFDMEqSVG(OFDM_Rx_struct *rx, int sequence) {
    // Create an SVG file with the equalizer I/Q points
    // Create a text file for sync search development data
    FILE *svg;
    char work_string[100];
    snprintf(work_string, sizeof(work_string), "svg/eq-%04i.svg", sequence);
    svg = fopen(work_string, "w");
    if (svg == NULL) {
        printf("Could not create %s.\r\n", work_string);
    } else {
        // Make an SVG viewbox
        int16_t uc_rad = 1<<rx->ConstShift;
		double eq_max_gain = 32767/(double)uc_rad;
        int xdim = 65536;
        int ydim = 65536;
		// Initialize an SVG viewbox
        snprintf(work_string, sizeof(work_string), "%i, %i, %i, %i", /* xmin */-(xdim/2), /* ymin */ -(ydim/2), /* width */ xdim, /* height */ ydim);
        fprintf(svg, "<svg xmlns='http://www.w3.org/2000/svg' viewBox='%s'>\r\n", work_string);
		// Add some text
        int text_size = 1000;
        int text_line_int = 1500;
        int text_x = -32767;
        int text_y = -32767 + text_size;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i'", text_x, text_y);
        fprintf(svg, " <text %s font-family='Arial, sans-serif' font-size='1000' fill='black'>\r\n", work_string);
        fprintf(svg, "Equalizer I/Q Plot, Sync Sequence %i", sequence);
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i'", text_x, text_y);
        fprintf(svg, " <text %s font-family='Arial, sans-serif' font-size='1000' fill='black'>\r\n", work_string);
        fprintf(svg, "SNR %.1fdB", 10*log((double)rx->SignalE / (double)rx->NoiseE)/log(10));
        fprintf(svg, " </text>\r\n");
		
		double i = (double)rx->Eq[rx->FirstBin<<1] / 32768;
		double q = (double)rx->Eq[(rx->FirstBin<<1)+1] / 32768;
		double angle = atan2(q,i);
		double sto = (angle * OFDM_FFT_N) / ((double)rx->FirstBin * 2 * M_PI);
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i'", text_x, text_y);
        fprintf(svg, " <text %s font-family='Arial, sans-serif' font-size='1000' fill='black'>\r\n", work_string);
        fprintf(svg, "Phase of Bin %i: %0.2f", rx->FirstBin, angle*180/M_PI);
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i'", text_x, text_y);
        fprintf(svg, " <text %s font-family='Arial, sans-serif' font-size='1000' fill='black'>\r\n", work_string);
        fprintf(svg, "STO: %0.2f", sto);
        fprintf(svg, " </text>\r\n");



        // Draw the imag axis
        fprintf(svg, " <line stroke='gray' stroke-width='100' x1='%i' y1='%i' x2='%i' y2='%i' fill='none'>\r\n", 0, -ydim/2, 0, ydim/2);
        fprintf(svg, "  <title>Constellation Imag Axis Size %i</title>\r\n", ydim);
        fprintf(svg, " </line>\r\n");
        // Label the imag axis
        text_x = text_size/2;
        text_y = text_size-(ydim/2);
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "Q");
        fprintf(svg, " </text>\r\n");


        // Draw the real axis
        fprintf(svg, " <line stroke='gray' stroke-width='100' x1='%i' y1='%i' x2='%i' y2='%i' fill='none'>\r\n", -xdim/2, 0, xdim/2, 0);
        fprintf(svg, "  <title>Constellation Imag Axis Size %i</title>\r\n", xdim);
        fprintf(svg, " </line>\r\n");
        // Label the real axis
        text_x = (xdim/2) - (2*text_size);
        text_y = text_size;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "I");
        fprintf(svg, " </text>\r\n");


		
        // Draw the equalizer unity-gain circle
        fprintf(svg, " <circle stroke='green' stroke-width='100' r='%i' fill='none'>\r\n", uc_rad);
        fprintf(svg, "   <title>Equalizer Unity Value %i Gain %.2f</title>\r\n", uc_rad, 1.00);
		fprintf(svg, " </circle>\r\n");
        // Label equalizer max-gain circle
        text_x = uc_rad * sqrt(2)/2;
        text_y = -text_x;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "1x");
        fprintf(svg, " </text>\r\n");

        // Draw the equalizer max-gain circle
        fprintf(svg, " <circle stroke='red' stroke-width='100' r='%i' fill='none'>\r\n", (ydim/2)-50);
        fprintf(svg, "   <title>Equalizer Max Value %i Gain %.2f</title>\r\n", 32767, eq_max_gain);
		fprintf(svg, " </circle>\r\n");
        // Label equalizer max-gain circle
        text_x = (xdim/4) * sqrt(2);
        text_y = -text_x;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='courier' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "%.0fx", eq_max_gain);
        fprintf(svg, " </text>\r\n");


        fprintf(svg, " <g fill='blue' stroke-width='0'>\r\n");
        for (int i = rx->FirstBin; i <= rx->LastBin; i++) {
            int ii = i<<1;
			// SVG graphics treat the y-axis as low values at top, so invert y axis for I/Q plot
            snprintf(work_string, sizeof(work_string), "cx='%i' cy='%i'", /* cx */rx->Eq[ii], /* cy */ -rx->Eq[ii+1]);
            fprintf(svg, "  <circle %s r='100'>\r\n", work_string);
			double tap_mag = sqrt(pow(rx->Eq[ii+1], 2) + pow(rx->Eq[ii], 2))/(double)uc_rad;
			double tap_ang = atan2(rx->Eq[ii], rx->Eq[ii+1]) * 180 /  M_PI;
            fprintf(svg, "   <title>FFT Bin %i (%i, %i)  %.2f\u2220%.1f\u00B0</title>\r\n", i, rx->Eq[ii], rx->Eq[ii+1], tap_mag, tap_ang);
			fprintf(svg, "  </circle>\r\n");
        }
        fprintf(svg, " </g>\r\n");
        fprintf(svg, "</svg>\r\n");
    }
    fclose(svg);
}


void OFDMSyncDump(OFDM_Rx_struct *rx, OFDM_Dev_struct *od) {
    od->Index++;
    if (od->Index >= OD_BUF_LEN) {
        od->Index = 0;
    }
    od->E[od->Index] = rx->E;
    od->P1MA[od->Index] = rx->P1MA;
    od->P1[od->Index] = rx->P1;
}

void OFDMSyncSVG(OFDM_Rx_struct *rx, OFDM_Dev_struct *od, int sequence) {
    // Create an SVG file with the Schmidl-Cox detection products
    FILE *svg;
    char work_string[100];
    snprintf(work_string, sizeof(work_string), "svg/sync-%04i.svg", sequence);
    svg = fopen(work_string, "w");
    if (svg == NULL) {
        printf("Could not create %s.\r\n", work_string);
    } else {

        // Determine scale based on E and P1MA
        int dim = 2;
        for (int i = 0; i < OD_BUF_LEN; i++) {
            while (dim < (od->P1[i]*4/3)) {
                dim *= 2;
            }
        }
        for (int i = 0; i < OD_BUF_LEN; i++) {
            while (dim < (od->E[i]*4/3)) {
                dim *= 2;
            }
        }

        // Make an SVG viewbox
        double xdim = dim;
        double ydim = dim;
		double xmin = 0;
        double ymin = -3*dim/4;
		double xnorm = dim / (double)OD_BUF_LEN;
        // Initialize an SVG viewbox
        snprintf(work_string, sizeof(work_string), "%i, %i, %i, %i", /* xmin */(int)xmin, /* ymin */(int)ymin, /* width */ (int)xdim, /* height */ (int)ydim);
        fprintf(svg, "<svg xmlns='http://www.w3.org/2000/svg' viewBox='%s'>\r\n", work_string);
        // Add some text
        int text_size = dim / 50;
        int text_line_int = text_size;
        int text_x = (int)xmin + text_size / 3;
        int text_y = ymin + text_size;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='Arial, sans-serif' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "Sync Search");
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='Arial, sans-serif' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "Sequence %i", sequence);
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='Arial, sans-serif' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "SNR %.1fdB", 10*log((double)rx->SignalE / (double)rx->NoiseE)/log(10));
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='Arial, sans-serif' font-size='%i' fill='green'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "P1");
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='Arial, sans-serif' font-size='%i' fill='blue'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "E");
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='Arial, sans-serif' font-size='%i' fill='purple'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "P1MA");
        fprintf(svg, " </text>\r\n");
        text_y += text_line_int;
        snprintf(work_string, sizeof(work_string), "x='%i' y='%i' font-family='Arial, sans-serif' font-size='%i' fill='black'", text_x, text_y, text_size);
        fprintf(svg, " <text %s>", work_string);
        fprintf(svg, "Y Dimension %i", dim);
        fprintf(svg, " </text>\r\n");

        // Draw a vertical line at the sync sample point
        int line_x = (int)(OFDM_FFT_N * dim / OD_BUF_LEN);

        fprintf(svg, " <line stroke='gray' stroke-width='%i' x1='%i' y1='%i' x2='%i' y2='%i'>\r\n", (int)(dim / 1024), line_x, (int)ymin, line_x, (int)(ymin+ydim));
        fprintf(svg, "  <title>Vertical Graticule</title>\r\n");
        fprintf(svg, " </line>\r\n");
		
		// Draw vertical lines to represent width of CP
		line_x -= (int)((double)OFDM_CP_N * xnorm / 2);
        fprintf(svg, " <line stroke='green' stroke-width='%i' x1='%i' y1='%i' x2='%i' y2='%i'>\r\n", (int)(dim / 1024), line_x, (int)ymin, line_x, 0);
        fprintf(svg, "  <title>Vertical Graticule</title>\r\n");
        fprintf(svg, " </line>\r\n");
		line_x += (int)((double)OFDM_CP_N * xnorm);
        fprintf(svg, " <line stroke='green' stroke-width='%i' x1='%i' y1='%i' x2='%i' y2='%i'>\r\n", (int)(dim / 1024), line_x, (int)ymin, line_x, 0);
        fprintf(svg, "  <title>Vertical Graticule</title>\r\n");
        fprintf(svg, " </line>\r\n");

        // Draw a vertical line half a symbol back in time
        line_x = (int)(OFDM_FFT_N * dim / OD_BUF_LEN) - (int)((OFDM_FFT_N+OFDM_CP_N) * xnorm / 2);
        fprintf(svg, " <line stroke='green' stroke-width='%i' x1='%i' y1='%i' x2='%i' y2='%i'>\r\n", (int)(dim / 1024), line_x, (int)ymin, line_x, 0);
        fprintf(svg, "  <title>Vertical Graticule</title>\r\n");
        fprintf(svg, " </line>\r\n");

        // Draw a horizontal line at y = 0
        fprintf(svg, " <line stroke='gray' stroke-width='%i' x1='%i' y1='%i' x2='%i' y2='%i'>\r\n", (int)(dim / 1024), 0, 0, (int)xdim, 0);
        fprintf(svg, "  <title>Horizontal Zero Graticule</title>\r\n");
        fprintf(svg, " </line>\r\n");

        text_y += text_line_int;
		
		int filter_delay = OFDM_BUF2_N>>1;
        
        int j = od->Index-rx->Delay;
        for (int i = 0; i < rx->Delay-filter_delay; i++) {
            while(j < 0) {
                j += OD_BUF_LEN;
            }
            if (j >= OD_BUF_LEN) {
                j = 0;
            }
            int k = j + filter_delay;
            if (k >= OD_BUF_LEN) {
                k -= OD_BUF_LEN;
            }
            // SVG graphics treat the y-axis as low values at top
            snprintf(work_string, sizeof(work_string), "cx='%i' cy='%i' fill='blue' stroke-width='0' r='%i'", /* cx */(int)((double)i * xnorm), /* cy */ -(int)od->E[j], /* r */ dim / 1024);
            fprintf(svg, "  <circle %s>\r\n", work_string);
            fprintf(svg, "   <title>E %i, %i</title>\r\n", i, od->E[j]);
            fprintf(svg, "  </circle>\r\n");
            snprintf(work_string, sizeof(work_string), "cx='%i' cy='%i' fill='purple' stroke-width='0' r='%i'", /* cx */(int)((double)i * xnorm), /* cy */ -(int)od->P1MA[k], /* r */ dim / 1024);
            fprintf(svg, "  <circle %s>\r\n", work_string);
            fprintf(svg, "   <title>P1MA %i, %i</title>\r\n", i, od->P1MA[k]);
            fprintf(svg, "  </circle>\r\n");
            snprintf(work_string, sizeof(work_string), "cx='%i' cy='%i' fill='green' stroke-width='0' r='%i'", /* cx */(int)((double)i * xnorm), /* cy */ -(int)od->P1[j], /* r */ dim / 1024);
            fprintf(svg, "  <circle %s>\r\n", work_string);
            fprintf(svg, "   <title>P1 %i, %i</title>\r\n", i, od->P1[j]);
            fprintf(svg, "  </circle>\r\n");
            j++;
        }

        fprintf(svg, "</svg>\r\n");
    }
    fclose(svg);
}