#include <GyverOLED.h>
GyverOLED<SSD1306_128x32, OLED_NO_BUFFER> oled;

#include <TimerOne.h>
#include <GyverEncoder.h>
#define CLK 5
#define DT 6
#define SW 4
Encoder enc(CLK, DT, SW);

#define pedal 3
#define soundPin 11
byte delayImp = 10;                      //длина сигнала
int tones[] = {1950, 1000, 1000, 1000};  //высота тона

byte temp = 120;      //стартовое значения
byte bits = 3;
byte i = 0;
uint32_t btnTimer = 0;
uint32_t timing1 = 0;
uint32_t timing2 = 0;
byte debounce_time = 110;

bool play_flag = 0;
bool main_flag = 0;

void oled_output(byte temp, byte bits) {
  oled.home();
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
  oled.setScale(3);
  oled_output(temp, bits);
}

void loop() {
  if (enc.isRight()) {
    temp++;
    oled_output(temp, bits);
  }
  if (enc.isLeft()) {
    temp--;
    oled_output(temp, bits);
  }
  if (enc.isFastR()) {
    temp += 3;
    oled_output(temp, bits);
  }
  if (enc.isFastL()) {
    temp -= 3;
    oled_output(temp, bits);
  }
  if (enc.isRightH()) {
    bits = 3;
    oled_output(temp, bits);
  }
  if (enc.isLeftH())  {
    bits = 2;
    oled_output(temp, bits);
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
    timing1 = millis();
    tone(soundPin, tones[i]);
    delay(delayImp);
    noTone(soundPin);
    i = (i > bits - 1) ? 0 : i + 1;
    timing2 = timing1 + ((60000 - delayImp) / (temp * 2));
    Serial.println(timing1);
  }

  if (play_flag && temp < 86 && millis() >= timing2 && millis() <= timing2 + 2) {
    tone(soundPin, 1000);
    delay(1);
    noTone(soundPin);
  }
}
