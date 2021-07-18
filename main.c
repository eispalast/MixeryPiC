#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include "channel.h"
#include "lib/LCD.h"
#include "lib/network.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <wiringPi.h>
#include <lcd.h>
#include <signal.h>
#include <pthread.h>



//define all GPIO-PINS
#define LCD_RS 4
#define LCD_E  17
#define LCD_DATA4 18
#define LCD_DATA5 22
#define LCD_DATA6 23
#define LCD_DATA7 24

//define buttons for Channel switching
#define buttonCh1 26
#define buttonCh2 21
#define buttonCh3 20
#define buttonCh4 16
#define buttonCh5 12
#define buttonCh6 7
#define buttonCh7 8
#define buttonCh8 25


//defining the other buttons
#define buttonToggleLayer 19
#define buttonMuteOutputs 13
#define buttonFineMode 9
#define buttonToggleMuteCh 6

//defining RotaryEncoders
#define volKnobCLK 5
#define volKnobDT 11
#define outKnobCLK 10
#define outKnobDT 27


//Global variables
int display;
int output=0;
int bank=0;
int active_channel=0;
int total_channel=0;
channel ** channel_list;
char** output_names;
int times_updated=0;
int changed=0;
float fine_mode=1;
short all_muted=0;




//converts a float into an array of 4 Bytes
uint8_t* f2b(float f_val){
    uint8_t *bytes=calloc(4,sizeof(uint8_t));
    int32_t* fasi=&f_val;
    bytes[0]=0|(*fasi)>>24;
    bytes[1]=(0x00FF0000&*fasi)>>16;
    bytes[2]=(0x0000FF00&*fasi)>>8;
    bytes[3]=(0x000000FF&*fasi);
    return bytes;

}

//converts an array of 4 bytes into a float
float b2f(uint8_t* b_val){
    float *f_val=0;
    uint32_t fasi=0;
    fasi=0|(b_val[0]<<24);
    fasi=fasi|((0|b_val[1])<<16);
    fasi=fasi|((0|b_val[2])<<8);
    fasi=fasi|((0|b_val[3])<<8);
    f_val=&fasi;
    return *(f_val);
}


char* fill0(int num){
    char* outp="__";
    outp[0]=('0')+num/10;
    outp[1]='0'+num % 10;
    return outp;
}


char* name_req_msg(char* num){
    char* msg=calloc(28,sizeof(uint8_t));
    sprintf(msg,"/ch/%s/config/name%c%c",num,0,0);
    return msg;
}

void refresh_channel_name(channel* chan){
    char* req_m=name_req_msg(chan->number);
    int max_tries=3;
    int tries=0;
    net_ans* answer;
    bool failed=true;
    while(failed && tries<max_tries){
        to_mixer(req_m,20);
        answer=from_mixer();
        tries++;
        if (answer->len>-1){
            failed=false;
        }
    }    
    if (answer->len<=0){
        sprintf(chan->name,"Chan %s",chan->number);
    }
    else{
        int i=0;
        int j=24;
        while(answer->msg[j]!='\0'){
            chan->name[i]=answer->msg[j];
            i++;
            j++;
        }
        chan->name[i]='\0';
    }
    free(req_m);
    free(answer->msg);
    free(answer);
}

char* fader_set_msg(channel* c, int output){
    char* msg= calloc(29,sizeof(char));
    char *bs=f2b(c->fader_val[output]);
    if (output==0){
        sprintf(msg,"/ch/%s/mix/fader%c%c%c%c,f%c%c%c%c%c%c",c->number,0,0,0,0,0,0,bs[0],bs[1],bs[2],bs[3]);
    }
    else{
        sprintf(msg,"/ch/%s/mix/0%d/level%c,f%c%c%c%c%c%c",c->number,(output),0,0,0,bs[0],bs[1],bs[2],bs[3]);
    }
    free(bs);
    return msg;
}

//sends a fader value for a specific  channel to the mixer
void set_fader(channel * c, int output){
    char *set_msg=fader_set_msg(c,output);
    to_mixer(set_msg,28);
    free(set_msg);
}

char *fader_req_msg(channel *c, int output){
    char* msg= calloc(28,sizeof(char));
    if (output==0){
        sprintf(msg,"/ch/%s/mix/fader%c%c",c->number,0,0);
    }
    else{
        sprintf(msg,"/ch/%s/mix/0%d/level%c%c",c->number,(output),0,0);
    }
    
    return msg;
}



/*gets the fader value for a given channel from the mixer
 *You might notice the while(finde_mixer)-loop. This is because this function is called
 periodically and also has the functionality of checking whether there is still
 a connection to the mixer. It's not very pretty. Will fix that in the future (#TODO)
 */
int refresh_fader_value(channel* chan, int output){
    float before=chan->fader_val[output];
    char *req_m=fader_req_msg(chan, output);
    int max_tries=3;
    int tries=0;
    net_ans* answer;
    bool failed=true;
    
    while(failed && tries<max_tries){
        to_mixer(req_m,21);
        answer=from_mixer();
        tries++;
        if (answer->len!=-1){
            failed=false;
        }
    }
    
    if (answer->len!=0 && answer->len!=-1){
        char* bval=answer->msg;
        bval=bval+sizeof(char)*answer->len-4;
        chan->fader_val[output]=b2f(bval);
    }
    else if (answer->len==-1){
        struct timespec ts;
        ts.tv_sec=0;
        ts.tv_nsec=2e8;
        while(find_mixer()==-1){
            lcdClear(display);
            lcdHome(display);
            lcdPrintf(display,"Lost  connection");
            for (int i=0;i<16;i++){
                lcdPosition(display,i,1);
                lcdPrintf(display,".");
                nanosleep(&ts,NULL);
            }
        }
        refresh_status(1);
    }
    free(answer->msg);
    free(answer);
    free(req_m);
    if (before!=chan->fader_val[output]){
        return 1;
    }
    else {
        return 0;
    }
}


char* aux_name_req_msg(int number){
    char* req_m=calloc(21, sizeof(char));
    sprintf(req_m,"/bus/%d/config/name",number);
    return req_m;
}

char* get_aux_name(int number){
    char *req_m=aux_name_req_msg(number);
    int max_tries=3;
    int tries=0;
    net_ans* answer;
    bool failed=true;
    
    while(failed && tries<max_tries){
        to_mixer(req_m,20);
        answer=from_mixer();
        tries++;
        if (answer->len!=-1){
            failed=false;
        }
    }
    if(answer->len>0){
        return (answer->msg)+24;
    }
    else{
        return "not defined";
    }
    
}

char* mute_req_msg(channel* c){
    char* req_m=calloc(28, sizeof(char));
    sprintf(req_m,"/ch/%s/mix/on",c->number);
    return req_m;
}

char* mute_set_msg(channel * c,int on){
    char* msg=calloc(25,sizeof(char));
    sprintf(msg,"/ch/%s/mix/on%c%c%c,i%c%c%c%c%c%c",c->number,0,0,0,0,0,0,0,0,on);
    return msg;
}

//returns 1 if is_muted changed
int refresh_mute(channel * c){
    bool before=c->is_muted;
    char *req_m=mute_req_msg(c);
    int max_tries=3;
    int tries=0;
    net_ans* answer;
    bool failed=true;
    while(failed && tries<max_tries){
        to_mixer(req_m,21);
        answer=from_mixer();
        tries++;
        if (answer->len!=-1){
            failed=false;
        }
    }
    if(answer->len>0){
        if (answer->msg[answer->len-1]==1){
            c->is_muted=false;
            
        }
        else{
            c->is_muted=true;
           
        }
    }
    free(req_m);
    free(answer->msg);
    free(answer);
    if (before!=c->is_muted){
        return 1;
    }
    else{
        return 0;
    }
}

void toggle_mute(channel * c){
    char* mute_msg;
    if (c->is_muted==true){
        mute_msg=mute_set_msg(c,1);
        c->is_muted=false;
    }
    else{
        mute_msg=mute_set_msg(c,0);
        c->is_muted=true;
    }
    all_muted=0;
    to_mixer(mute_msg,24);
    free(mute_msg);
}

void startup(channel** channel_list,char** output_names){
    
    for (int i=1; i<17;i++){
        channel_list[i-1]=calloc(1,sizeof(channel));
        
        //Initialize Number
        channel_list[i-1]->number=calloc(3,sizeof(char));
        if(i<10){
            channel_list[i-1]->number[0]='0';
            channel_list[i-1]->number[1]='0'+i;

        }
        else{
            channel_list[i-1]->number[0]='1';
            channel_list[i-1]->number[1]='0'+(i%10);

        }

        //Init Name
        channel_list[i-1]->name=calloc(33,sizeof(char));
        refresh_channel_name(channel_list[i-1]);
        printf("%s\n",channel_list[i-1]->name);
        //Init mute-status and faders
        refresh_mute(channel_list[i-1]);
        for (int j=0; j<7;j++){
            refresh_fader_value(channel_list[i-1],j);
        }
    }

    //get Aux-names
    output_names[0]="Main";
    for (int i=1;i<7;i++){
        output_names[i]=calloc(32,sizeof(char));
        output_names[i]=get_aux_name(i);
        printf("Aux %d = %s",i,output_names[i]);
    }
    
}


void initialize_pins(){
    pinMode(LCD_RS, OUTPUT);
    pinMode(LCD_E, OUTPUT);
	pinMode(LCD_DATA4, OUTPUT);
	pinMode(LCD_DATA5, OUTPUT);
	pinMode(LCD_DATA6, OUTPUT);
	pinMode(LCD_DATA7, OUTPUT);
    
    pinMode(buttonCh1, INPUT);
    pinMode(buttonCh2, INPUT);
    pinMode(buttonCh3, INPUT);
    pinMode(buttonCh4, INPUT);
    pinMode(buttonCh5, INPUT);
    pinMode(buttonCh6, INPUT);
    pinMode(buttonCh7, INPUT);
    pinMode(buttonCh8, INPUT);
    
    pinMode(buttonToggleLayer, INPUT);
    pinMode(buttonToggleMuteCh, INPUT);
    pinMode(buttonFineMode, INPUT);
    pinMode(buttonMuteOutputs, INPUT);
    
    pinMode(volKnobCLK, INPUT);
    pinMode(volKnobDT, INPUT);
    pinMode(outKnobCLK, INPUT);
    pinMode(outKnobDT, INPUT);
    display= lcdInit(2,16,4,LCD_RS,LCD_E,LCD_DATA4,LCD_DATA5,LCD_DATA6,LCD_DATA7,0,0,0,0);
    
}


void refresh_status(int switched){
    if (switched==0){
        lcdPrint_lower2(channel_list[total_channel],output, display);
    }
    else if (switched){
        lcdPrint_status(channel_list,total_channel,output_names,output,display);
    }
    lcdCursor(display,false);
}

void *refresh_display(void * val){
    struct timespec ts;
    ts.tv_sec=0;
    ts.tv_nsec=1e8;

    while(1){
        nanosleep(&ts,NULL);
        if (changed){
            if (changed==2){
                lcdPrint_lower2(channel_list[total_channel],output, display);
            }else{
                lcdPrint_status(channel_list,total_channel,output_names,output,display);
            }
            changed=0;
        }
    }
        
}

//Interrupthandler
void choose1 (void){
    active_channel=0;
    total_channel=active_channel+bank;
    lcdPrint_status(channel_list,total_channel,output_names,output,display);
    
}

void choose2 (void){
    active_channel=1;
    total_channel=active_channel+bank;
    lcdPrint_status(channel_list,total_channel,output_names,output,display);
    
}

void choose3 (void){
    active_channel=2;
    total_channel=active_channel+bank;
    lcdPrint_status(channel_list,total_channel,output_names,output,display);
    
}

void choose4 (void){
    active_channel=3;
    total_channel=active_channel+bank;
    lcdPrint_status(channel_list,total_channel,output_names,output,display);
    
}

void choose5 (void){
    active_channel=4;
    total_channel=active_channel+bank;
    lcdPrint_status(channel_list,total_channel,output_names,output,display);
    
}

void choose6 (void){
    active_channel=5;
    total_channel=active_channel+bank;
    lcdPrint_status(channel_list,total_channel,output_names,output,display);
   
}


void choose7 (void){
    active_channel=6;
    total_channel=active_channel+bank;
    lcdPrint_status(channel_list,total_channel,output_names,output,display);
   
}

void choose8 (void){
    active_channel=7;
    total_channel=active_channel+bank;
    lcdPrint_status(channel_list,total_channel,output_names,output,display);
    
}

void toggle_fine_mode(void){
    if (digitalRead(buttonFineMode)){
        return;
    }
    if (fine_mode==1){
        fine_mode=0.2;
    }
    else{
        fine_mode=1;
    }
}

void toggle_bank(void){
    if (!digitalRead(buttonToggleLayer)){
        return;
    }
    if (bank==0){
        bank=8;
    }
    else{
        bank=0;
    }
    total_channel=active_channel+bank;
    changed=1;
}


void change_vol(void){
    if (digitalRead(volKnobCLK)!=digitalRead(volKnobDT)){
        //Increase volume
        float new_val=channel_list[total_channel]->fader_val[output]+0.02*fine_mode;
        if (new_val>=1.0){
            new_val=1.0;
        }
        channel_list[total_channel]->fader_val[output]=new_val;
        set_fader(channel_list[total_channel],output);
    }
    else {
        //decrease volume
        float new_val=channel_list[total_channel]->fader_val[output]-0.02*fine_mode;
        if (new_val<=0.0){
            new_val=0.0;
        }
        channel_list[total_channel]->fader_val[output]=new_val;
        set_fader(channel_list[total_channel],output);            
    }
    changed=2;

}

void change_output(void){
    if (digitalRead(outKnobCLK)!=digitalRead(outKnobDT)){
        //next output
        output=(output+1)%7;
    }
    else {
        //previous output
        output=(output+6)%7;
    }
    changed=1;

}

void t_mute(void){
    if(digitalRead(buttonToggleMuteCh)){
        return;
    }
    toggle_mute(channel_list[total_channel]);
    changed=1; 
}


void mute_all(void){
    if (!digitalRead(buttonMuteOutputs)){
        return;
    }
    if (all_muted==0){
        for (int i=0; i<16;i++){
            if (channel_list[i]->is_muted==false){
                toggle_mute(channel_list[i]);
            }
        }
        all_muted=1;
    }
    else{
        for (int i=0; i<16;i++){
            toggle_mute(channel_list[i]);
        }
    }
    changed=1;
    sleep(1);
}


void setup_interrupts(){
    wiringPiISR (buttonCh1,INT_EDGE_RISING,&choose1);
    wiringPiISR (buttonCh2,INT_EDGE_RISING,&choose2);
    wiringPiISR (buttonCh3,INT_EDGE_RISING,&choose3);
    wiringPiISR (buttonCh4,INT_EDGE_RISING,&choose4);
    wiringPiISR (buttonCh5,INT_EDGE_RISING,&choose5);
    wiringPiISR (buttonCh6,INT_EDGE_RISING,&choose6);
    wiringPiISR (buttonCh7,INT_EDGE_RISING,&choose7);
    wiringPiISR (buttonCh8,INT_EDGE_RISING,&choose8);
    wiringPiISR (buttonToggleLayer,INT_EDGE_RISING,&toggle_bank);
    wiringPiISR (buttonMuteOutputs,INT_EDGE_RISING,&mute_all);
    wiringPiISR (buttonToggleMuteCh, INT_EDGE_FALLING, &t_mute);
    wiringPiISR (buttonFineMode, INT_EDGE_FALLING, &toggle_fine_mode);
    wiringPiISR (volKnobCLK, INT_EDGE_FALLING, &change_vol);
    wiringPiISR (outKnobCLK, INT_EDGE_FALLING, &change_output);
    

}



int main (int argc,char** argv){
    printf("Welcome, this is MixeryPiC!\n"
           "Make sure your mixer is switched on and in the same network.\n"
           "For more information on how to set the system up visit https://github.com/eispalast/MixeryPiC\n");
    wiringPiSetupGpio();
    initialize_pins();
    setup_interrupts();
    //define Recv-timeout
    to.tv_sec = 1;
    to.tv_usec = 50000;
    
    //set the timeout for the cool load screen animation
    struct timespec ts;
    ts.tv_sec=0;
    ts.tv_nsec=2e8;

    //initialize networksocket
    ini_bc_net();

    //try to find the mixer
    while(find_mixer()==-1){
        lcdClear(display);
        lcdHome(display);
        lcdPrintf(display,"   Initialize   ");
        for (int i=0;i<16;i++){
            lcdPosition(display,i,1);
            lcdPrintf(display,".");
            nanosleep(&ts,NULL);
        }
        printf("Did not find tho mixer... yet\n");
    }
    
    channel_list=calloc(16,sizeof(channel*));
    output_names=calloc(7,sizeof(char*));
    
    //load all information from the mixer
    startup(channel_list,output_names);
   
    lcdPrint_status(channel_list,total_channel,output_names,output,display);
    
    ts.tv_sec=1;
    ts.tv_nsec=0;
    pthread_t lcd_printer;
    pthread_create(&lcd_printer,NULL,&refresh_display,NULL);
    while(1){
        total_channel=active_channel+bank;
        sleep(1);
        changed=refresh_fader_value(channel_list[active_channel], output);
        changed=changed+refresh_mute(channel_list[active_channel]);
      
    }    
    return 0;
}
