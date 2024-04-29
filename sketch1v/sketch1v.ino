#include <GyverOLED.h>
GyverOLED<SSD1306_128x32, OLED_NO_BUFFER> oled;

#include <TimerOne.h>
#include <GyverEncoder.h>
#define CLK 5
#define DT 6
#define SW 4
Encoder enc(CLK, DT, SW);

#define pedal 2
#define soundPin 11

int freq = 985;
byte delayImp = 22;                      //длина сигнала
int tones[] = {freq + 950, freq, freq, freq, freq, freq, freq, freq};  //высота тона

byte beats[] = {2, 3, 5, 7};
byte temp = 120;      //стартовое значения
byte i = 0;
byte u = 1;
byte bits = beats[u];
uint32_t btnTimer = 0;
uint32_t timing1 = 0;
uint32_t timing2 = 0;
byte debounce_time = 110;

bool play_flag = 0;
bool beat_flag = 0;
bool main_flag = 0;
bool firstBeat_flag = 0;

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
  Serial.begin(9600);
  enc.setType(TYPE2);
  Timer1.initialize(1000);           // установка таймера на каждые 1000 микросекунд (= 1 мс)
  Timer1.attachInterrupt(timerIsr);  // запуск таймера

  pinMode(soundPin, OUTPUT);
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
  if (enc.isRightH()) {
    u = (u == 3) ? 3 : u + 1;
    bits = beats[u];
    oled_output(temp, bits, beat_flag);
  }
  if (enc.isLeftH())  {
    u = (u == 0) ? 0 : u - 1;
    bits = beats[u];
    oled_output(temp, bits, beat_flag);
  }
  if (enc.isSingle()) {
    beat_flag = !beat_flag;
    oled_output(temp, bits, beat_flag);
  }
  if (enc.isDouble()) {
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
    Serial.println(millis());
    timing1 = millis();
    tone(soundPin, tones[i]);
    delay(delayImp);
    noTone(soundPin);
    i = (i > bits - 1) ? 0 : i + 1;
    timing2 = timing1 + ((60000 - delayImp) / (temp * 2));
  }
  if (play_flag && beat_flag && millis() >= timing2 && millis() <= timing2 + 2) {
    tone(soundPin, freq);
    delay(delayImp / 2);
    noTone(soundPin);
  }
}
