// AlashMHZ19: датчики углекислого газа CO2 серии MH-Z19 (B, C, D, E) и MH-Z14A по UART: ppm, температура, оценка проветренности,
// калибровка, автокалибровка ABC, диапазон. Приём без блокировки, проверка контрольной суммы, любой порт (Serial, Serial2, SoftwareSerial).
// Без зависимостей. MIT License. Alash Electronics, https://github.com/Alash-electronics/AlashMHZ19
#ifndef ALASH_MHZ19_H
#define ALASH_MHZ19_H

#include <Arduino.h>

class AlashMHZ19 {
 public:
  enum Error : uint8_t {
    ERR_OK = 0,
    ERR_NO_DATA,        // по порту не пришло ни одного байта
    ERR_NO_FRAME,       // байты идут, но ответа датчика (FF 86) нет: не та скорость или чужое устройство
    ERR_CHECKSUM,       // ответ пришёл, но сумма не сошлась: помехи
    ERR_TIMEOUT,        // ответа не дождались за отведённое время
    ERR_BAD_VALUE       // параметр вне допустимого (диапазон не 2000 / 5000 / 10000)
  };
  enum Quality : uint8_t { Q_GOOD = 0, Q_OK, Q_STUFFY, Q_BAD };   // до 800, до 1000, до 1500, выше (ppm)
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };

  explicit AlashMHZ19(Stream &port) : _s(&port) {}

  void begin();                                // сбросить состояние (порт откройте сами: Serial2.begin(9600))
  void request();                              // послать запрос концентрации
  bool update();                               // в loop(): разбирает байты; true, если пришёл новый ответ
  bool read(uint32_t timeoutMs = 1500);        // запросить и дождаться ответа (блокирует)
  bool poll(uint32_t everyMs = 2000);          // в loop(): сам запрашивает раз в everyMs и разбирает ответы; true, когда есть новое значение

  uint16_t co2() const { return _ppm; }        // ppm
  int16_t temperature() const { return _temp; }   // °C, грубая оценка датчика (±2..3 градуса), для компенсации не годится
  uint8_t statusByte() const { return _status; }
  bool hasData() const { return _have; }
  uint32_t ageMs() const { return _have ? (uint32_t)(millis() - _at) : 0xFFFFFFFFUL; }
  Quality quality() const;                     // оценка проветренности по ppm
  const __FlashStringHelper *qualityText();    // «хорошо», «душновато», «проветрите»... на выбранном языке

  // ---- Настройка датчика (команды записываются во флеш датчика) ---------------------------------
  void calibrateZero();                        // отметить текущий воздух как 400 ppm: ТОЛЬКО после 20 минут на свежем воздухе (на улице)
  void setAutoCalibration(bool on);            // ABC: раз в 24 часа датчик считает самый чистый воздух за 400 ppm; выключите, если помещение не проветривается ночью
  bool setRange(uint16_t ppm);                 // 2000, 5000 или 10000 (не у всех версий)
  bool warmedUp() const { return (uint32_t)(millis() - _startAt) >= _warmupMs; }
  void setWarmupMs(uint32_t ms) { _warmupMs = ms; }   // по умолчанию 180 с (MH-Z19B); у MH-Z19C / D / E достаточно 60 с

  uint32_t framesOk() const { return _ok; }
  uint32_t framesBad() const { return _bad; }
  uint32_t bytesSeen() const { return _bytes; }
  Error error() const { return _err; }
  void clearError() { _err = ERR_OK; }
  void setLanguage(Language lang) { _lang = lang; }
  const __FlashStringHelper *errorText();
  void diagnose(Print &out);

  static uint8_t checksum(const uint8_t *frame9);   // 0xFF - сумма байт 1..7 + 1

 private:
  void send(const uint8_t *cmd9);
  void sendCommand(uint8_t cmd, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7);
  Stream *_s;
  uint8_t _buf[9]; uint8_t _i = 0;
  uint16_t _ppm = 0; int16_t _temp = 0; uint8_t _status = 0;
  bool _have = false;
  uint32_t _at = 0, _startAt = 0, _warmupMs = 180000UL, _lastReq = 0, _ok = 0, _bad = 0, _bytes = 0;
  bool _polled = false;
  Error _err = ERR_OK; Language _lang = LANG_RU;
};

#endif
