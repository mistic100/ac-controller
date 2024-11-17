#pragma once

#define _IR_ENABLE_DEFAULT_ false
#define SEND_FUJITSU_AC true
#define DECODE_FUJITSU_AC false

#include <ArduinoJson.h>
#include <ir_Fujitsu.h>

#include "constants.h"
#include "model.hpp"
#include "Config.hpp"

using namespace stdAc;

class Controller
{
private:
    Config *config;

    Mode mode = OFF;
    IRFujitsuAC ac_ctrl = IRFujitsuAC(PIN_AC, MODEL_AC);

    unsigned long previousMillis = 0;
    bool needsUpdate = false;
    decode_results results;

public:
    Controller(Config *config) : config(config)
    {
    }

    void init()
    {
        ac_ctrl.begin();
        //ac_ctrl.calibrate();
        ac_ctrl.setSwing(kFujitsuAcSwingOff);

        needsUpdate = true;
    }

    void loop()
    {
        unsigned long currentMillis = millis();
        if (needsUpdate || (currentMillis - previousMillis >= UPDATE_INTERVAL))
        {
            update(needsUpdate);
            previousMillis = currentMillis;
            needsUpdate = false;
        }
    }

    void willUpdate()
    {
        needsUpdate = true;
    }

    void getStatus(Print* output) const
    {
        JsonDocument status;
        status[KEY_AC] = modeToStr(mode);
        serializeJson(status, *output);
    }

private:
    void update(bool force = false)
    {
        struct tm time;
        getLocalTime(&time);

        info("Update");
        Ac ac = config->ac();
        Mode newMode = getMode(ac, time);
        if (force || mode != newMode)
        {
            apply(ac, newMode);
            mode = newMode;
        }
    }

    const Mode getMode(const Ac &ac, const struct tm &time) const
    {
        debug("Mode", modeToStr(ac.mode));

        if (ac.mode == SCHEDULE)
        {
            bool isWeekend = time.tm_wday == 0 || time.tm_wday == 6;
            auto timetable = isWeekend ? ac.weekend : ac.weekday;
            return getMode(timetable, time.tm_hour);
        }

        return ac.mode;
    }

    const Mode getMode(const std::vector<TimeItem> &timetable, int hour) const
    {
        for (size_t i = timetable.size() - 1; i >= 0; i--)
        {
            if (hour >= timetable[i].hour)
            {
                debug("Scheduled mode", modeToStr(timetable[i].mode));
                return timetable[i].mode;
            }
        }

        error("No suitable timetable found");
        return OFF;
    }

    void apply(const Ac &ac, Mode newMode)
    {
        if (newMode == OFF) {
            ac_ctrl.setCmd(kFujitsuAcCmdTurnOff);

        } else {
            ac_ctrl.setTemp(ac.temp);

            switch (newMode)
            {
            case AUTO:
                ac_ctrl.setMode(kFujitsuAcModeAuto);
                break;

            case HEAT:
                ac_ctrl.setMode(kFujitsuAcModeHeat);
                break;

            case COOL:
                ac_ctrl.setMode(kFujitsuAcModeCool);
                break;

            case FAN:
                ac_ctrl.setMode(kFujitsuAcModeFan);
                break;
            
            default:
                error("Should not be there");
                ac_ctrl.setMode(kFujitsuAcModeAuto);
                break;
            }

            switch (ac.fan)
            {
            case 0:
                ac_ctrl.setFanSpeed(kFujitsuAcFanAuto);
                break;
            case 1:
                ac_ctrl.setFanSpeed(kFujitsuAcFanQuiet);
                break;
            case 2:
                ac_ctrl.setFanSpeed(kFujitsuAcFanLow);
                break;
            case 3:
                ac_ctrl.setFanSpeed(kFujitsuAcFanMed);
                break;
            case 4:
                ac_ctrl.setFanSpeed(kFujitsuAcFanHigh);
                break;
            default:
                error("Should not be there");
                ac_ctrl.setFanSpeed(kFujitsuAcFanAuto);
                break;
            }

            ac_ctrl.setCmd(kFujitsuAcCmdTurnOn);
        }
        
        ac_ctrl.send();

        debug("AC state\n", ac_ctrl.toString());
        unsigned char* ir_code = ac_ctrl.getRaw();
        Serial.print(" IR Code: 0x");
        for (uint8_t i = 0; i < ac_ctrl.getStateLength(); i++)
            Serial.printf("%02X", ir_code[i]);
        Serial.println();
    }

};
