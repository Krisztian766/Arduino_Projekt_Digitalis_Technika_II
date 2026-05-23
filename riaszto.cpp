#include <Wire.h>

#define LCD_ADDR 0x20
#define LCD_BACKLIGHT 0x08
#define LCD_EN 0x04
#define LCD_RW 0x02
#define LCD_RS 0x01

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

void lcdClear() {
  lcdParancs(0x01);
  delay(2);
}

void lcdKurzor(uint8_t sor, uint8_t oszlop) {
  uint8_t cim = (sor == 0) ? 0x80 + oszlop : 0xC0 + oszlop;
  lcdParancs(cim);
}

void lcdPrint(const char* szoveg) {
  for (int i = 0; szoveg[i] != '\0'; i++) {
    lcdKarakter(szoveg[i]);
  }
}


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

const String HELYES_KOD = "1234";

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


void riasztoHang() {
  unsigned long most = millis();

  
  unsigned long fazisEltelt = most - hangIdeje;

  if (fazisEltelt >= 400) {
    hangIdeje = most;
    hangFazis = (hangFazis + 1) % 4;
  }

  
  float arany = (float)fazisEltelt / 400.0;
  int freq;

  switch (hangFazis) {
    case 0: 
      freq = 800 + (int)(800.0 * arany);
      break;
    case 1: 
      freq = 1600 - (int)(800.0 * arany);
      break;
    case 2: 
      freq = 1000 + (int)(1000.0 * arany);
      break;
    case 3: 
      freq = 600;
      break;
    default:
      freq = 1000;
  }

  tone(BUZZER_PIN, freq);

  
  if (most - ledIdeje >= 200) {
    ledIdeje = most;
    ledAllapot = !ledAllapot;
    digitalWrite(LED_PIN, ledAllapot ? HIGH : LOW);
  }
}


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


void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(PIR_PIN,    INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN,    OUTPUT);
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);

  for (int r = 0; r < 4; r++) {
    pinMode(ROW_PINS[r], OUTPUT);
    digitalWrite(ROW_PINS[r], HIGH);
  }
  for (int c = 0; c < 4; c++) {
    pinMode(COL_PINS[c], INPUT_PULLUP);
  }

  lcdInit();
  lcdFrissit();
  Serial.println("=== Riasztórendszer kesz ===");
}


void loop() {
  char bill = olvasKeypad();

  if (bill == 'D') {
    allapotValt(INAKTIV);
    return;
  }

  switch (allapot) {

    case INAKTIV:
      if (bill) {
        if (bill == '#') {
          beirtKod = "";
          lcdFrissit();
        } else if (bill == '*') {
          if (beirtKod == HELYES_KOD) {
            allapotValt(AKTIV);
          } else {
            lcdHibas();
            delay(1200);
            beirtKod = "";
            lcdFrissit();
          }
        } else if (beirtKod.length() < 8) {
          beirtKod += bill;
          lcdFrissit();
        }
      }
      break;

    case AKTIV:
      if (digitalRead(PIR_PIN) == HIGH) {
        allapotValt(RIASZTAS);
      }
      break;

    case RIASZTAS:
      // Folyamatos szirénázás
      riasztoHang();

      // LCD frissítés 800ms-onként
      if (millis() - lcdIdeje >= 800) {
        lcdIdeje = millis();
        lcdFrissit();
      }
      break;
  }
}


void allapotValt(Allapot uj) {
  allapot = uj;
  beirtKod = "";
  noTone(BUZZER_PIN);
  digitalWrite(LED_PIN, LOW);
  hangFazis = 0;
  hangIdeje = millis();
  lcdFrissit();
  switch (uj) {
    case INAKTIV:  Serial.println(">> INAKTIV");  break;
    case AKTIV:    Serial.println(">> AKTIV");    break;
    case RIASZTAS:
      hangIdeje = millis();
      Serial.println(">> RIASZTAS");
      break;
  }
}

void lcdFrissit() {
  lcdClear();
  switch (allapot) {
    case INAKTIV:
      lcdKurzor(0, 0); lcdPrint("  INAKTIV       ");
      lcdKurzor(1, 0);
      if (beirtKod.length() == 0) {
        lcdPrint("Kod: _          ");
      } else {
        char sor[17] = "Kod: ";
        for (unsigned int i = 0; i < beirtKod.length() && i < 11; i++) {
          sor[5 + i] = '*';
          sor[6 + i] = '\0';
        }
        lcdPrint(sor);
      }
      break;
    case AKTIV:
      lcdKurzor(0, 0); lcdPrint("  AKTIV         ");
      lcdKurzor(1, 0); lcdPrint("Figyel [D=ki]   ");
      break;
    case RIASZTAS:
      lcdKurzor(0, 0); lcdPrint("*** RIASZTAS ***");
      lcdKurzor(1, 0); lcdPrint("Mozgas! [D=stop]");
      break;
  }
}

void lcdHibas() {
  lcdClear();
  lcdKurzor(0, 0); lcdPrint("  Hibas kod!    ");
  lcdKurzor(1, 0); lcdPrint("  Probald ujra  ");
}