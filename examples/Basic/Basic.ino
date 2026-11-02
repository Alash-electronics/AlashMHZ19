// [ru] AlashMHZ19: самый простой пример. Концентрация углекислого газа CO2 в ppm раз в 2 секунды с датчика MH-Z19B / MH-Z19C / MH-Z19E.
//
//      Подключение: Vin датчика к 5 В, GND к GND, TX датчика к RX платы (Uno: пин 10, ESP32: GPIO16, ESP8266: D5), RX датчика к TX платы (Uno: пин 11, ESP32: GPIO17, ESP8266: D6).
//      Первые 1-3 минуты после включения датчик прогревается: значения заниженные. Свежий воздух около 420 ppm, в закрытой комнате с людьми 1000-2500.
// [kk] AlashMHZ19: ең қарапайым мысал. MH-Z19B / MH-Z19C / MH-Z19E датчигінен көмірқышқыл газы CO2 концентрациясы ppm-мен 2 секунд сайын.
//
//      Қосу: датчик Vin 5 В-қа, GND GND-ға, датчик TX тақта RX-ке (Uno: 10 пин, ESP32: GPIO16, ESP8266: D5), датчик RX тақта TX-ке (Uno: 11 пин, ESP32: GPIO17, ESP8266: D6).
//      Қосқаннан кейінгі алғашқы 1-3 минутта датчик қызады: мәндер төмендетілген. Таза ауа шамамен 420 ppm, адамдар бар жабық бөлмеде 1000-2500.
// [en] AlashMHZ19: the simplest example. The carbon dioxide CO2 concentration in ppm every 2 seconds from an MH-Z19B / MH-Z19C / MH-Z19E sensor.
//
//      Wiring: sensor Vin to 5 V, GND to GND, sensor TX to the board RX (Uno: pin 10, ESP32: GPIO16, ESP8266: D5), sensor RX to the board TX (Uno: pin 11, ESP32: GPIO17, ESP8266: D6).
//      For the first 1-3 minutes after switch-on the sensor warms up: the values are low. Fresh air is about 420 ppm, a closed room with people 1000-2500.
#include <AlashMHZ19.h>

#if defined(ARDUINO_ARCH_ESP32)
HardwareSerial &sensorPort = Serial2;            // ESP32: RX2 = GPIO16, TX2 = GPIO17
#elif defined(ARDUINO_ARCH_ESP8266)
#include <SoftwareSerial.h>
SoftwareSerial sensorPort(14, 12);               // ESP8266: RX = GPIO14 (D5), TX = GPIO12 (D6); пины 9-11 заняты флеш-памятью
#else
#include <SoftwareSerial.h>
SoftwareSerial sensorPort(10, 11);               // Uno / Nano / Mega: RX = пин 10, TX = пин 11
#endif
AlashMHZ19 co2sensor(sensorPort);

void setup() {
  Serial.begin(115200);
#if defined(ARDUINO_ARCH_ESP32)
  Serial2.begin(9600);
#else
  sensorPort.begin(9600);                        // датчик всегда работает на 9600
#endif
  co2sensor.begin();
}

void loop() {
  if (co2sensor.read()) {                        // запрос и ожидание ответа до 1.5 с
    Serial.print(F("CO2: ")); Serial.print(co2sensor.co2());
    Serial.print(F(" ppm   ")); Serial.println(co2sensor.qualityText());
  } else {
    Serial.println(co2sensor.errorText());       // что проверить
  }
  delay(2000);
}
