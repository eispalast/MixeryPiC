#ifndef LCD_H
#define LCD_H

#include <wiringPi.h>
#include <lcd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "channel.h"




char* construct_upper_line(char* ch_name, char* out_name);

char* construct_lower_line(channel *ch, int out);

void lcdPrint_status(channel **ch_list, int total_ch, char** output_names,int output, int display);

void lcdPrint_lower(channel **ch_list,int total_ch ,int output, int display);

void lcdPrint_lower2(channel *ch, int out, int display);
#endif
