// [ru] AlashMHZ19: датчик молчит или врёт? Скетч трижды запрашивает концентрацию, считает ответы и ошибки суммы и подсказывает, что проверить.
//      Язык подсказок: LANG_RU, LANG_KK или LANG_EN. Подключение как в примере Basic.
// [kk] AlashMHZ19: датчик үнсіз бе немесе өтірік айта ма? Скетч концентрацияны үш рет сұрайды, жауаптар мен сома қателерін санайды және нені тексеру керегін айтады.
//      Кеңес тілі: LANG_RU, LANG_KK немесе LANG_EN. Қосу Basic мысалындағыдай.
// [en] AlashMHZ19: the sensor is silent or lies? The sketch requests the concentration three times, counts replies and checksum errors and tells what to check.
//      Tips language: LANG_RU, LANG_KK or LANG_EN. Wiring as in the Basic example.
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
  co2sensor.setLanguage(AlashMHZ19::LANG_RU);
  co2sensor.diagnose(Serial);
}

void loop() {}
