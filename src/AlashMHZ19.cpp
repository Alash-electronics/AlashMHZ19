// MIT License. Alash Electronics.
#include "AlashMHZ19.h"

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))

// Команда и ответ: 9 байт. Контрольная сумма: 0xFF минус сумма байт 1..7, плюс 1.
uint8_t AlashMHZ19::checksum(const uint8_t *f) {
  uint8_t s = 0;
  for (uint8_t i = 1; i < 8; i++) s += f[i];
  return (uint8_t)(0xFF - s + 1);
}

void AlashMHZ19::begin() {
  _i = 0; _have = false; _err = ERR_OK; _ok = _bad = _bytes = 0; _polled = false;
  _startAt = millis();
  while (_s->available()) _s->read();
}

void AlashMHZ19::send(const uint8_t *cmd) { _s->write(cmd, 9); }

void AlashMHZ19::sendCommand(uint8_t cmd, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7) {
  uint8_t c[9] = {0xFF, 0x01, cmd, b3, b4, b5, b6, b7, 0};
  c[8] = checksum(c);
  send(c);
}

void AlashMHZ19::request() { sendCommand(0x86, 0, 0, 0, 0, 0); }                         // FF 01 86 00 00 00 00 00 79
void AlashMHZ19::calibrateZero() { sendCommand(0x87, 0, 0, 0, 0, 0); }                   // FF 01 87 00 00 00 00 00 78
void AlashMHZ19::setAutoCalibration(bool on) { sendCommand(0x79, on ? 0xA0 : 0x00, 0, 0, 0, 0); }   // E6 / 86

bool AlashMHZ19::setRange(uint16_t ppm) {
  if (ppm != 2000 && ppm != 5000 && ppm != 10000) { _err = ERR_BAD_VALUE; return false; }
  sendCommand(0x99, 0, 0, 0, (uint8_t)(ppm >> 8), (uint8_t)ppm);                         // 2000 -> 07 D0, 5000 -> 13 88, 10000 -> 27 10
  return true;
}

// Ответ: FF 86, CO2 (старший, младший), температура + 40, статус, ещё два байта, контрольная сумма.
bool AlashMHZ19::update() {
  bool got = false;
  while (_s->available()) {
    int c = _s->read();
    if (c < 0) break;
    _bytes++;
    uint8_t b = (uint8_t)c;
    if (_i == 0) { if (b == 0xFF) _buf[_i++] = b; continue; }
    if (_i == 1) { if (b == 0x86) _buf[_i++] = b; else _i = (b == 0xFF) ? 1 : 0; continue; }   // ответы на другие команды не нужны
    _buf[_i++] = b;
    if (_i == 9) {
      if (checksum(_buf) == _buf[8]) {
        _ppm = ((uint16_t)_buf[2] << 8) | _buf[3];
        _temp = (int16_t)_buf[4] - 40;
        _status = _buf[5];
        _ok++; _err = ERR_OK; _have = true; _at = millis(); got = true;
      } else { _bad++; _err = ERR_CHECKSUM; }
      _i = 0;
    }
  }
  return got;
}

bool AlashMHZ19::read(uint32_t timeoutMs) {
  while (_s->available()) _s->read();
  _i = 0;
  request();
  uint32_t t0 = millis(), before = _bytes;
  while ((uint32_t)(millis() - t0) < timeoutMs) {
    if (update()) return true;
    delay(5);
  }
  if (_bytes == before) _err = ERR_NO_DATA;
  else if (_err != ERR_CHECKSUM) _err = ERR_TIMEOUT;
  return false;
}

bool AlashMHZ19::poll(uint32_t everyMs) {
  uint32_t now = millis();
  if (!_polled || (uint32_t)(now - _lastReq) >= everyMs) { request(); _lastReq = now; _polled = true; }
  return update();
}

AlashMHZ19::Quality AlashMHZ19::quality() const {
  if (_ppm <= 800) return Q_GOOD;
  if (_ppm <= 1000) return Q_OK;
  if (_ppm <= 1500) return Q_STUFFY;
  return Q_BAD;
}

const __FlashStringHelper *AlashMHZ19::qualityText() {
  switch (quality()) {
    case Q_GOOD: return T("хорошо", "жақсы", "good");
    case Q_OK: return T("нормально", "қалыпты", "acceptable");
    case Q_STUFFY: return T("душно, проветрите", "тыныс тарылады, желдетіңіз", "stuffy, ventilate");
    case Q_BAD: return T("плохо, срочно проветрите", "нашар, тез желдетіңіз", "bad, ventilate now");
  }
  return F("");
}

const __FlashStringHelper *AlashMHZ19::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_NO_DATA:
      return T("Датчик молчит. Проверьте питание 5 В (Vin), провод TX датчика к RX платы и RX датчика к TX платы (крест-накрест), общий GND и скорость 9600. Логика датчика 3.3 В, ESP подключается напрямую.",
               "Датчик үнсіз. 5 В қуатын (Vin), датчик TX сымын тақта RX-ке және датчик RX-ін тақта TX-ке (айқастырып), ортақ GND және 9600 жылдамдығын тексеріңіз. Датчик логикасы 3.3 В, ESP тікелей қосылады.",
               "The sensor is silent. Check the 5 V power (Vin), the sensor TX wire to the board RX and the sensor RX to the board TX (crossed), a common GND and the speed 9600. The sensor logic is 3.3 V, an ESP connects directly.");
    case ERR_NO_FRAME:
      return T("Байты приходят, но ответа датчика (FF 86) нет. Скорость порта должна быть 9600, к порту должен быть подключён датчик MH-Z19.",
               "Байттар келеді, бірақ датчик жауабы (FF 86) жоқ. Порт жылдамдығы 9600 болуы керек, портқа MH-Z19 датчигі қосылуы керек.",
               "Bytes arrive but there is no sensor reply (FF 86). The port speed must be 9600 and an MH-Z19 sensor must be connected to the port.");
    case ERR_CHECKSUM:
      return T("Ответ пришёл, но контрольная сумма не сошлась: помехи. Укоротите провода, добавьте общий GND, проверьте питание (датчику нужно до 150 мА при прогреве).",
               "Жауап келді, бірақ бақылау сомасы сәйкес келмеді: кедергі. Сымдарды қысқартыңыз, ортақ GND қосыңыз, қуатты тексеріңіз (қызу кезінде датчикке 150 мА-ге дейін керек).",
               "A reply arrived but the checksum did not match: noise. Shorten the wires, add a common GND, check the power (the sensor draws up to 150 mA while heating).");
    case ERR_TIMEOUT:
      return T("Ответ не пришёл за отведённое время. Датчик может быть ещё в прогреве (до 3 минут) или порт читается не тем кодом.",
               "Белгіленген уақытта жауап келмеді. Датчик әлі қызып жатқан болуы мүмкін (3 минутқа дейін) немесе порт басқа кодпен оқылады.",
               "No reply arrived in time. The sensor may still be warming up (up to 3 minutes) or the port is read by other code.");
    case ERR_BAD_VALUE:
      return T("Диапазон должен быть 2000, 5000 или 10000 ppm.", "Диапазон 2000, 5000 немесе 10000 ppm болуы керек.", "The range must be 2000, 5000 or 10000 ppm.");
  }
  return F("");
}

void AlashMHZ19::diagnose(Print &out) {
  out.println(T("AlashMHZ19: диагностика", "AlashMHZ19: диагностика", "AlashMHZ19: diagnostics"));
  out.println(T("Запрашиваю концентрацию 3 раза...", "Концентрацияны 3 рет сұраймын...", "Requesting the concentration 3 times..."));
  uint8_t good = 0; uint32_t b0 = _bytes, bad0 = _bad;
  for (uint8_t k = 0; k < 3; k++) { if (read(1500)) good++; delay(500); }
  out.print(T("Ответов получено: ", "Алынған жауаптар: ", "Replies received: ")); out.print((long)good);
  out.print(T("   байт: ", "   байт: ", "   bytes: ")); out.print((long)(_bytes - b0));
  out.print(T("   с ошибкой суммы: ", "   қате сомалы: ", "   with a bad sum: ")); out.println((long)(_bad - bad0));
  if (_bytes == b0) _err = ERR_NO_DATA;
  else if (good == 0 && _bad == bad0) _err = ERR_NO_FRAME;
  else if (good == 0) _err = ERR_CHECKSUM;
  if (good > 0) _err = ERR_OK;
  if (_err != ERR_OK) out.println(errorText());
  if (_have) {
    out.print(F("CO2=")); out.print((long)_ppm); out.print(F(" ppm  ")); out.print(qualityText());
    out.print(F("  T=")); out.print((long)_temp); out.println(F(" C"));
    if (!warmedUp()) out.println(T("Датчик ещё прогревается: до 3 минут после включения значения занижены или равны 400.", "Датчик әлі қызып жатыр: қосқаннан кейін 3 минутқа дейін мәндер төмендетілген немесе 400-ге тең.", "The sensor is still warming up: for up to 3 minutes after switch-on the values are low or equal to 400."));
  }
  out.println(T("Подсказки: если показывает 400 и не меняется, датчик не прогрелся или его выдох не доходит (подуйте на него: должно вырасти до 1000-2000). Не держите датчик у окна на солнце или рядом с обогревателем. Автокалибровка ABC считает самый чистый воздух за сутки 400 ppm: в помещении, которое никогда не проветривается, она занижает показания, тогда отключите её (setAutoCalibration(false)). Калибровку нуля делайте только на улице после 20 минут.",
                "Кеңестер: 400 көрсетіп, өзгермесе, датчик қызбаған немесе оған демі жетпейді (үрлеңіз: 1000-2000-ға дейін өсуі керек). Датчикті күн түсетін терезе жанында немесе жылытқыш қасында ұстамаңыз. ABC автокалибрлеуі тәулік ішіндегі ең таза ауаны 400 ppm деп санайды: ешқашан желдетілмейтін бөлмеде ол көрсеткіштерді төмендетеді, сонда оны өшіріңіз (setAutoCalibration(false)). Нөлді калибрлеуді тек далада 20 минуттан кейін жасаңыз.",
                "Tips: if it shows 400 and does not change, the sensor has not warmed up or the air does not reach it (breathe on it: it should rise to 1000-2000). Do not keep the sensor by a sunny window or a heater. The ABC auto-calibration takes the cleanest air of the day as 400 ppm: in a room that is never ventilated it lowers the readings, then switch it off (setAutoCalibration(false)). Do the zero calibration only outdoors after 20 minutes."));
}
