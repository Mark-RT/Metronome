#include <GyverOLED.h>
GyverOLED<SSD1306_128x32, OLED_NO_BUFFER> oled;

#include <TimerOne.h>
#include <GyverEncoder.h>
#define CLK 2
#define DT 3
#define SW 4
Encoder enc(CLK, DT, SW);

#define doli 5
#define dop_doli 6
#define pedal 7
#define soundPin 10

int freq = 985;
byte delayImp = 22;                      //длина сигнала
int tones[] = {freq + 950, freq, freq, freq, freq, freq, freq, freq};  //высота тона

byte bits = 3;
byte temp = 120;      //стартовое значения
byte i = 0;
uint32_t btnTimer = 0;
uint32_t dop_doliTimer = 0;
uint32_t doliTimer = 0;
uint32_t timing1 = 0;
uint32_t timing2 = 0;
byte debounce_time = 110;

bool play_flag = 0;
bool beat_flag = 0;
bool main_flag = 0;
bool firstBeat_flag = 0;
bool dop_doli_flag = 0;
bool doli_flag = 0;

void oled_output(byte temp, byte bits, bool beat_flag) {
  oled.home();
  if (!beat_flag) {
    if (temp >= 100) {
      oled.print(temp);
      oled.print("   ");
      oled.print(bits + 1);
    }
    else if (temp >= 10 && temp <= 99) {
      oled.print(temp);
      oled.print("    ");
      oled.print(bits + 1);
    }
    else {
      oled.print(temp);
      oled.print("     ");
      oled.print(bits + 1);
    }
  }
  else if (beat_flag) {
    if (temp >= 100) {
      oled.print(temp);
      oled.print("  *");
      oled.print(bits + 1);
    }
    else if (temp >= 10 && temp <= 99) {
      oled.print(temp);
      oled.print("   *");
      oled.print(bits + 1);
    }
    else {
      oled.print(temp);
      oled.print("    *");
      oled.print(bits + 1);
    }
  }
}

void timerIsr() {
  enc.tick();
}

void setup() {
  Serial.begin(115200);
  enc.setType(TYPE2);
  Timer1.initialize(1000);           // установка таймера на каждые 1000 микросекунд (= 1 мс)
  Timer1.attachInterrupt(timerIsr);  // запуск таймера

  pinMode(soundPin, OUTPUT);
  pinMode(dop_doli, INPUT_PULLUP);
  pinMode(doli, INPUT_PULLUP);
  pinMode(pedal, INPUT_PULLUP);

  oled.init();
  oled.clear();
  oled.setCursor(0, 3);
  oled.setScale(1);
  oled.print("ON ");
  oled.setScale(3);
  oled_output(temp, bits, beat_flag);
}

void loop() {
  if (enc.isRight()) {
    temp++;
    oled_output(temp, bits, beat_flag);
  }
  if (enc.isLeft()) {
    temp--;
    oled_output(temp, bits, beat_flag);
  }
  if (enc.isFastR()) {
    temp += 4;
    oled_output(temp, bits, beat_flag);
  }
  if (enc.isFastL()) {
    temp -= 4;
    oled_output(temp, bits, beat_flag);
  }
  if (enc.isSingle()) {
    firstBeat_flag = !firstBeat_flag;
    if (firstBeat_flag) {
      tones[0] = freq;
      oled.setCursor(0, 3);
      oled.setScale(1);
      oled.print("OFF");
      oled.setScale(3);
    }
    else {
      tones[0] = freq + 950;
      oled.setCursor(0, 3);
      oled.setScale(1);
      oled.print("ON ");
      oled.setScale(3);
    }
  }

  if (!digitalRead(doli) && !doli_flag && millis() - doliTimer > debounce_time) {
    if (bits == 3) {
      bits = 2;
      oled_output(temp, bits, beat_flag);
    }
    else {
      bits = 3;
      oled_output(temp, bits, beat_flag);
    }
    oled_output(temp, bits, beat_flag);
    doli_flag = 1;
    doliTimer = millis();
  }
  else if (digitalRead(doli) && doli_flag && millis() - doliTimer > debounce_time) {
    doli_flag = 0;
    doliTimer = millis();
  }

  if (!digitalRead(dop_doli) && !dop_doli_flag && millis() - dop_doliTimer > debounce_time) {
    beat_flag = !beat_flag;
    oled_output(temp, bits, beat_flag);
    dop_doli_flag = 1;
    dop_doliTimer = millis();
  }
  else if (digitalRead(dop_doli) && dop_doli_flag && millis() - dop_doliTimer > debounce_time) {
    dop_doli_flag = 0;
    dop_doliTimer = millis();
  }

  if (!digitalRead(pedal) && !main_flag && millis() - btnTimer > debounce_time) {
    play_flag = !play_flag;
    main_flag = 1;
    i = 0;
    btnTimer = millis();
  }
  else if (digitalRead(pedal) && main_flag && millis() - btnTimer > debounce_time) {
    main_flag = 0;
    btnTimer = millis();
  }

  if (play_flag && millis() - timing1 > (60000 - delayImp) / temp) {
    if (millis() - timing1 > (60000 / temp)) timing1 = millis() - 1;
    else timing1 = millis();
    tone(soundPin, tones[i], delayImp);
    i = (i > bits - 1) ? 0 : i + 1;
    timing2 = timing1 + ((60000 - delayImp) / (temp * 2));
  }
  if (play_flag && beat_flag && millis() >= timing2 && millis() <= timing2 + 2) tone(soundPin, freq, delayImp / 2);
}
