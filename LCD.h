#ifndef LCD_H
#define LCD_H

#include <wiringPi.h>
#include <lcd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "channel.h"
char sign=1;
char* construct_upper_line(char* ch_name, char* out_name){
	char* line=calloc(17,sizeof(char));
	
	for (int i=0;i<7;i++){
		if(ch_name[i]!='\0'){
			line[i]=ch_name[i];
		}
		else{
			line[i]=' ';
		}
	}
	line[7]='-';
	line[8]='>';
	
	int offset=strlen(out_name);
	for (int i=9; i<(16-offset); i++){
		line[i]=' ';
	}
	
	int index=16-offset;
	if (offset>7){
		index=9;
		offset=8;
	}

	for (int i=0;i<offset;i++){
		line[index]=out_name[i];
		index++;
	}
	
	
	return line;
	
}

char* construct_lower_line(channel *ch, int out){
	
	char* line=calloc(17,sizeof(char));
	
	int tiles=(int)(ch->fader_val[out]*11);
	int percent= (int) (ch->fader_val[out]*100);
	for (int i=0;i<12;i++){
		if (i<=tiles){
			line[i]=4;
			sign++;
		}
		else{
			line[i]=' ';
		}
	}
	if (ch->is_muted){
		sprintf(line+(12*sizeof(char)),"Mute");
	}
	else{
		if (percent==100){
			sprintf(line+(12*sizeof(char)),"100");
		}
		else if (percent>=10){
			sprintf(line+(12*sizeof(char))," %d",percent);
		}
		else{
			sprintf(line+(12*sizeof(char)),"  %d",percent);
		}
		line[15]='%';
	}
	
	return line;
}

void lcdPrint_status(channel **ch_list, int total_ch, char** output_names,int output, int display){
		char* upper=construct_upper_line(ch_list[total_ch]->name,output_names[output]);
		char* lower=construct_lower_line(ch_list[total_ch],output);
		lcdClear(display);
		lcdHome(display);
		lcdPuts(display,upper);
		lcdPosition(display, 0,1);
		lcdPuts(display,lower);
		free(lower);
		free(upper);
}

void lcdPrint_lower(channel **ch_list,int total_ch ,int output, int display){
	char* lower=construct_lower_line(ch_list[total_ch],output);
	lcdPosition(display, 0,1);
	lcdPuts(display,lower);
	free(lower);
}

void lcdPrint_lower2(channel *ch, int out, int display){
	int tiles=(int)(ch->fader_val[out]*11);
	int percent= (int) (ch->fader_val[out]*100);
	for (int i=0; i<16;i++){
		lcdPosition(display,i,1);
		if (i<=tiles){
			lcdPutchar(display,4);
		}
		else if(i<=11){
			lcdPutchar(display,' ');
		}

		else if(i>11){
			if (ch->is_muted){
				lcdPuts(display,"Mute");
				break;
			}
			else{
				if (percent==100){
					lcdPuts(display,"100");
				}
				else{
					lcdPutchar(display,' ');
					lcdPosition(display,i+1,1);
					lcdPutchar(display,'0'+(percent/10));
					lcdPosition(display,i+2,1);
					lcdPutchar(display,'0'+(percent%10));
				}
				lcdPosition(display,15,i);
				lcdPutchar(display,'%');
				break;
			}
		}
	}
	lcdHome(display);
}

void lcdPrint_upper(channel ** ch_list,char** output_names, int total_ch, int output){

}

#endif
