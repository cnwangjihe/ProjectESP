#define CAMERA_MODEL_AI_THINKER
#include <Arduino.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
// #include <WiFiUdp.h>
// #include <HTTPClient.h>
// #include "esp_http_client.h"
// #include "esp_websocket_client.h"
#include "esp_camera.h"
#include "camera_pins.h"

#define RX_MAX_LEN 2048
IPAddress RemoteIP(192, 168, 137, 1);
const uint16_t CommandPort = 5555,
               StreamPort = 5556,
               LocalPort = 1926;
const char ssid[] = "DESKTOP-KDLUQ7R 5100",
           pswd[] = "12435687";
const char bound[] = {0xED, 0x4D, 0xB9, 0x6F},
           pbgn[]  = {0xDE, 0xDA, 0xBE, 0xFE},
           pend[]  = {0xED, 0xF4};
AsyncUDP udp;
struct sockaddr_in sudp_tar;
int sudp;
uint8_t *buf;
size_t pos;

// AsyncUDP sudp;
// const char http_host[] = "192.168.137.1",
//            http_path[] = "/upload";
// const int http_port    = 5556,
//           http_timeout = 300;
// esp_http_client_config_t http_config;
// esp_http_client_handle_t http_client;
// const char ws_uri[] = "ws://192.168.137.1:5556/upload";
// esp_websocket_client_config_t ws_config;
// esp_websocket_client_handle_t ws_client;

// uint8_t *trash = NULL;

HardwareSerial CommandSerial(1);

void udp_callback(AsyncUDPPacket &packet)
{
  CommandSerial.write(pbgn, 4);
  CommandSerial.write(packet.data(), packet.length());
  CommandSerial.write(pend, 2);
}

void CameraDataTask(void *p)
{
  // int64_t cs, ce, hs, he;
  camera_fb_t *fb = NULL;
  TickType_t ticks = xTaskGetTickCount();
  for (;;)
  {
    // cs = esp_timer_get_time();
    fb = esp_camera_fb_get();
    // ce = esp_timer_get_time();
    // Serial.printf("JPEG_LEN:%d\n", fb->len);
    if (!fb)
      Serial.println("Camera capture failed");
    else
    {
      // Serial.printf("%p\n", fb);
      // Serial.printf("Format: %d\n", fb->format);
      // hs = esp_timer_get_time();
      // esp_websocket_client_send_text(ws_client,"shit",4,pdMS_TO_TICKS(400));
      // esp_websocket_client_send_bin(ws_client,(char *)fb->buf,fb->len,pdMS_TO_TICKS(4000));
      // http_client = esp_http_client_init(&http_config);
      // esp_http_client_set_header(http_client,"Content-Type","image/jpeg");
      // esp_http_client_set_header(http_client,"Connection","keep-alive");
      // esp_http_client_set_post_field(http_client,(char *)(fb->buf),fb->len);
      // esp_http_client_perform(http_client);
      // esp_http_client_cleanup(http_client);
      // sudp.write(fb->buf, fb->len);
      sendto(sudp, fb->buf, fb->len, 0, (struct sockaddr*) &sudp_tar, sizeof(sudp_tar));
      // he = esp_timer_get_time();
      // Serial.printf("Camera: %us, transmisson: %us\n", (uint32_t)((ce - cs) / 1000), (uint32_t)((he - hs) / 1000));
      esp_camera_fb_return(fb);
    }
    ticks = xTaskGetTickCount() - ticks;
    ticks = (ticks >= pdMS_TO_TICKS(10)) ? 0 : (pdMS_TO_TICKS(10) - ticks);
    vTaskDelay(ticks);
    ticks = xTaskGetTickCount();
  }
}

void WifiSetup()
{
  WiFi.mode(WIFI_STA);
  // WiFi.config(staticIP, gateway, subnet);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, pswd);
  Serial.println(WiFi.waitForConnectResult());
  Serial.println("Wifi init OK!");
}

void CameraSetup()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 13;
  config.fb_count = 2;
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  // sensor_t *s = esp_camera_sensor_get();
  // s->set_reg(s, REG32_CIF, 0xFF, 0x09);
  // s->set_reg(s, CLKRC, 0xFF, 0x00 | CLKRC_2X);
  // s->set_framesize(s, FRAMESIZE_QVGA);
  Serial.println("Camera init OK!");
}

void ConnectionSetup()
{
  udp.connect(RemoteIP, CommandPort);
  udp.onPacket(udp_callback);
  sudp_tar.sin_addr.s_addr = (uint32_t)RemoteIP;
  sudp_tar.sin_family = AF_INET;
  sudp_tar.sin_port = htons(StreamPort);
  sudp = socket(AF_INET, SOCK_DGRAM, 0);
  int yes = 1;
  setsockopt(sudp,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
  struct sockaddr_in addr;
  memset((char *) &addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(LocalPort);
  addr.sin_addr.s_addr = (in_addr_t)IPAddress(INADDR_ANY);
  bind(sudp , (struct sockaddr*)&addr, sizeof(addr));
  fcntl(sudp, F_SETFL, O_NONBLOCK);
  // sudp.connect(RemoteIP, StreamPort);
  // ws_config.uri = ws_uri;
  // ws_config.buffer_size = 40960;
  // ws_client = esp_websocket_client_init(&ws_config);
  // esp_websocket_client_start(ws_client);
  // http_config.host = http_host;
  // http_config.port = http_port;
  // http_config.path = http_path;
  // http_config.timeout_ms = http_timeout;
  // http_config.method = HTTP_METHOD_POST;
}

void setup()
{
  Serial.begin(115200);
  // 12 tx, 13 rx
  CommandSerial.begin(1152000, SERIAL_8N1, 13, 12);
  CommandSerial.setRxBufferSize(RX_MAX_LEN);
  buf = (uint8_t *)malloc(RX_MAX_LEN);
  pos = 0;
  WifiSetup();
  ConnectionSetup();
  CameraSetup();
  xTaskCreatePinnedToCore(CameraDataTask, "CameraDataTask", 4096, NULL, 3, NULL, CONFIG_BLUEDROID_PINNED_TO_CORE);
  // trash = (uint8_t *)malloc(512);
  // memset(trash,0xFF,512);
}

void loop()
{
  if (CommandSerial.available())
  {
    buf[pos++] = CommandSerial.read();
    // Serial.printf("%x\n",buf[pos-1]);
    if (pos >= 4 && buf[pos-4] == bound[0] && buf[pos-3] == bound[1] && buf[pos-2] == bound[2] && buf[pos-1] == bound[3])
    {
      if (pos != 4)
      {
        udp.write(buf,pos-4);
        Serial.println("Sent!");
      }
      pos = 0;
    }
    if (pos == RX_MAX_LEN)
    {
      pos = 0;
      Serial.println("Rx buffer overflowed!");
    }
  }
  delay(10);
  // udp.write(trash,512);
}