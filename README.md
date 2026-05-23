# 🚨 Automata Riasztórendszer

**Tinkercad és Arduino alapú beadandó dokumentáció**

![Arduino](https://img.shields.io/badge/Arduino-Uno-00979D?style=flat&logo=arduino&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Tinkercad-E85B2A?style=flat)
![Language](https://img.shields.io/badge/Language-C%2B%2B-blue?style=flat&logo=cplusplus)
![License](https://img.shields.io/badge/License-MIT-green?style=flat)
![Status](https://img.shields.io/badge/Status-Kész-brightgreen?style=flat)

> 🔗 **Tinkercad szimuláció:** [Kattints ide a szimulációhoz](https://www.tinkercad.com/things/lGVWRD1bFoh/editel?returnTo=%2Fdashboard&sharecode=e10EUwuSezz-bokL_V-uNGx4AJHEmUH73w_tv3J7cUM)

| Adat | Leírás |
|---|---|
| Készítette | [Neved - Neptun kód] |
| Téma | Automata riasztórendszer szimulációja |
| Környezet | Tinkercad Circuits |
| Vezérlő | Arduino Uno R3 |
| Fő funkció | PIR érzékelés, kódzár, szirén, LCD kijelzés |

---

## 📷 Kapcsolási rajz

![Kapcsolási rajz](images/kapcsolasi_rajz.png)

*1. ábra: A Tinkercad-ben elkészített kapcsolás*

---

## 📋 Tartalomjegyzék

- [A projekt célja](#1-a-projekt-célja)
- [Felhasznált alkatrészek](#2-felhasznált-alkatrészek)
- [Pin kiosztás és bekötés](#3-pin-kiosztás-és-bekötés)
- [Működési elv](#4-a-rendszer-működési-elve)
- [Programkód](#5-teljes-arduino-programkód)
- [Tesztelés](#6-tesztelés-tinkercadben)
- [Hibakeresés](#7-hibakeresés)
- [Összegzés](#8-összegzés)

---

## 1. A projekt célja

A beadandó célja egy kódzárral védett automata riasztórendszer elkészítése Tinkercad környezetben. A rendszer Arduino Uno vezérlőre épül, amely egy PIR mozgásérzékelő jelét dolgozza fel. A riasztó élesítése és hatástalanítása 4×4-es billentyűzettel és titkos kóddal történik. Mozgás esetén piezo szirén és villogó LED jelez, az állapotokat I2C LCD kijelző mutatja.

### 1.1. A megoldandó feladat

- A rendszer élesítése titkos kód beírásával (keypad + `*` gomb).
- PIR mozgásérzékelő folyamatos figyelése élesített állapotban.
- Mozgás esetén wee-woo szirénázó hang (`tone()` alapú frekvenciasöprés).
- Piros LED villogása riasztás alatt.
- `D` gomb = azonnali hatástalanítás.
- LCD kijelző: `INAKTIV` / `AKTIV` / `*** RIASZTAS ***` állapotok.

### 1.2. Miért hasznos ez a rendszer?

Egy kódzárral védett riasztórendszer csak jogosult személyek számára kapcsolható ki, így védelmet nyújt illetéktelen belépés ellen. A Tinkercad szimuláció jól szemlélteti az állapotgép alapú vezérlési logikát, a digitális be- és kimenetek kezelését, valamint az I2C kommunikációt.

---

## 2. Felhasznált alkatrészek

| Alkatrész | Feladat |
|---|---|
| Arduino Uno R3 | Központi vezérlő, minden alkatrészt kezel. |
| PIR HC-SR501 | Mozgásérzékelő, HIGH jelet ad mozgás esetén. |
| 4×4 Keypad | Kód bevitelére és a D (kikapcsoló) gombra. |
| LCD 16×2 I2C (PCF8574) | Állapot kijelzése (INAKTIV / AKTIV / RIASZTAS). |
| Piezo buzzer (passzív) | Szirénázó hang `tone()` függvénnyel. |
| Piros LED | Riasztás alatt villog. |
| 220 Ω ellenállás | LED áramkorlátozás. |
| Breadboard + vezetékek | Összekötés. |

---

## 3. Pin kiosztás és bekötés

| Alkatrész | Láb / jel | Arduino pin |
|---|---|---|
| PIR HC-SR501 | VCC | 5V |
| PIR HC-SR501 | GND | GND |
| PIR HC-SR501 | OUT | D2 |
| Keypad | R1, R2, R3, R4 (sorok) | D4, D5, D6, D7 |
| Keypad | C1, C2, C3, C4 (oszlopok) | D8, D9, D10, D11 |
| Piezo buzzer | + (pozitív) | D12 |
| Piezo buzzer | – (negatív) | GND |
| Piros LED | Anód (+ 220Ω) | D13 |
| Piros LED | Katód | GND |
| LCD I2C (PCF8574) | VCC | 5V |
| LCD I2C (PCF8574) | GND | GND |
| LCD I2C (PCF8574) | SDA | A4 |
| LCD I2C (PCF8574) | SCL | A5 |

### 3.1. Fontos megjegyzések

- A LED-hez kötelező a **220 Ω ellenállás** — nélküle az Arduino pinje tönkremegy.
- A keypad sorpinei `OUTPUT HIGH`, oszloppinei `INPUT_PULLUP` módban vannak.
- Az LCD I2C cím `0x20` (32 decimális) — PCF8574 chip, A0–A2 jumperek alapján.
- A piezo **passzív buzzer** — csak passzív buzzernél működik a `tone()` szirén.

---

## 4. A rendszer működési elve

A program három állapotot különböztet meg: `INAKTIV`, `AKTIV` és `RIASZTAS`.

### 4.1. Állapottáblázat

| Állapot | LCD 1. sor | LCD 2. sor | Buzzer | LED |
|---|---|---|---|---|
| INAKTIV | `INAKTIV` | `Kod: ****` | Csendes | Ki |
| AKTIV | `AKTIV` | `Figyel [D=ki]` | Csendes | Ki |
| RIASZTAS | `*** RIASZTAS ***` | `Mozgas! [D=stop]` | Szirénázik | Villog |

### 4.2. Állapotátmenetek

```
┌─────────────────────────────────────────────────────┐
│                                                     │
│   [INAKTIV] ──helyes kód + *──► [AKTIV]             │
│       ▲                            │                │
│       │                        PIR mozgás           │
│       │                            │                │
│    D gomb ◄────────────────── [RIASZTAS]            │
│    (bármikor)                                       │
│                                                     │
└─────────────────────────────────────────────────────┘
```

| Esemény | Eredmény |
|---|---|
| Helyes kód + `*` gomb | INAKTIV → AKTIV |
| Rossz kód + `*` gomb | Marad INAKTIV, "Hibas kod!" üzenet |
| PIR mozgást érzékel (AKTIV-ban) | AKTIV → RIASZTAS |
| `D` gomb lenyomása (bármikor) | → INAKTIV (hatástalanítás) |
| `#` gomb | Beírt kód törlése |

### 4.3. Szirénázó hang működése

A piezo buzzer a `tone()` Arduino függvénnyel szólal meg. A `riasztoHang()` függvény 4 fázisban folyamatosan változtatja a frekvenciát, klasszikus **wee-woo** szirénát imitálva:

| Fázis | Frekvencia | Jelleg | Időtartam |
|---|---|---|---|
| 1 | 800 Hz → 1600 Hz | Felmegy (wee) | 400 ms |
| 2 | 1600 Hz → 800 Hz | Lemegy (woo) | 400 ms |
| 3 | 1000 Hz → 2000 Hz | Gyors felmegy | 400 ms |
| 4 | 600 Hz (fix) | Mély szünet | 400 ms |

---

## 5. Teljes Arduino programkód

```cpp
// ============================================================
// Riasztórendszer - Tinkercad (NULLA külső könyvtár)
// Csak: Wire.h (beépített Arduino)
// ============================================================

#include <Wire.h>

#define LCD_ADDR      0x20
#define LCD_BACKLIGHT 0x08
#define LCD_EN        0x04
#define LCD_RW        0x02
#define LCD_RS        0x01

void lcdPulse(uint8_t data) {
  Wire.beginTransmission(LCD_ADDR);
  Wire.write(data | LCD_EN | LCD_BACKLIGHT);
  Wire.endTransmission();
  delayMicroseconds(1);
  Wire.beginTransmission(LCD_ADDR);
  Wire.write((data & ~LCD_EN) | LCD_BACKLIGHT);
  Wire.endTransmission();
  delayMicroseconds(50);
}

void lcdParancs(uint8_t cmd) {
  lcdPulse(cmd & 0xF0);
  lcdPulse((cmd << 4) & 0xF0);
  delay(2);
}

void lcdKarakter(char ch) {
  lcdPulse((ch & 0xF0) | LCD_RS);
  lcdPulse(((ch << 4) & 0xF0) | LCD_RS);
  delayMicroseconds(50);
}

void lcdInit() {
  delay(50);
  lcdPulse(0x30); delay(5);
  lcdPulse(0x30); delay(1);
  lcdPulse(0x30); delay(1);
  lcdPulse(0x20); delay(1);
  lcdParancs(0x28);
  lcdParancs(0x0C);
  lcdParancs(0x06);
  lcdParancs(0x01);
  delay(2);
}

void lcdClear() { lcdParancs(0x01); delay(2); }

void lcdKurzor(uint8_t sor, uint8_t oszlop) {
  lcdParancs((sor == 0) ? 0x80 + oszlop : 0xC0 + oszlop);
}

void lcdPrint(const char* szoveg) {
  for (int i = 0; szoveg[i] != '\0'; i++) lcdKarakter(szoveg[i]);
}

// --- Pinek ---
const int PIR_PIN    = 2;
const int BUZZER_PIN = 12;
const int LED_PIN    = 13;
const int ROW_PINS[4] = {4, 5, 6, 7};
const int COL_PINS[4] = {8, 9, 10, 11};

char keyMap[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

const String HELYES_KOD = "1234";  // <-- itt változtathatod

enum Allapot { INAKTIV, AKTIV, RIASZTAS };
Allapot allapot = INAKTIV;

String beirtKod = "";
unsigned long lcdIdeje = 0;
unsigned long hangIdeje = 0;
int hangFazis = 0;
bool ledAllapot = false;
unsigned long ledIdeje = 0;

void lcdFrissit();
void lcdHibas();
void allapotValt(Allapot uj);

char olvasKeypad() {
  for (int r = 0; r < 4; r++) {
    digitalWrite(ROW_PINS[r], LOW);
    for (int c = 0; c < 4; c++) {
      if (digitalRead(COL_PINS[c]) == LOW) {
        delay(50);
        while (digitalRead(COL_PINS[c]) == LOW);
        digitalWrite(ROW_PINS[r], HIGH);
        return keyMap[r][c];
      }
    }
    digitalWrite(ROW_PINS[r], HIGH);
  }
  return 0;
}

void riasztoHang() {
  unsigned long fazisEltelt = millis() - hangIdeje;
  if (fazisEltelt >= 400) {
    hangIdeje = millis();
    hangFazis = (hangFazis + 1) % 4;
    fazisEltelt = 0;
  }
  float arany = (float)fazisEltelt / 400.0;
  int freq;
  switch (hangFazis) {
    case 0: freq = 800  + (int)(800.0  * arany); break;
    case 1: freq = 1600 - (int)(800.0  * arany); break;
    case 2: freq = 1000 + (int)(1000.0 * arany); break;
    case 3: freq = 600;                           break;
    default: freq = 1000;
  }
  tone(BUZZER_PIN, freq);
  if (millis() - ledIdeje >= 200) {
    ledIdeje = millis();
    ledAllapot = !ledAllapot;
    digitalWrite(LED_PIN, ledAllapot ? HIGH : LOW);
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(PIR_PIN,    INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN,    OUTPUT);
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);
  for (int r = 0; r < 4; r++) { pinMode(ROW_PINS[r], OUTPUT); digitalWrite(ROW_PINS[r], HIGH); }
  for (int c = 0; c < 4; c++) { pinMode(COL_PINS[c], INPUT_PULLUP); }
  lcdInit();
  lcdFrissit();
  Serial.println("=== Riasztórendszer kesz ===");
}

void loop() {
  char bill = olvasKeypad();
  if (bill == 'D') { allapotValt(INAKTIV); return; }

  switch (allapot) {
    case INAKTIV:
      if (bill) {
        if (bill == '#') { beirtKod = ""; lcdFrissit(); }
        else if (bill == '*') {
          if (beirtKod == HELYES_KOD) { allapotValt(AKTIV); }
          else { lcdHibas(); delay(1200); beirtKod = ""; lcdFrissit(); }
        } else if (beirtKod.length() < 8) { beirtKod += bill; lcdFrissit(); }
      }
      break;
    case AKTIV:
      if (digitalRead(PIR_PIN) == HIGH) allapotValt(RIASZTAS);
      break;
    case RIASZTAS:
      riasztoHang();
      if (millis() - lcdIdeje >= 800) { lcdIdeje = millis(); lcdFrissit(); }
      break;
  }
}

void allapotValt(Allapot uj) {
  allapot = uj; beirtKod = "";
  noTone(BUZZER_PIN); digitalWrite(LED_PIN, LOW);
  hangFazis = 0; hangIdeje = millis();
  lcdFrissit();
  switch (uj) {
    case INAKTIV:  Serial.println(">> INAKTIV");  break;
    case AKTIV:    Serial.println(">> AKTIV");    break;
    case RIASZTAS: Serial.println(">> RIASZTAS"); break;
  }
}

void lcdFrissit() {
  lcdClear();
  switch (allapot) {
    case INAKTIV:
      lcdKurzor(0,0); lcdPrint("  INAKTIV       ");
      lcdKurzor(1,0);
      if (beirtKod.length() == 0) { lcdPrint("Kod: _          "); }
      else {
        char sor[17] = "Kod: ";
        for (unsigned int i = 0; i < beirtKod.length() && i < 11; i++) { sor[5+i]='*'; sor[6+i]='\0'; }
        lcdPrint(sor);
      }
      break;
    case AKTIV:
      lcdKurzor(0,0); lcdPrint("  AKTIV         ");
      lcdKurzor(1,0); lcdPrint("Figyel [D=ki]   ");
      break;
    case RIASZTAS:
      lcdKurzor(0,0); lcdPrint("*** RIASZTAS ***");
      lcdKurzor(1,0); lcdPrint("Mozgas! [D=stop]");
      break;
  }
}

void lcdHibas() {
  lcdClear();
  lcdKurzor(0,0); lcdPrint("  Hibas kod!    ");
  lcdKurzor(1,0); lcdPrint("  Probald ujra  ");
}
```

---

## 6. Tesztelés Tinkercadben

### 6.1. Tesztelési lépések

1. Szimuláció indítása a **Start Simulation** gombbal.
2. Serial Monitor megnyitása az állapotüzenetek figyeléséhez.
3. Keypadon `1` `2` `3` `4` begépelése, majd `*` lenyomása → **AKTIV** állapot.
4. PIR szenzorra kattintás (mozgás szimulálása) → **RIASZTAS**.
5. LCD ellenőrzése: `*** RIASZTAS ***` felirat megjelenik-e.
6. LED villogás és buzzer szirénázás ellenőrzése.
7. `D` gomb lenyomása → **INAKTIV** visszaállás.

### 6.2. Elvárt eredmények

| Tesztlépés | Elvárt működés |
|---|---|
| Helyes kód + `*` | LCD: AKTIV, Serial: `>> AKTIV` |
| Rossz kód + `*` | LCD: Hibas kod!, 1.2s után visszaáll |
| PIR mozgás (AKTIV-ban) | LCD: RIASZTAS, buzzer szirénáz, LED villog |
| `D` gomb (bármikor) | Minden leáll, LCD: INAKTIV |
| `#` gomb | Beírt kód törlődik |

---

## 7. Hibakeresés

| Hiba | Lehetséges megoldás |
|---|---|
| LCD nem jelenít meg semmit | I2C cím ellenőrzése: `0x20` helyes-e? SDA→A4, SCL→A5 bekötve? |
| Keypad nem reagál | ROW/COL pinek ellenőrzése; `INPUT_PULLUP` beállítva? |
| Buzzer nem szól | Passzív buzzer szükséges `tone()`-hoz; D12 bekötve? |
| PIR mindig 0 | Tinkercadben kattintani kell a szenzorra a mozgás szimulálásához. |
| Fordítási hiba | Csak `Wire.h` szükséges — más könyvtár nem kell. |

---

## 8. Összegzés

A beadandóban elkészült egy Tinkercad-ben szimulált, kódzárral védett automata riasztórendszer. A rendszer Arduino Uno vezérlőt, PIR mozgásérzékelőt, 4×4-es billentyűzetet, I2C LCD kijelzőt, piezo buzzert és piros LED-et tartalmaz.

A projekt bemutatja:
- Az állapotgép alapú programozást (INAKTIV / AKTIV / RIASZTAS)
- A digitális be- és kimenetek kezelését
- Az I2C protokoll közvetlen vezérlését könyvtár nélkül
- A `tone()` függvény alapú hangszintézist
- A keypad mátrix szkennelési módszerét

### 8.1. Ellenőrzőlista leadás előtt

- [ ] PIR OUT lába D2-re van kötve.
- [ ] Keypad 8 lába D4–D11-re van kötve.
- [ ] Buzzer + lába D12-re, – lába GND-re van kötve.
- [ ] LED anódja 220Ω ellenálláson át D13-ra, katódja GND-re van kötve.
- [ ] LCD SDA→A4, SCL→A5, I2C cím: `0x20`.
- [ ] Helyes kód beírásával (`1234` + `*`) a rendszer AKTIV állapotba lép.
- [ ] PIR mozgás szimulálására a szirén megszólal és a LED villog.
- [ ] `D` gombra a rendszer azonnal INAKTIV állapotba áll vissza.
