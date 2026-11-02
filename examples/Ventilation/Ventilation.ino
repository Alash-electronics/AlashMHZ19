// [ru] AlashMHZ19: напоминание проветрить. poll() сам запрашивает датчик раз в 2 секунды и не блокирует программу. Когда CO2 выше 1000 ppm,
//      зуммер на пине 8 пищит трижды раз в минуту, светодиод на пине 13 горит. Подключение датчика как в примере Basic.
// [kk] AlashMHZ19: желдетуді еске салу. poll() датчикті 2 секунд сайын өзі сұрайды және бағдарламаны блоктамайды. CO2 1000 ppm-нен жоғары болғанда
//      8 пиндегі зуммер минутына бір рет үш дыбыс береді, 13 пиндегі жарықдиод жанады. Датчикті қосу Basic мысалындағыдай.
// [en] AlashMHZ19: a ventilation reminder. poll() requests the sensor every 2 seconds by itself and does not block the program. When CO2 is above 1000 ppm
//      the buzzer on pin 8 beeps three times a minute and the LED on pin 13 lights. The sensor wiring is as in the Basic example.
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
const uint8_t BUZZER = 8, LED = 13;
const uint16_t LIMIT = 1000;
uint32_t lastBeep = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);
#if defined(ARDUINO_ARCH_ESP32)
  Serial2.begin(9600);
#else
  sensorPort.begin(9600);                        // датчик всегда работает на 9600
#endif
  co2sensor.begin();
  co2sensor.setLanguage(AlashMHZ19::LANG_RU);    // LANG_KK или LANG_EN
}

void loop() {
  if (co2sensor.poll(2000)) {                    // true, когда пришло новое значение
    Serial.print(co2sensor.co2()); Serial.print(F(" ppm  ")); Serial.println(co2sensor.qualityText());
  }
  bool stuffy = co2sensor.hasData() && co2sensor.warmedUp() && co2sensor.co2() > LIMIT;   // во время прогрева не пищим
  digitalWrite(LED, stuffy ? HIGH : LOW);
  if (stuffy && millis() - lastBeep > 60000UL) {
    lastBeep = millis();
    for (uint8_t i = 0; i < 3; i++) { tone(BUZZER, 2000, 150); delay(300); }
  }
}
