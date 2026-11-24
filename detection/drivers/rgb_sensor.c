#include "rgb_sensor.h"
#include <stdio.h>
//#include <stdlib.h>

#ifndef RGB_SENSOR_REPLAY_PATH
#define RGB_SENSOR_REPLAY_PATH "1_no more drama.txt"
#endif

static FILE *replay_file;
static RgbReading latest;

void rgb_sensor_init(void){
    if (replay_file != NULL){ // we could continue reading from a file, and automatically write new info from the sensor to it as a log base
        fclose(replay_file);
    }

    latest = (RgbReading){0};
    replay_file = fopen(RGB_SENSOR_REPLAY_PATH, "r"); // this will need to be replaced with live data or a log that updates within milliseconds
}

void rgb_sensor_poll(void){

}

RgbReading rgb_sensor_get_latest(void){
    return latest;
}