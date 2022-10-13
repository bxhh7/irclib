#include "utils.h"


void udebug(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	//SAGI_PRINT(fmt, "[DEBUG]", SAGI_COLOR_GREEN_FG , ap);
	printf(SAGI_COLOR_BLUE_FG "[DEBUG]" SAGI_COLOR_RESET);
	vprintf(fmt, ap);
	printf("\n");
	va_end(ap);
}


void uwarn(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	printf(SAGI_COLOR_RED_FG "[WARN]" SAGI_COLOR_RESET);
	vprintf(fmt, ap);
	printf("\n");

	va_end(ap);
}



void uerror(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	printf(SAGI_COLOR_RED_FG "[ERROR]" SAGI_COLOR_RESET);
	vprintf(fmt, ap);
	printf("\n");
	
	va_end(ap);
}

