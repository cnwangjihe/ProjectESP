#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include "BindableUDP.h"

// IPAddress staticIP(192,168,137,213);
// IPAddress gateway(192,168,137,1);
// IPAddress subnet(255,255,255,0);
IPAddress RemoteIP(192,168,137,1);
const uint16_t RemotePort = 5555,
               LocalPort  = 1926;
const char ssid[] = "DESKTOP-KDLUQ7R 5100",
           pswd[] = "12435687";
const char pbgn[] = {0xDE, 0xAD, 0xBE, 0xEF},
           pend[] = {0xED, 0xF4};

BindableUDP udp;
WiFiUDP aa;
unsigned long st;
uint8_t lraw[5], tmp[5];
uint8_t spi_buffer[20000];
uint8_t tlen;
uint16_t len;
uint64_t ss,sst;

void udp_callback(AsyncUDPPacket& packet)
{
  Serial.write(pbgn,4);
  Serial.write(packet.data(),packet.length());
  Serial.write(pend,2);
}

void WifiSetup()
{
  Serial1.println("Setup..");
  WiFi.mode(WIFI_STA);
  // WiFi.config(staticIP, gateway, subnet);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid,pswd);
  Serial1.println(WiFi.waitForConnectResult());
  udp.connectbind(RemoteIP,RemotePort,LocalPort);
  udp.onPacket(udp_callback);
  Serial1.println("OK!");
}

void setup()
{
  pinMode(5,OUTPUT);
  digitalWrite(5,LOW);
  pinMode(4,INPUT);
  // Serial.setRxBufferSize(1024);
  Serial.begin(1152000);
  Serial1.begin(115200);
  SPI.begin();
  SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));
  WifiSetup();
  digitalWrite(5,HIGH);
  // Serial1.println(ESP.getCpuFreqMHz());
}

bool Send(uint8_t *data, uint16_t len)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial1.println(WiFi.status());
    return false;
  }
  return udp.write(data,len) == len;
}

inline bool check_parity(uint16_t raw)
{
  uint8_t odd,even,all;
  odd = even = all = 0;
  for (uint8_t i=0;i<9;i++)
  {
    odd ^= i & 1 & ( (raw>>i) & 1);
    even ^= ((i & 1) ^ 1) & ( (raw>>i) & 1);
    all ^= ( (raw>>i) & 1);
  }
  return (odd==((raw>>9)&1) && even==((raw>>10)&1) && all==((raw>>11)&1) && (odd^even) == all);
}

void loop()
{
  // Serial1.printf("SW:%llu\n",micros64()-sst);
  // sst = micros64();
  tlen = Serial.readBytesUntil(0xFF,lraw,4); // packge len is 8*n+(2/3), 0xFF is not a valid len
  if (tlen!=2)
    return ;
  // Serial1.printf("Receive something:%d\n",tlen);
  if (Serial.available() > 0 &&  Serial.peek() == 0xFE)
  {
    Serial1.printf("@@@@@@@@%d:%d@@@@@@@@",Serial.available(),Serial.peek());
    Serial1.println("throw!");
    Serial1.readBytes(tmp,1);
    return ;
  }
  len = lraw[0] + (lraw[1] << 8);
  // Serial1.printf("Get len:%d\n",len);
  if ( !check_parity(len) ) // 3bit parity check
    return ;
  // Serial1.printf("Check passed\n");
  len = len & ((1<<9)-1);
  if (len - (len/8)*8 != 2 && len - (len/8)*8 != 3) // package len check
    return ;
  digitalWrite(5,LOW);
  // Serial1.print(digitalRead(4));
  // Serial1.printf("handshake pull down\n");
  // ss = micros64();
  while (digitalRead(4)==LOW)
    yield();
  // Serial1.printf("GPIO:%llu\n",micros64()-ss);
  // Serial1.printf("notify got\n");
  // ss = micros64();
  SPI.transfer(spi_buffer,len);
  // Serial1.printf("SPI:%llu\n",micros64()-ss);
  // Serial1.printf("spi done\n");
  // Serial1.printf("handshake pull up\n");
  // ss = micros64();
  // for (int i=0;i<20;i++)
  // {
  Send(spi_buffer,len);
    // yield();
  // }
  // Serial1.printf("SND:%llu\n",micros64()-ss);
  // Serial1.printf("%lu\n",millis());
  // spi_buffer[len]='\0';
  // Serial1.print((char *)spi_buffer);
  digitalWrite(5,HIGH);
  // Serial1.printf("SUM:%llu\n",micros64()-sst);
  // sst = micros64();
}
