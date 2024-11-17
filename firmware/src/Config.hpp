#pragma once

#include <ArduinoJson.h>
#include <LittleFS.h>

#include "constants.h"
#include "model.hpp"
#include "utils.hpp"

typedef struct
{
    bool valid;
    String line1;
    String line2;
} Lines;

class Config
{
private:
    String _wifi_ssid;
    String _wifi_pass;
    String _auth_user;
    String _auth_pass;
    Ac _ac;

public:
    Config() {}

    const char *wifi_ssid() const
    {
        return _wifi_ssid.c_str();
    }

    const char *wifi_pass() const
    {
        return _wifi_pass.c_str();
    }

    const char *auth_user() const
    {
        return _auth_user.c_str();
    }

    const char *auth_pass() const
    {
        return _auth_pass.c_str();
    }

    const Ac &ac() const
    {
        return _ac;
    }

    bool save()
    {
        File file = LittleFS.open(CONFIG_FILE, FILE_WRITE);
        if (!file)
        {
            error("Failed to open config file for writing");
            return false;
        }

        JsonDocument data;
        JsonObject ac_data = data[KEY_AC].to<JsonObject>();
        serialize(_ac, ac_data);

        size_t s = serializeJson(data, file);
        file.close();

        if (s == 0)
        {
            error("Failed to write config file");
            return false;
        }
        else
        {
            info("Saved config file");
            return true;
        }
    }

    bool load()
    {
        File file = LittleFS.open(CONFIG_FILE);
        if (!file || file.size() == 0)
        {
            info("config file not found, creating a new one");
            init();
            return save();
        }
        else
        {
            info("Read config file");

            JsonDocument data;
            auto nok = deserializeJson(data, file);
            if (nok)
            {
                error("Config file invalid");
            }
            else
            {
                deserialize(_ac, data[KEY_AC]);
            }

            file.close();
            return !nok;
        }
    }

    void set(const JsonVariant &json)
    {
        JsonObjectConst data = json.as<JsonObjectConst>();
        deserialize(_ac, data[KEY_AC]);
        save();
    }

    bool loadWifi()
    {
        Lines res = readFileTwoLines(WIFI_CONFIG_FILE);
        if (!res.valid)
        {
            return false;
        }
        else
        {
            _wifi_ssid = res.line1;
            _wifi_pass = res.line2;
            return true;
        }
    }

    bool loadAuth()
    {
        Lines res = readFileTwoLines(AUTH_CONFIG_FILE);
        if (!res.valid)
        {
            return false;
        }
        else
        {
            _auth_user = res.line1;
            _auth_pass = res.line2;
            return true;
        }
    }

    bool saveWifi(const String ssid, const String pass)
    {
        if (ssid.isEmpty() || pass.isEmpty())
        {
            error("Invalid ssid/password");
            return false;
        }

        _wifi_ssid = ssid;
        _wifi_pass = pass;

        return writeFileTwoLines(WIFI_CONFIG_FILE, _wifi_ssid, _wifi_pass);
    }

    bool saveAuth(const String user, const String pass)
    {
        if (user.isEmpty() || pass.isEmpty())
        {
            error("Invalid user/password");
            return false;
        }

        _auth_user = user;
        _auth_pass = pass;

        return writeFileTwoLines(AUTH_CONFIG_FILE, _auth_user, _auth_pass);
    }

    bool saveAll()
    {
        return save() && saveWifi(_wifi_ssid, _wifi_pass) && saveAuth(_auth_user, _auth_pass);
    }

private:
    void init()
    {
        _ac.name = AC_NAME;
        _ac.mode = SCHEDULE;
        _ac.temp = 20;
        _ac.fan = 0;
        _ac.weekday.push_back({0, OFF});
        _ac.weekend.push_back({0, OFF});
    }

    void deserialize(Ac &ac, const JsonObjectConst &data)
    {
        ac.name = AC_NAME;
        ac.mode = strToMode(data["mode"]);
        ac.temp = data["temp"];
        ac.fan = data["fan"];
        deserialize(ac.weekday, data["weekday"].as<JsonArrayConst>());
        deserialize(ac.weekend, data["weekend"].as<JsonArrayConst>());
    }

    void deserialize(std::vector<TimeItem> &items, const JsonArrayConst &data)
    {
        items.clear();
        for (size_t i = 0; i < data.size(); i++)
        {
            items.push_back({(u8_t)data[i]["hour"], strToMode(data[i]["mode"])});
        }
    }

    void serialize(const Ac &ac, JsonObject &data)
    {
        data["name"] = ac.name;
        data["mode"] = modeToStr(ac.mode);
        data["temp"] = ac.temp;
        data["fan"] = ac.fan;
        JsonArray weekday_data = data["weekday"].to<JsonArray>();
        serialize(ac.weekday, weekday_data);
        JsonArray weekend_data = data["weekend"].to<JsonArray>();
        serialize(ac.weekend, weekend_data);
    }

    void serialize(const std::vector<TimeItem> &items, JsonArray &data)
    {
        for (uint i = 0; i < items.size(); i++)
        {
            data[i]["hour"] = items[i].hour;
            data[i]["mode"] = modeToStr(items[i].mode);
        }
    }

    Lines readFileTwoLines(const char *path)
    {
        debug("Read file", path);

        File file = LittleFS.open(path);
        if (!file || file.size() == 0)
        {
            info("File not found");
            return {false};
        }

        String line1 = file.readStringUntil('\n');
        String line2 = file.readStringUntil('\n');
        file.close();

        if (line1.isEmpty() || line2.isEmpty())
        {
            error("File is empty or invalid");
            return {false};
        }

        return {true, line1, line2};
    }

    bool writeFileTwoLines(const char *path, const String &line1, const String &line2)
    {
        debug("Write file", path);

        File file = LittleFS.open(path, FILE_WRITE);
        if (!file)
        {
            error("Failed to open file for writing");
            return false;
        }

        bool ok = file.print(line1);
        if (ok)
            ok = file.print('\n');
        if (ok)
            ok = file.print(line2);
        if (ok)
            ok = file.print('\n');

        file.close();

        if (!ok)
        {
            error("Failed to write file");
        }

        return ok;
    }
};
