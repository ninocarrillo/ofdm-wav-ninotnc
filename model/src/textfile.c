#include "textfile.h"
int txtCountLines(FILE *input_file) {
	int line_count = 0;
	int char_count = 0;
	int line_char_count = 0;
	int c = getc(input_file);
	while (c != EOF) {
		if (c == '\n') {
			line_count++;
			line_char_count = 0;
		}
		if ((c != '\r') && (c != '\n')) {
			char_count++;
			line_char_count++;
		}
		c = getc(input_file);
	}
	if (line_char_count > 0) {
		line_count++;
	}
	//printf("File contains %i lines and %i characters.\r\n", line_count, char_count);
	fseek(input_file, 0, 0);
	return(line_count);
}

int txtCountChars(uint8_t *buffer) {
	// counts characters before null
	int i = 0;
	while ((buffer[i] > 0) && (buffer[i] != 10) && (buffer[i] != 13)) {
		i++;
	}
	return i;
}