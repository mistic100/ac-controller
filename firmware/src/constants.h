#pragma once

#define AP_SSID "AP-AC"
#define HOSTNAME "ac-ctrl"

#define AUTH_CONFIG_FILE "/auth.txt"
#define WIFI_CONFIG_FILE "/wifi.txt"
#define CONFIG_FILE "/config.json"

#define AUTH_DEFAULT_USER "admin"
#define AUTH_DEFAULT_PASS "admin"

#define WIFI_RETRIES 5
#define WIFI_RETRY_INTERVAL 5000

// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#define TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"
#define NTP_POOL "pool.ntp.org"

#define AC_NAME "AC"
#define KEY_AC "ac"

#define UPDATE_INTERVAL 10 * 60 * 1000
#define PIN_AC D2
#define MODEL_AC ARREB1E
