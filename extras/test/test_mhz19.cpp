// Тесты AlashMHZ19 без железа: порт имитируется очередью байтов; команды сверяются побайтно с описанием Winsen MH-Z19.
//   g++ -std=c++17 -Wall -fsanitize=address,undefined -I mock -I ../../src test_mhz19.cpp ../../src/AlashMHZ19.cpp && ./a.out
#include <stdio.h>
#include "Arduino.h"
#include "AlashMHZ19.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)
typedef AlashMHZ19 M;
typedef std::vector<uint8_t> Bytes;

static Bytes reply(uint16_t ppm, int temp, uint8_t status = 0, bool good = true) {
  Bytes r = {0xFF, 0x86, (uint8_t)(ppm >> 8), (uint8_t)ppm, (uint8_t)(temp + 40), status, 0x00, 0x00, 0x00};
  uint8_t s = 0; for (int i = 1; i < 8; i++) s += r[i];
  r[8] = (uint8_t)(0xFF - s + 1);
  if (!good) r[8] ^= 0x33;
  return r;
}
static void feed(Stream &s, const Bytes &f) { for (uint8_t b : f) s.rx.push_back(b); }
static void restart() { g_ms += 1000000; g_onDelay = nullptr; }

int main() {
  // 1) контрольная сумма по документированным командам
  { const Bytes req = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    CHECK(M::checksum(req.data()) == 0x79);
    const Bytes cal = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78}; CHECK(M::checksum(cal.data()) == 0x78);
    const Bytes abcOn = {0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6}; CHECK(M::checksum(abcOn.data()) == 0xE6);
    const Bytes abcOff = {0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86}; CHECK(M::checksum(abcOff.data()) == 0x86);
    const Bytes r2k = {0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x07, 0xD0, 0x8F}; CHECK(M::checksum(r2k.data()) == 0x8F);
    const Bytes r5k = {0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x13, 0x88, 0xCB}; CHECK(M::checksum(r5k.data()) == 0xCB);
    const Bytes r10k = {0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x27, 0x10, 0x2F}; CHECK(M::checksum(r10k.data()) == 0x2F);
  }

  // 2) команды побайтно
  { restart(); Stream s; M m(s); m.begin();
    m.request(); CHECK(s.tx == Bytes({0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79})); s.tx.clear();
    m.calibrateZero(); CHECK(s.tx == Bytes({0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78})); s.tx.clear();
    m.setAutoCalibration(true); CHECK(s.tx == Bytes({0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6})); s.tx.clear();
    m.setAutoCalibration(false); CHECK(s.tx == Bytes({0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86})); s.tx.clear();
    CHECK(m.setRange(2000) && s.tx == Bytes({0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x07, 0xD0, 0x8F})); s.tx.clear();
    CHECK(m.setRange(5000) && s.tx == Bytes({0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x13, 0x88, 0xCB})); s.tx.clear();
    CHECK(m.setRange(10000) && s.tx == Bytes({0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x27, 0x10, 0x2F})); s.tx.clear();
    CHECK(!m.setRange(3000) && m.error() == M::ERR_BAD_VALUE && s.tx.empty());                 // плохой диапазон ничего не шлёт
  }

  // 3) ответ: ppm, температура, статус; мусор перед ответом; кадр по частям
  { restart(); Stream s; M m(s); m.begin();
    for (uint8_t b : {0x00, 0xFF, 0xFF, 0x12}) s.rx.push_back(b);                             // мусор и «ложное начало» FF
    auto r = reply(1234, 23, 0x40);
    for (int i = 0; i < 5; i++) s.rx.push_back(r[i]);
    CHECK(!m.update());
    for (size_t i = 5; i < r.size(); i++) s.rx.push_back(r[i]);
    CHECK(m.update() && m.hasData() && m.co2() == 1234 && m.temperature() == 23 && m.statusByte() == 0x40 && m.framesOk() == 1);
    feed(s, reply(65535, -5)); CHECK(m.update() && m.co2() == 65535 && m.temperature() == -5);   // верхняя и отрицательная границы
    feed(s, reply(0, 0)); CHECK(m.update() && m.co2() == 0);
  }

  // 4) испорченная сумма не портит данные; ответы на другие команды пропускаются
  { restart(); Stream s; M m(s); m.begin(); feed(s, reply(700, 20)); m.update();
    feed(s, reply(900, 20, 0, false));
    CHECK(!m.update() && m.error() == M::ERR_CHECKSUM && m.co2() == 700 && m.framesBad() == 1);
    for (uint8_t b : {0xFF, 0x79, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86}) s.rx.push_back(b);   // подтверждение другой команды
    feed(s, reply(710, 21)); CHECK(m.update() && m.co2() == 710 && m.error() == M::ERR_OK);
  }

  // 5) блокирующее чтение: запрос уходит, ответ принимается; тайм-ауты
  { restart(); Stream s; M m(s); m.begin(); int n = 0;
    s.rx.push_back(0x77);                                                                      // старые байты в буфере сбрасываются
    g_onDelay = [&]() { if (++n == 5) feed(s, reply(555, 24)); };
    CHECK(m.read(1500) && m.co2() == 555 && s.tx == Bytes({0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79}));
    Stream e; M me(e); me.begin(); g_onDelay = nullptr; uint32_t t = g_ms;
    CHECK(!me.read(800) && me.error() == M::ERR_NO_DATA && g_ms - t >= 800);
    Stream g; M mg(g); mg.begin(); g_onDelay = [&]() { g.rx.push_back(0x55); }; CHECK(!mg.read(300) && mg.error() == M::ERR_TIMEOUT);
  }

  // 6) poll(): сам запрашивает раз в заданный интервал
  { restart(); Stream s; M m(s); m.begin();
    CHECK(!m.poll(2000) && s.tx.size() == 9);                                                  // первый запрос сразу
    g_ms += 1000; m.poll(2000); CHECK(s.tx.size() == 9);
    g_ms += 1100; feed(s, reply(800, 22)); CHECK(m.poll(2000) && s.tx.size() == 18 && m.co2() == 800);
  }

  // 7) оценка проветренности, возраст данных, прогрев
  { restart(); Stream s; M m(s); m.begin();
    CHECK(m.ageMs() == 0xFFFFFFFFUL && !m.hasData());
    struct { uint16_t ppm; M::Quality q; } cases[] = {{400, M::Q_GOOD}, {800, M::Q_GOOD}, {801, M::Q_OK}, {1000, M::Q_OK}, {1001, M::Q_STUFFY}, {1500, M::Q_STUFFY}, {1501, M::Q_BAD}, {5000, M::Q_BAD}};
    for (auto &c : cases) { feed(s, reply(c.ppm, 20)); m.update(); CHECK(m.quality() == c.q); }
    m.setLanguage(M::LANG_RU); const char *ru = (const char *)m.qualityText(); m.setLanguage(M::LANG_KK); const char *kk = (const char *)m.qualityText(); m.setLanguage(M::LANG_EN); const char *en = (const char *)m.qualityText();
    CHECK(strcmp(ru, kk) && strcmp(kk, en) && strlen(ru) > 5);
    g_ms += 500; CHECK(m.ageMs() >= 500 && m.ageMs() < 510);
    Stream w; M mw(w); mw.begin(); CHECK(!mw.warmedUp()); g_ms += 179000; CHECK(!mw.warmedUp()); g_ms += 2000; CHECK(mw.warmedUp());
    mw.begin(); mw.setWarmupMs(60000); g_ms += 61000; CHECK(mw.warmedUp());
  }

  // 8) ошибки на трёх языках и диагностика
  { for (int e = M::ERR_NO_DATA; e <= M::ERR_BAD_VALUE; e++) {
      restart(); Stream s; M m(s); m.begin();
      if (e == M::ERR_NO_DATA) m.read(100);
      else if (e == M::ERR_NO_FRAME) { m.setLanguage(M::LANG_RU); continue; }                  // ставится только диагностикой
      else if (e == M::ERR_CHECKSUM) { feed(s, reply(1, 1, 0, false)); m.update(); }
      else if (e == M::ERR_TIMEOUT) { g_onDelay = [&]() { s.rx.push_back(0x55); }; m.read(100); g_onDelay = nullptr; }
      else m.setRange(1);
      CHECK(m.error() == e);
      m.setLanguage(M::LANG_RU); const char *ru = (const char *)m.errorText(); m.setLanguage(M::LANG_KK); const char *kk = (const char *)m.errorText(); m.setLanguage(M::LANG_EN); const char *en = (const char *)m.errorText();
      CHECK(strlen(ru) > 20 && strlen(kk) > 20 && strlen(en) > 20 && strcmp(ru, kk) && strcmp(kk, en) && strcmp(ru, en));
    }
    restart(); Stream s; M m(s); m.begin(); m.setLanguage(M::LANG_EN); int n = 0;
    g_onDelay = [&]() { if (++n % 20 == 0) feed(s, reply(1100, 25)); };
    Print out; m.diagnose(out); g_onDelay = nullptr;
    CHECK(out.text.find("Replies received: 3") != std::string::npos && out.text.find("CO2=1100 ppm  stuffy") != std::string::npos && out.text.find("warming up") != std::string::npos && out.text.find("ABC") != std::string::npos);
    restart(); Stream e; M me(e); me.begin(); me.setLanguage(M::LANG_KK); Print oe; me.diagnose(oe); CHECK(me.error() == M::ERR_NO_DATA && oe.text.find("Датчик үнсіз") != std::string::npos);
    restart(); Stream g; M mg(g); mg.begin(); mg.setLanguage(M::LANG_EN); g_onDelay = [&]() { g.rx.push_back(0x55); }; Print og; mg.diagnose(og); g_onDelay = nullptr;
    CHECK(mg.error() == M::ERR_NO_FRAME && og.text.find("no sensor reply") != std::string::npos);
    CHECK(strcmp((const char *)M(g).errorText(), "OK") == 0);
  }

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
