/** Arduino library **/
#include <Arduino.h>

/** ESP8266 library **/
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>

/** External library **/
#include <../lib/U8g2/U8g2lib.h>
#include <../lib/OneWire/OneWire.h>
#include <../lib/DS18B20/DS18B20.h>
#include <../lib/NTPClient/NTPClient.h>

/** Own-defined header **/
#include "metadata.h"
#include "launch_pic.h"

HTTPClient http_client_weather;

WiFiClient wifi_client_weather;

/** Screen **/
U8G2_SSD1306_128X64_NONAME_F_HW_I2C screen(U8G2_R0, U8X8_PIN_NONE, D1, D2);

/** DS18B20 Sensor **/
DS18B20 ds18B20(new OneWire(D3));

const String months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
const String day[] = {"SUN", "MON", "TUE", "WED", "THU", "FIR", "SAT"};

String date_cur;
String time_cur;
int day_cur;

String temp_min = "-";
String temp_max = "-";
String pressure = "-";
String humidity = "-";
int icon;

bool page = false;

WiFiUDP wifi_udp;
NTPClient time_server(wifi_udp, "pool.ntp.org");

void implement_clock();
void implement_weather();

void get_time();
bool get_weather();

void draw_clock();
void draw_weather();

void draw_bottom_bar();

int get_str_int(const String &);

int title_width = 0;

void setup() {

    Serial.begin(9600);

    Serial.println("===== SCREEN =====");
    Serial.print("- Screen(0)...");

    screen.begin();

    screen.clearBuffer();
    screen.setFont(u8g2_font_wqy12_t_gb2312a);
    title_width = screen.getStrWidth("MAX ");

    // draw_great_wall(screen);
    draw_tian_an_men(screen);

    screen.sendBuffer();

    Serial.println("===== DS18B20 =====");
    ds18B20.begin();
    Serial.println("Done");

    Serial.println("===== Wi-Fi =====");
    Serial.print("- Connecting to " + String(ssid) + "...");
    WiFi.begin(ssid, passwd);
    while (!WiFi.isConnected()) {
        delay(1);
    }
    Serial.println("Connected");

    http_client_weather.setTimeout(2000);

    time_server.begin();
    time_server.setTimeOffset(28800);

    Serial.println("===== SETUP END =====");
    Serial.println();
}

void loop() {

    screen.clearBuffer();
    !page ? implement_clock() : implement_weather();
    draw_bottom_bar();
    screen.sendBuffer();

    page = !page;
    delay(5000);
}

void implement_clock() {
    Serial.println("===== Clock =====");
    get_time();
    draw_clock();
}

void implement_weather() {
    Serial.println("===== Weather =====");
    get_weather() ? draw_weather() : (void) Serial.println("- Getting Data Failed");
}

void get_time() {
    Serial.println("- Getting Data...");
    time_server.update();

    long epoch_time = time_server.getEpochTime();
    struct tm *ptm = gmtime((time_t *) &epoch_time);

    Serial.println("- Getting Data...");
    time_cur = String(time_server.getHours() < 10 ? '0' + String(time_server.getHours()) : String(
            time_server.getHours())) + ':' +
               String(time_server.getMinutes() < 10 ? '0' + String(time_server.getMinutes()) : String(
                       time_server.getMinutes()));
    date_cur = String(ptm->tm_mday) + ' ' + months[ptm->tm_mon] + ' ' + String(ptm->tm_year + 1900);
    day_cur = time_server.getDay();
}

bool get_weather() {
    String resp;
    String icon_str;
    Serial.println("- Getting Data...");
    http_client_weather.begin(wifi_client_weather, weather_url.c_str());
    if (http_client_weather.GET()) {
        resp = http_client_weather.getString();

        Serial.println(resp);

        if (resp.isEmpty()) {
            if (temp_min == "-" || temp_max == "-" || pressure == "-" || humidity == "=")
                return false;
            return true;
        }

        resp = resp.substring(resp.indexOf("\"icon\":") + 8);
        icon_str = resp.substring(0, resp.indexOf("\"}"));

        if (icon_str[0] == '0') {
            if (icon_str[1] == '1') {
                icon = icon_str[2] == 'd' ? 69 : 66;
            } else {
                switch (icon_str[1]) {
                    case '2':
                        icon = 65;
                        break;
                    case '9':
                        icon = 67;
                    case '3':
                    case '4':
                        icon = 64;
                        break;
                }
            }
        } else if (icon_str[1] == '1') {
            icon = 67;
        } else {
            icon = 64;
        }

        Serial.println(icon_str);

        resp = resp.substring(resp.indexOf("\"temp_min\":") + 11);
        temp_min = resp.substring(0, resp.indexOf(",\""));
        Serial.println(temp_min);

        resp = resp.substring(resp.indexOf("\"temp_max\":") + 11);
        temp_max = resp.substring(0, resp.indexOf(",\""));
        Serial.println(temp_max);

        resp = resp.substring(resp.indexOf("\"pressure\":") + 11);
        pressure = resp.substring(0, resp.indexOf(",\""));
        Serial.println(pressure);

        resp = resp.substring(resp.indexOf("\"humidity\":") + 11);
        humidity = resp.substring(0, resp.indexOf("},"));
        Serial.println(humidity);

    }

    return true;
}

void draw_clock() {
    Serial.println("- Drawing Clock...");
    screen.setFont(u8g2_font_freedoomr10_tu);
    auto text_height = screen.getFontAscent();
    screen.drawStr((screen.getWidth() - screen.getStrWidth(date_cur.c_str())) / 2, text_height, date_cur.c_str());
    screen.setFont(u8g2_font_freedoomr25_tn);
    screen.drawStr((screen.getWidth() - screen.getStrWidth(time_cur.c_str())) / 2,text_height * 1.2 + screen.getFontAscent(), time_cur.c_str());
}

void draw_weather() {
    Serial.println("- Drawing Weather...");

    screen.setFont(u8g2_font_open_iconic_weather_4x_t);
    screen.drawGlyph(8, 40, icon);

    screen.setFont(u8g2_font_wqy12_t_gb2312a);

    auto height = screen.getFontAscent() * 1.2;
    auto tmp = temp_max + "°C";
    screen.drawStr(48, height, "MAX");
    screen.drawStr(48 + title_width, height, tmp.c_str());

    height += screen.getFontAscent() * 1.2;
    tmp = temp_max + "°C";
    screen.drawStr(48, height, "MIN");
    screen.drawStr(48 + title_width, height, tmp.c_str());

    height += screen.getFontAscent() * 1.2;
    tmp = humidity + "%";
    screen.drawStr(48, height, "HUM");
    screen.drawStr(48 + title_width, height, tmp.c_str());

    height += screen.getFontAscent() * 1.2;
    tmp = pressure + "hPa";
    screen.drawStr(48, height, "ATM");
    screen.drawStr(48 + title_width, height, tmp.c_str());

}

void draw_bottom_bar() {
    screen.drawLine(0, 48, 128, 48);
    if (!page) {
        screen.drawDisc(59, 56, 3, U8G2_DRAW_ALL);
        screen.drawCircle(69, 56, 3, U8G2_DRAW_ALL);
    } else {
        screen.drawCircle(59, 56, 3, U8G2_DRAW_ALL);
        screen.drawDisc(69, 56, 3, U8G2_DRAW_ALL);
    }
    screen.setFont(u8g2_font_wqy12_t_gb2312a);
    ds18B20.requestTemperatures();
    while (!ds18B20.isConversionComplete());
    auto tmp = (String(ds18B20.getTempC()) + "°C");
    auto height = 48 + (((16 - screen.getFontAscent()) / 2) + screen.getFontAscent());
    screen.drawStr(screen.getWidth() - screen.getStrWidth(tmp.c_str()), height, tmp.c_str());
    screen.drawStr(screen.getWidth() / 20, height, day[day_cur].c_str());
}