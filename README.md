# AlashMHZ19

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **датчиков углекислого газа CO2 MH-Z19B / MH-Z19C / MH-Z19E** (UART). Концентрация в ppm и оценка словами («душно, проветрите»), калибровка, автокалибровка ABC. Приём без блокировки, проверка контрольной суммы. Работает с любым портом. Сделана для уроков экологии и наборов Alash Electronics.

```cpp
#include <AlashMHZ19.h>
#include <SoftwareSerial.h>
SoftwareSerial port(10, 11);          // RX, TX
AlashMHZ19 co2(port);

void setup() { Serial.begin(115200); port.begin(9600); co2.begin(); }
void loop() {
  if (co2.read()) Serial.println(co2.co2());   // ppm
  delay(2000);
}
```

### Что умеет
- **`co2()` в ppm:** `read()` просит и ждёт ответ, `poll()` и `update()` работают без блокировки.
- **Оценка проветренности:** `qualityText()` («хорошо», «нормально», «душно, проветрите», «плохо») на трёх языках, `quality()` для своей логики.
- **Проверка данных:** контрольная сумма, поиск начала ответа после мусора, счётчики исправных и испорченных ответов.
- **Калибровка:** `calibrateZero()` (только на улице после 20 минут), `setAutoCalibration()` включает и выключает ABC, `setRange()` задаёт диапазон.
- **Прогрев:** `warmedUp()` скажет, прошло ли время (по умолчанию 180 секунд, `setWarmupMs()`).
- **Любой порт:** `Serial2`, `SoftwareSerial`, `Serial1`.
- **Видно, что происходит:** `diagnose(Serial)` запрашивает датчик и подсказывает (питание, TX / RX, скорость, ABC) на трёх языках.
- Работает на **Arduino AVR, ESP32, ESP8266**.

### Подключение
| MH-Z19 | Uno / Nano | ESP32 |
|---|---|---|
| Vin | 5 В | 5 В (VIN) |
| GND | GND | GND |
| TX датчика | пин 10 (RX) | GPIO16 (RX2) |
| RX датчика | пин 11 (TX) | GPIO17 (TX2) |

Датчику нужно 5 В и до 150 мА при прогреве, логика у него 3.3 В, поэтому к ESP32 и ESP8266 он подключается напрямую. TX датчика на RX платы (крест-накрест). Скорость порта всегда 9600. Первые 1-3 минуты после включения значения занижены.

### Чем отличается от других библиотек
Мы изучили MH-Z-CO2-Sensors (Tobias Schürg, MIT), MH-Z19 (WifWaf, LGPL-3.0) и MHZ19 (strange-v, GPL-3.0):
- WifWaf и strange-v под LGPL / GPL, форкнуть в MIT нельзя; протокол (команды, контрольная сумма) сверен по описанию датчика и по открытой MIT-библиотеке, код написан заново.
- Чужие библиотеки возвращают отрицательные коды (-2, -3, -5) вместо причины; у нас `errorText()` на трёх языках.
- Блокирующее чтение с задержкой до 10 секунд; у нас `poll()` и `update()` без блокировки.
- Нет подсказок про ABC, калибровку на улице и прогрев; у нас `diagnose()` и `warmedUp()`.

### Примеры
`Basic`, `Ventilation`, `Calibration`, `Diagnose`

### Тесты
Без железа (с AddressSanitizer) на имитации порта: контрольная сумма и все команды побайтно по описанию Winsen (запрос, калибровка нуля, ABC вкл и выкл, диапазоны 2000, 5000, 10000), ответ с мусором и «ложным началом», кадр по частям, испорченная сумма, ответы на другие команды, блокирующее чтение, `poll()`, оценка проветренности на границах, прогрев, ошибки и диагностика: `extras/test`.

Подробное руководство: [docs/WIKI.md](docs/WIKI.md).

---

## Қазақша

**MH-Z19B / MH-Z19C / MH-Z19E көмірқышқыл газы CO2 датчиктеріне** (UART) арналған қарапайым кітапхана. ppm-мен концентрация және сөзбен баға («тыныс тарылады, желдетіңіз»), калибрлеу, ABC автокалибрлеуі. Блоктамай қабылдау, бақылау сомасын тексеру. Кез келген портпен жұмыс істейді.

```cpp
#include <AlashMHZ19.h>
#include <SoftwareSerial.h>
SoftwareSerial port(10, 11);          // RX, TX
AlashMHZ19 co2(port);

void setup() { Serial.begin(115200); port.begin(9600); co2.begin(); }
void loop() {
  if (co2.read()) Serial.println(co2.co2());   // ppm
  delay(2000);
}
```

### Мүмкіндіктері
- **`co2()` ppm-мен:** `read()` сұрап, жауапты күтеді, `poll()` және `update()` блоктамай жұмыс істейді.
- **Желдетілу бағасы:** үш тілде `qualityText()` («жақсы», «қалыпты», «тыныс тарылады, желдетіңіз», «нашар»), өз логикаңыз үшін `quality()`.
- **Деректерді тексеру:** бақылау сомасы, қоқыстан кейін жауап басын іздеу, дұрыс және бүлінген жауаптар санауыштары.
- **Калибрлеу:** `calibrateZero()` (тек далада 20 минуттан кейін), `setAutoCalibration()` ABC-ды қосады және өшіреді, `setRange()` диапазонды береді.
- **Қызу:** `warmedUp()` уақыттың өткенін айтады (әдепкі 180 секунд, `setWarmupMs()`).
- **Кез келген порт:** `Serial2`, `SoftwareSerial`, `Serial1`.
- **Не болып жатқаны көрінеді:** `diagnose(Serial)` датчикті сұрап, үш тілде кеңес береді (қуат, TX / RX, жылдамдық, ABC).
- **Arduino AVR, ESP32, ESP8266** тақталарында жұмыс істейді.

### Қосу
| MH-Z19 | Uno / Nano | ESP32 |
|---|---|---|
| Vin | 5 В | 5 В (VIN) |
| GND | GND | GND |
| TX датчика | пин 10 (RX) | GPIO16 (RX2) |
| RX датчика | пин 11 (TX) | GPIO17 (TX2) |

Датчикке 5 В және қызу кезінде 150 мА-ге дейін керек, логикасы 3.3 В, сондықтан ESP32 және ESP8266-ға тікелей қосылады. Датчик TX тақта RX-ке (айқастырып). Порт жылдамдығы әрқашан 9600. Қосқаннан кейінгі алғашқы 1-3 минутта мәндер төмендетілген.

### Басқа кітапханалардан айырмашылығы
MH-Z-CO2-Sensors (Tobias Schürg, MIT), MH-Z19 (WifWaf, LGPL-3.0) және MHZ19 (strange-v, GPL-3.0) қарадық:
- WifWaf және strange-v LGPL / GPL астында, MIT-ке форк жасау мүмкін емес; протокол (командалар, бақылау сомасы) датчик сипаттамасы және ашық MIT кітапханасы бойынша салыстырылды, код қайта жазылды.
- Басқа кітапханалар себептің орнына теріс кодтар (-2, -3, -5) қайтарады; бізде үш тілде `errorText()`.
- 10 секундқа дейін кідірісі бар блоктайтын оқу; бізде блоктамайтын `poll()` және `update()`.
- ABC, далада калибрлеу және қызу туралы кеңестер жоқ; бізде `diagnose()` және `warmedUp()`.

### Мысалдар
`Basic`, `Ventilation`, `Calibration`, `Diagnose`

### Тесттер
Құрылғысыз (AddressSanitizer-мен) порт имитациясында: бақылау сомасы және Winsen сипаттамасы бойынша барлық командалар байтпа-байт (сұраныс, нөлді калибрлеу, ABC қосу және өшіру, 2000, 5000, 10000 диапазондары), қоқысы мен «жалған басы» бар жауап, бөліктеп келген кадр, бүлінген сома, басқа командаларға жауаптар, блоктайтын оқу, `poll()`, шекараларда желдетілу бағасы, қызу, қателер: `extras/test`.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

---

## English

A simple library for the **MH-Z19B / MH-Z19C / MH-Z19E carbon dioxide CO2 sensors** (UART). The concentration in ppm and a verbal rating ("stuffy, ventilate"), calibration, ABC auto-calibration. Non-blocking reception, checksum verification. Works with any port. Made for ecology lessons and Alash Electronics kits.

```cpp
#include <AlashMHZ19.h>
#include <SoftwareSerial.h>
SoftwareSerial port(10, 11);          // RX, TX
AlashMHZ19 co2(port);

void setup() { Serial.begin(115200); port.begin(9600); co2.begin(); }
void loop() {
  if (co2.read()) Serial.println(co2.co2());   // ppm
  delay(2000);
}
```

### Features
- **`co2()` in ppm:** `read()` asks and waits for the reply, `poll()` and `update()` work without blocking.
- **Ventilation rating:** `qualityText()` ("good", "acceptable", "stuffy, ventilate", "bad") in three languages, `quality()` for your own logic.
- **Data checks:** the checksum, finding the reply start after garbage, counters of good and bad replies.
- **Calibration:** `calibrateZero()` (outdoors only after 20 minutes), `setAutoCalibration()` switches ABC on and off, `setRange()` sets the range.
- **Warm-up:** `warmedUp()` tells whether the time has passed (180 seconds by default, `setWarmupMs()`).
- **Any port:** `Serial2`, `SoftwareSerial`, `Serial1`.
- **See what happens:** `diagnose(Serial)` queries the sensor and gives tips (power, TX / RX, speed, ABC) in three languages.
- Works on **Arduino AVR, ESP32, ESP8266**.

### Wiring
| MH-Z19 | Uno / Nano | ESP32 |
|---|---|---|
| Vin | 5 В | 5 В (VIN) |
| GND | GND | GND |
| TX датчика | пин 10 (RX) | GPIO16 (RX2) |
| RX датчика | пин 11 (TX) | GPIO17 (TX2) |

The sensor needs 5 V and up to 150 mA while heating, with 3.3 V logic, so it connects directly to ESP32 and ESP8266. The sensor TX to the board RX (crossed). The port speed is always 9600. For the first 1-3 minutes after switch-on the values are low.

### How it differs from other libraries
We studied MH-Z-CO2-Sensors (Tobias Schürg, MIT), MH-Z19 (WifWaf, LGPL-3.0) and MHZ19 (strange-v, GPL-3.0):
- WifWaf and strange-v are LGPL / GPL and cannot be forked into MIT; the protocol (commands, checksum) was checked against the sensor description and an open MIT library, and the code is written anew.
- The other libraries return negative codes (-2, -3, -5) instead of a reason; ours has `errorText()` in three languages.
- A blocking read with a delay of up to 10 seconds; ours has non-blocking `poll()` and `update()`.
- No tips about ABC, calibrating outdoors and warm-up; ours has `diagnose()` and `warmedUp()`.

### Examples
`Basic`, `Ventilation`, `Calibration`, `Diagnose`

### Tests
Without hardware (with AddressSanitizer) on a simulated port: the checksum and all commands byte by byte per the Winsen description (request, zero calibration, ABC on and off, ranges 2000, 5000, 10000), a reply with garbage and a "false start", a frame in parts, a bad checksum, replies to other commands, a blocking read, `poll()`, the ventilation rating at the boundaries, warm-up, errors and diagnostics: `extras/test`.

Full guide: [docs/WIKI.md](docs/WIKI.md).

---

MIT License, Alash Electronics.
