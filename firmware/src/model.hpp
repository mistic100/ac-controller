#pragma once

#include <Arduino.h>
#include <vector>
#include "utils.hpp"

typedef enum
{
    SCHEDULE,
    AUTO,
    HEAT,
    COOL,
    FAN,
    OFF
} Mode;

typedef struct
{
    u8_t hour;
    Mode mode;
} TimeItem;

typedef struct
{
    std::string name;
    Mode mode;
    u8_t temp;
    u8_t fan;
    std::vector<TimeItem> weekday;
    std::vector<TimeItem> weekend;
} Ac;

const String modeToStr(const Mode mode)
{
    switch (mode)
    {
    case SCHEDULE:
        return "schedule";
    case AUTO:
        return "auto";
    case HEAT:
        return "heat";
    case COOL:
        return "cool";
    case FAN:
        return "fan";
    case OFF:
        return "off";
    default:
        error("Invalid mode");
        return "off";
    }
}

const Mode strToMode(const char *mode)
{
    if (strcmp(mode, "schedule") == 0)
    {
        return SCHEDULE;
    }
    if (strcmp(mode, "auto") == 0)
    {
        return AUTO;
    }
    if (strcmp(mode, "heat") == 0)
    {
        return HEAT;
    }
    if (strcmp(mode, "cool") == 0)
    {
        return COOL;
    }
    if (strcmp(mode, "fan") == 0)
    {
        return FAN;
    }
    if (strcmp(mode, "off") == 0)
    {
        return OFF;
    }
    error("Invalid mode");
    return OFF;
}
