#include "rgb_sensor.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef RGB_SENSOR_REPLAY_PATH
#define RGB_SENSOR_REPLAY_PATH "1_no more drama.txt"
#endif

static FILE *replay_file;
static RgbReading latest;

void rgb_sensor_init(void){
    if (replay_file != NULL){
        fclose(replay_file);
    }

    latest = (RgbReading){0};
    replay_file = fopen(RGB_SENSOR_REPLAY_PATH, "r"); // this will need to be replaced with live data or a log that updates within milliseconds
}

void rgb_sensor_poll(void){
    if (replay_file == NULL){
        return;
    }

    latest = (RgbReading){0};
    char line[128];
    char red[4], green[4], blue[4];

    while (fgets(line, sizeof(line), replay_file) != NULL){
        if (line[0] != 'R' || (line[1] != ' ' && line[1] != '=')) {
            continue;
        }

        sscanf(line, "R = %3[0-9] | G = %3[0-9] | B = %3[0-9]", red, green, blue);

        const unsigned long r = strtoul(red, NULL, 10);
        const unsigned long g = strtoul(green, NULL, 10);
        const unsigned long b = strtoul(blue, NULL, 10);
        latest = (RgbReading){(uint8_t)r, (uint8_t)g, (uint8_t)b, true};

        return;
    }

    fclose(replay_file);
    replay_file = NULL;
}

RgbReading rgb_sensor_get_latest(void){
    return latest;
}