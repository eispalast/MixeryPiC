#ifndef CHANNEL_H
#define CHANNEL_H

#include<stdbool.h>
#include<stdint.h>

typedef struct _channel{
    char* number;
    char* name;
    float fader_val[7];
    bool is_muted;
}channel;



#endif
