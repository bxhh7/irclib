#pragma once

#include <stdio.h>
#include <stdarg.h>


#define SAGI_COLOR_BLACK_FG	"\033[1;30m"         
#define SAGI_COLOR_RED_FG 	   	"\033[1;31m"         
#define SAGI_COLOR_GREEN_FG  	"\033[1;32m"         
#define SAGI_COLOR_YELLOW_FG 	"\033[1;33m"         
#define SAGI_COLOR_BLUE_FG    	"\033[1;34m"         
#define SAGI_COLOR_MAGENTA_FG 	"\033[1;35m"         
#define SAGI_COLOR_CYAN_FG    	"\033[1;36m"         
#define SAGI_COLOR_WHITE_FG   	"\033[1;37m"         
#define SAGI_COLOR_BLACK_BG  	"\033[1;40m"
#define SAGI_COLOR_RED_BG 	  	"\033[1;41m"	
#define SAGI_COLOR_GREEN_BG  	"\033[1;42m"
#define SAGI_COLOR_YELLOW_BG 	"\033[1;43m"
#define SAGI_COLOR_BLUE_BG   	"\033[1;44m"
#define SAGI_COLOR_MAGENTA_BG 	"\033[1;45m"
#define SAGI_COLOR_CYAN_BG   	"\033[1;46m"
#define SAGI_COLOR_WHITE_BG  	"\033[1;47m"
#define SAGI_COLOR_RESET           "\033[1;0m"
#define SAGI_COLOR_BOLD_B		"\033[1;1m"  
#define SAGI_COLOR_UNDERLINE       "\033[1;4m" 
#define SAGI_COLOR_INVERSE         "\033[1;7m" 
#define SAGI_COLOR_BOLD_O 		"\033[1;21m"  
#define SAGI_COLOR_UNDERLINE_OFF   "\033[1;24m" 
#define SAGI_COLOR_INVERSE_OFF "\033[1;27m" 


void uprint(const char* fmt, const char *txt, const char *color, va_list ap);
void udebug(const char *fmt, ...);
void uwarn(const char *fmt, ...);
void uerror(const char *fmt, ...);
void udie(const char *msg);
