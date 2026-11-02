// [ru] AlashMHZ19: калибровка нуля и автокалибровка. Нужна, если датчик со временем «уплыл» (в свежем воздухе показывает 500-600 вместо 420).
//      1) Вынесите датчик на улицу (или к открытому окну, подальше от дыхания) минимум на 20 минут.
//      2) Отправьте в монитор порта (115200) букву z: текущий воздух запишется как 400 ppm. ВНИМАНИЕ: в помещении так делать нельзя.
//      Буква a включает автокалибровку ABC (раз в сутки самый чистый воздух считается 400 ppm), буква n выключает: выключайте, если комната не проветривается ночью.
// [kk] AlashMHZ19: нөлді калибрлеу және автокалибрлеу. Датчик уақыт өте «ауытқыса» керек (таза ауада 420 орнына 500-600 көрсетеді).
//      1) Датчикті далаға (немесе ашық терезе жанына, тыныстан алыс) кемінде 20 минутқа шығарыңыз.
//      2) Порт мониторына (115200) z әрпін жіберіңіз: ағымдағы ауа 400 ppm деп жазылады. НАЗАР: бөлмеде бұлай істеуге болмайды.
//      a әрпі ABC автокалибрлеуін қосады (тәулігіне бір рет ең таза ауа 400 ppm деп саналады), n өшіреді: бөлме түнде желдетілмесе өшіріңіз.
// [en] AlashMHZ19: zero calibration and auto-calibration. Needed if the sensor has drifted over time (in fresh air it shows 500-600 instead of 420).
//      1) Take the sensor outdoors (or to an open window, away from your breath) for at least 20 minutes.
//      2) Send the letter z to the serial monitor (115200): the current air is stored as 400 ppm. WARNING: never do this indoors.
//      The letter a switches the ABC auto-calibration on (once a day the cleanest air is taken as 400 ppm), n switches it off: switch it off if the room is not ventilated at night.
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
  Serial.println(F("z = calibrate zero (OUTDOORS only), a = ABC on, n = ABC off"));
}

void loop() {
  if (co2sensor.read()) { Serial.print(co2sensor.co2()); Serial.println(F(" ppm")); }
  else Serial.println(co2sensor.errorText());
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'z') { co2sensor.calibrateZero(); Serial.println(F("Zero calibration sent")); }
    if (c == 'a') { co2sensor.setAutoCalibration(true); Serial.println(F("ABC on")); }
    if (c == 'n') { co2sensor.setAutoCalibration(false); Serial.println(F("ABC off")); }
  }
  delay(2000);
}
