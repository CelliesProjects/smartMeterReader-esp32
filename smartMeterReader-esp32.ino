/*
    This is a serial to websocket server for application with a DSMR 5.0.2 conform 'Dutch Smart Meter'.

    This sketch reads telegrams (terminated by a checksum) from a smart meter connected through Serial2 on RXD_PIN

    and pushes these telegrams to websocket clients connected on '/raw'.

    There is also a websocket '/current' pushing the current usage stats to connected clients.

    Finally there is a webserver at port 80 with an index page that connects to websocket '/current' and shows energy use info.

    How to connect to a p1 port see: https://github.com/matthijskooijman/arduino-dsmr#connecting-the-p1-port
*/

#include <driver/uart.h>
#include <AsyncTCP.h>              /* https://github.com/me-no-dev/AsyncTCP */
#include <ESPAsyncWebServer.h>     /* https://github.com/me-no-dev/ESPAsyncWebServer */
#include <SSD1306.h>               /* Install via 'Manage Libraries' in Arduino IDE -> https://github.com/ThingPulse/esp8266-oled-ssd1306 */
#include <dsmr.h>                  /* https://github.com/matthijskooijman/arduino-dsmr */

#include "wifisetup.h"
#include "index_htm.h"
#include "decoded_fields.h"

/* settings for smartMeter */
#define RXD_PIN                         (26)
#define BAUDRATE                        (115200)
#define UART_NR                         (UART_NUM_2)

/* settings for a ssd1306 oled screen */
#define OLED_ADDRESS                    (0x3C)
#define I2C_SDA_PIN                     (5)
#define I2C_SCL_PIN                     (4)

/* settings for ntp time sync */
const char* NTP_POOL =                  "pool.ntp.org";
const char* TIMEZONE =                  "CET-1CEST,M3.5.0/2,M10.5.0/3"; /* Central European Time - see http://www.remotemonitoringsystems.ca/time-zone-abbreviations.php */

#define SET_STATIC_IP                   false            /* If SET_STATIC_IP is set to true then STATIC_IP, GATEWAY, SUBNET and PRIMARY_DNS have to be set to some sane values */

const IPAddress STATIC_IP(192, 168, 0, 90);              /* This should be outside your router dhcp range! */
const IPAddress GATEWAY(192, 168, 0, 1);                 /* Set to your gateway ip address */
const IPAddress SUBNET(255, 255, 255, 0);                /* Usually 255,255,255,0 check in your router or pc connected to the same network */
const IPAddress PRIMARY_DNS(192, 168, 0, 30);            /* Check in your router */
const IPAddress SECONDARY_DNS(192, 168, 0, 50);          /* Check in your router */

const char*     WS_RAW_URL{"/raw"};
const char*     WS_CURRENT_URL{"/current"};

AsyncWebServer  server(80);
AsyncWebSocket  ws_raw(WS_RAW_URL);
AsyncWebSocket  ws_current(WS_CURRENT_URL);
HardwareSerial  smartMeter(UART_NR);
SSD1306         oled(OLED_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN);
bool            oledFound{false};
uint8_t         currentMonthDay;

void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nsmartMeterReader-esp32\n\nConnecting to %s...\n", WIFI_NETWORK);

  /* check if oled display is present */
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.beginTransmission(OLED_ADDRESS);
  uint8_t error = Wire.endTransmission();
  if (error)
    Serial.println("No SSD1306 oled found.");
  else {
    oledFound = true;
    oled.init();
    oled.setContrast(10, 5, 0);
    oled.setTextAlignment(TEXT_ALIGN_CENTER);
    oled.setFont(ArialMT_Plain_16);
    oled.drawString(oled.width() >> 1, 0, "Connecting..");
    oled.display();
  }

  /* try to set a static IP */
  if (SET_STATIC_IP && !WiFi.config(STATIC_IP, GATEWAY, SUBNET, PRIMARY_DNS, SECONDARY_DNS))
    Serial.println("Setting static IP failed");

  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
  WiFi.setSleep(false);

  while (!WiFi.isConnected())
    delay(10);
  Serial.printf("Connected: %s\n", WiFi.localIP().toString().c_str());

  if (oledFound) {
    oled.clear();
    oled.drawString(oled.width() >> 1, 0, WiFi.localIP().toString());
    oled.display();
  }

  /* sync the clock with ntp */
  configTzTime(TIMEZONE, NTP_POOL);

  struct tm timeinfo = {0};

  while (!getLocalTime(&timeinfo, 0))
    delay(10);
  currentMonthDay = timeinfo.tm_mday;

  /* websocket setup */
  ws_raw.onEvent(onEvent);
  server.addHandler(&ws_raw);

  ws_current.onEvent(onEvent);
  server.addHandler(&ws_current);

  /* webserver setup */
  static const char* HTML_HEADER = "text/html";

  server.on("/", HTTP_GET, [] (AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, HTML_HEADER, index_htm, index_htm_len);
    request->send(response);
  });
  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404);
  });
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();

  /* start listening on the smartmeter */
  smartMeter.begin(BAUDRATE, SERIAL_8N1, RXD_PIN);

  Serial.printf("Listening on HardwareSerial(%i) with RXD_PIN=%i\n", UART_NR, RXD_PIN);
}

void loop() {
  static String telegram{""};

  ws_raw.cleanupClients();
  ws_current.cleanupClients();

  while (smartMeter.available()) {
    const char incomingChar = smartMeter.read();
    telegram.concat(incomingChar);
    if ('!' == incomingChar) parseAndSend(telegram);
  }
  delay(1);
}

char currentUseString[200];

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  switch (type) {

    case WS_EVT_CONNECT :
      ESP_LOGI(TAG, "[%s][%u] connect", server->url(), client->id());
      if (0 == strcmp(WS_CURRENT_URL, server->url()))
        client->text(currentUseString);
      break;

    case WS_EVT_DISCONNECT :
      ESP_LOGI(TAG, "[%s][%u] disconnect: %u", server->url(), client->id());
      break;

    case WS_EVT_ERROR :
      ESP_LOGE(TAG, "[%s][%u] error(%u): %s", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
      break;

    case WS_EVT_DATA : {
        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        // here all data is contained in a single packet - and since we only connect and listen we do not check for multi-packet or multi-frame telegrams
        if (info->final && info->index == 0 && info->len == len) {
          if (info->opcode == WS_TEXT) {
            data[len] = 0;
            ESP_LOGD(TAG, "ws message from client %i: %s", client->id(), reinterpret_cast<char*>(data));
          }
        }
      }
      break;

    default : ESP_LOGE(TAG, "unhandled ws event type");
  }
}

void parseAndSend(String& telegram) {
  /* checksum reached, wait for and read 6 more bytes then the telegram is received completely - see DSMR 5.0.2 ¶ 6.2 */
  while (smartMeter.available() < 6) delay(1);

  while (smartMeter.available()) telegram.concat((char)smartMeter.read());

  ws_raw.textAll(telegram);

  currentUseData data;
  const ParseResult<void> res = P1Parser::parse(&data, telegram.c_str(), telegram.length());

  if (res.err) {
    ESP_LOGE(TAG, "%s", res.fullError(telegram.c_str(), telegram.c_str() + telegram.length()));
    telegram = "";
    return;
  }
  telegram = "";

  static struct {
    uint32_t t1Start{0};
    uint32_t t2Start{0};
    uint32_t gasStart{0};
  } today;

  if (!today.t1Start) {
    today.t1Start = data.energy_delivered_tariff1.int_val();
    today.t2Start = data.energy_delivered_tariff2.int_val();
    today.gasStart = data.gas_delivered.int_val();
  };

  snprintf(currentUseString, sizeof(currentUseString), "%i\n%i\n%i\n%i\n%i\n%i\n%i\n%s",
           data.power_delivered.int_val(),
           data.energy_delivered_tariff1.int_val(),
           data.energy_delivered_tariff2.int_val(),
           data.gas_delivered.int_val(),
           data.energy_delivered_tariff1.int_val() - today.t1Start,
           data.energy_delivered_tariff2.int_val() - today.t2Start,
           data.gas_delivered.int_val() - today.gasStart,
           (data.electricity_tariff.equals("0001")) ? "laag" : "hoog"
          );

  ws_current.textAll(currentUseString);

  if (oledFound) {
    oled.clear();
    oled.setFont(ArialMT_Plain_16);
    oled.drawString(oled.width() >> 1, 0, WiFi.localIP().toString());
    oled.setFont(ArialMT_Plain_24);
    oled.drawString(oled.width() >> 1, 18, String(data.power_delivered.int_val()) + "W");
    oled.display();
  }

  /* check if we changed day and update starter values if so */
  struct tm timeinfo = {0};
  getLocalTime(&timeinfo);
  if (currentMonthDay != timeinfo.tm_mday) {
    today.t1Start = data.energy_delivered_tariff1.int_val();
    today.t2Start = data.energy_delivered_tariff2.int_val();
    today.gasStart = data.gas_delivered.int_val();
    currentMonthDay = timeinfo.tm_mday;
  }
}
