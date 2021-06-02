# ProjectESP

ESP8266固件+与其通讯的STM32程序

ESP_SPI为STM32程序
ArduinoESP为ESP8266固件

# 通讯
本固件需要一个SPI接口（3/4线），2个GPIO口，以及一个UART串口，还需要连接ESP8266 RST针脚到STM32  

SPI：  
单向传输，STM32作为从机发送数据，ESP8266作为主机接收数据  
用于STM32到ESP8266的数据传输，也就是网络包裹的发送  
基本格式：  
|长度(2byte)|数据(<512byte)|
|----------|------------|  


UART：  
STM32->ESP8266：传输需要发送的包裹的长度信息  
ESP8266->STM32：发送WiFi收到的网络包裹  

GPIO4：用于从机通知主机已准备好发送  
GPIO5：用于主机通知从机已通过串口收到长度信息  

# 进度
- [x] 调试SPI，当前ESP8266收到的均为0xFF  
- [x] 完成ESP8266->STM32的包裹发送  
