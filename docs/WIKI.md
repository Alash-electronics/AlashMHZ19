# AlashMHZ19: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **датчиков углекислого газа CO2 MH-Z19B / MH-Z19C / MH-Z19E** (UART). Концентрация в ppm и оценка словами («душно, проветрите»), калибровка, автокалибровка ABC. Приём без блокировки, проверка контрольной суммы. Работает с любым портом. Сделана для уроков экологии и наборов Alash Electronics.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashMHZ19** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

### Подключение
| MH-Z19 | Uno / Nano | ESP32 |
|---|---|---|
| Vin | 5 В | 5 В (VIN) |
| GND | GND | GND |
| TX датчика | пин 10 (RX) | GPIO16 (RX2) |
| RX датчика | пин 11 (TX) | GPIO17 (TX2) |

Датчику нужно 5 В и до 150 мА при прогреве, логика у него 3.3 В, поэтому к ESP32 и ESP8266 он подключается напрямую. TX датчика на RX платы (крест-накрест). Скорость порта всегда 9600. Первые 1-3 минуты после включения значения занижены.

### Быстрый старт
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

### Справочник по методам
| Метод | Что делает |
|---|---|
| `AlashMHZ19(port)` | Датчик на порту Serial2 / SoftwareSerial / Serial1 |
| `begin()` | Сбросить состояние; порт откройте сами на 9600 |
| `read(timeoutMs)` | Запросить и дождаться ответа (блокирует) |
| `poll(everyMs)` | В loop(): сам запрашивает и разбирает; true при новом значении |
| `co2() / temperature()` | CO2 в ppm / температура датчика (грубо) |
| `quality() / qualityText()` | Оценка проветренности |
| `calibrateZero()` | Калибровка нуля (только на улице) |
| `setAutoCalibration(on)` | Включить или выключить ABC |
| `setRange(ppm)` | Диапазон 2000, 5000 или 10000 |
| `warmedUp() / setWarmupMs(ms)` | Прошёл ли прогрев |
| `error() / errorText() / diagnose(Serial)` | Ошибка, сообщение, самопроверка на ru/kk/en |

### Если что-то не работает
**Всегда показывает 400 или 410 ppm**

Датчик ещё не прогрелся (до 3 минут) либо до него не доходит воздух. Подуйте на него: значение должно вырасти до 1000-2000 ppm. Если нет, проверьте питание 5 В и запустите пример `Diagnose`.

**В свежем воздухе показывает 600-800 ppm**

Датчик «уплыл». Вынесите его на улицу минимум на 20 минут и выполните `calibrateZero()` (пример `Calibration`). В помещении калибровку нуля делать нельзя: датчик запомнит комнатный воздух как 400 ppm.

**Показания в комнате занижены и «плывут»**

Включена автокалибровка ABC: раз в сутки она принимает самое чистое значение за 400 ppm. Если комнату ночью не проветривают, значения занижаются. Отключите: `setAutoCalibration(false)`, и калибруйте вручную на улице раз в несколько месяцев.

**Датчик молчит (ERR_NO_DATA)**

Проверьте питание 5 В (Vin, а не 3.3 В), TX датчика на RX платы и RX на TX (крест-накрест), общий GND, скорость 9600. На Uno используйте `SoftwareSerial` (пины 10, 11): аппаратный `Serial` занят USB.

**Температура датчика не совпадает с комнатной**

Температура из MH-Z19 это грубая внутренняя оценка (±2-3 градуса и зависит от нагрева платы). Для измерения температуры берите AHT20, BME280 или DS18B20. Датчик CO2 не нужно компенсировать этой температурой.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Basic`, `Ventilation`, `Calibration`, `Diagnose`

---

## Қазақша

**MH-Z19B / MH-Z19C / MH-Z19E көмірқышқыл газы CO2 датчиктеріне** (UART) арналған қарапайым кітапхана. ppm-мен концентрация және сөзбен баға («тыныс тарылады, желдетіңіз»), калибрлеу, ABC автокалибрлеуі. Блоктамай қабылдау, бақылау сомасын тексеру. Кез келген портпен жұмыс істейді.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashMHZ19** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

### Қосу
| MH-Z19 | Uno / Nano | ESP32 |
|---|---|---|
| Vin | 5 В | 5 В (VIN) |
| GND | GND | GND |
| TX датчика | пин 10 (RX) | GPIO16 (RX2) |
| RX датчика | пин 11 (TX) | GPIO17 (TX2) |

Датчикке 5 В және қызу кезінде 150 мА-ге дейін керек, логикасы 3.3 В, сондықтан ESP32 және ESP8266-ға тікелей қосылады. Датчик TX тақта RX-ке (айқастырып). Порт жылдамдығы әрқашан 9600. Қосқаннан кейінгі алғашқы 1-3 минутта мәндер төмендетілген.

### Жылдам бастау
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

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `AlashMHZ19(port)` | Порттағы датчик |
| `begin()` | Күйді тастау |
| `read(timeoutMs)` | Сұрап, жауапты күту |
| `poll(everyMs)` | loop() ішінде |
| `co2() / temperature()` | CO2 ppm / температура |
| `quality() / qualityText()` | Желдетілу бағасы |
| `calibrateZero()` | Нөлді калибрлеу |
| `setAutoCalibration(on)` | ABC қосу немесе өшіру |
| `setRange(ppm)` | Диапазон |
| `warmedUp() / setWarmupMs(ms)` | Қызу өтті ме |
| `error() / errorText() / diagnose(Serial)` | Қате, хабарлама, өзін-өзі тексеру |

### Бірдеңе жұмыс істемесе
**Әрқашан 400 немесе 410 ppm көрсетеді**

Датчик әлі қызбаған (3 минутқа дейін) немесе оған ауа жетпейді. Оған үрлеңіз: мән 1000-2000 ppm-ге дейін өсуі керек. Өспесе, 5 В қуатын тексеріп, `Diagnose` мысалын іске қосыңыз.

**Таза ауада 600-800 ppm көрсетеді**

Датчик «ауытқыған». Оны кемінде 20 минутқа далаға шығарып, `calibrateZero()` орындаңыз (`Calibration` мысалы). Бөлмеде нөлді калибрлеуге болмайды: датчик бөлме ауасын 400 ppm деп есте сақтайды.

**Бөлмедегі көрсеткіштер төмендетілген және «жылжиды»**

ABC автокалибрлеуі қосулы: тәулігіне бір рет ең таза мәнді 400 ppm деп алады. Бөлме түнде желдетілмесе, мәндер төмендейді. Өшіріңіз: `setAutoCalibration(false)`, және бірнеше айда бір рет далада қолмен калибрлеңіз.

**Датчик үнсіз (ERR_NO_DATA)**

5 В қуатын (Vin, 3.3 В емес), датчик TX-ін тақта RX-ке және RX-ті TX-ке (айқастырып), ортақ GND, 9600 жылдамдығын тексеріңіз. Uno-да `SoftwareSerial` қолданыңыз (10, 11 пиндері): аппараттық `Serial` USB-мен бос емес.

**Датчик температурасы бөлме температурасына сәйкес келмейді**

MH-Z19 температурасы шамалап ішкі баға (±2-3 градус, тақтаның қызуына байланысты). Температураны өлшеу үшін AHT20, BME280 немесе DS18B20 алыңыз. CO2 датчигін осы температурамен түзетудің қажеті жоқ.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Basic`, `Ventilation`, `Calibration`, `Diagnose`

---

## English

A simple library for the **MH-Z19B / MH-Z19C / MH-Z19E carbon dioxide CO2 sensors** (UART). The concentration in ppm and a verbal rating ("stuffy, ventilate"), calibration, ABC auto-calibration. Non-blocking reception, checksum verification. Works with any port. Made for ecology lessons and Alash Electronics kits.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashMHZ19** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

### Wiring
| MH-Z19 | Uno / Nano | ESP32 |
|---|---|---|
| Vin | 5 В | 5 В (VIN) |
| GND | GND | GND |
| TX датчика | пин 10 (RX) | GPIO16 (RX2) |
| RX датчика | пин 11 (TX) | GPIO17 (TX2) |

The sensor needs 5 V and up to 150 mA while heating, with 3.3 V logic, so it connects directly to ESP32 and ESP8266. The sensor TX to the board RX (crossed). The port speed is always 9600. For the first 1-3 minutes after switch-on the values are low.

### Quick start
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

### Method reference
| Method | What it does |
|---|---|
| `AlashMHZ19(port)` | A sensor on the port Serial2 / SoftwareSerial / Serial1 |
| `begin()` | Reset the state; open the port yourself at 9600 |
| `read(timeoutMs)` | Request and wait for the reply (blocks) |
| `poll(everyMs)` | In loop(): requests and parses by itself; true on a new value |
| `co2() / temperature()` | CO2 in ppm / the sensor temperature (rough) |
| `quality() / qualityText()` | The ventilation rating |
| `calibrateZero()` | Zero calibration (outdoors only) |
| `setAutoCalibration(on)` | Switch ABC on or off |
| `setRange(ppm)` | Range 2000, 5000 or 10000 |
| `warmedUp() / setWarmupMs(ms)` | Has the warm-up passed |
| `error() / errorText() / diagnose(Serial)` | Error, message, self-check in ru/kk/en |

### If something does not work
**It always shows 400 or 410 ppm**

The sensor has not warmed up yet (up to 3 minutes) or the air does not reach it. Breathe on it: the value should rise to 1000-2000 ppm. If not, check the 5 V power and run the `Diagnose` example.

**In fresh air it shows 600-800 ppm**

The sensor has drifted. Take it outdoors for at least 20 minutes and run `calibrateZero()` (the `Calibration` example). Never calibrate zero indoors: the sensor will remember the room air as 400 ppm.

**Readings in a room are too low and drift**

The ABC auto-calibration is on: once a day it takes the cleanest value as 400 ppm. If the room is not ventilated at night the values go low. Switch it off with `setAutoCalibration(false)` and calibrate by hand outdoors every few months.

**The sensor is silent (ERR_NO_DATA)**

Check the 5 V power (Vin, not 3.3 V), the sensor TX to the board RX and RX to TX (crossed), a common GND, the speed 9600. On Uno use `SoftwareSerial` (pins 10, 11): the hardware `Serial` is busy with USB.

**The sensor temperature does not match the room**

The MH-Z19 temperature is a rough internal estimate (±2-3 degrees and depends on the board heating). To measure temperature use an AHT20, BME280 or DS18B20. The CO2 reading does not need compensation with this temperature.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Basic`, `Ventilation`, `Calibration`, `Diagnose`

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashMHZ19
