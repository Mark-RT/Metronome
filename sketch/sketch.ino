#include <GyverOLED.h>
GyverOLED<SSD1306_128x32, OLED_NO_BUFFER> oled;

#include <GyverEncoder.h>
#include <TimerOne.h>
#define CLK 5
#define DT 6
#define SW 4
Encoder enc(CLK, DT, SW);

#define button 2
#define pedal 3
#define soundPin 11
byte delayImp = 5;     //длина сигнала
int value = 1000;     //высота тона импульса (квадратного)

byte max_speed = 250;
byte min_speed = 20;

byte temp = 120;      //установка стартового значения (среднее)
byte bits = 3;
byte i = 0;
volatile boolean pedal_flag = 0;
volatile boolean flag = 0;
volatile unsigned long timing;

void setup() {
  Serial.begin(9600);
  enc.setType(TYPE2);
  attachInterrupt(0, interBut, RISING);    // прерывание на 2 пине! CLK у энка
  attachInterrupt(1, interPedal, FALLING);
  Timer1.initialize(1000);           // установка таймера на каждые 1000 микросекунд (= 1 мс)
  Timer1.attachInterrupt(timerIsr);   // запуск таймера

  pinMode(soundPin, OUTPUT);
  pinMode(button, INPUT);
  pinMode(pedal, INPUT_PULLUP);

  oled.init();        // инициализация
  oled.clear();
  oled.setScale(3);
}

void interBut() {
  flag = !flag;
}
void interPedal() {
  if (pedal_flag) {       // защита от дребезга
    flag = !flag;
    timing = millis();
    pedal_flag = 0;
  }
}
void timerIsr() {   // прерывание таймера
  enc.tick();     // отработка теперь находится здесь
}

void loop() {
  if (flag == 0) {
    if (enc.isRight()) temp++;
    if (enc.isLeft()) temp--;
    if (enc.isFastR()) temp += 3;
    if (enc.isFastL()) temp -= 3;
    if (enc.isRightH()) bits = 3;
    if (enc.isLeftH())  bits = 2;

    if (temp > max_speed) temp = max_speed;
    if (temp < min_speed) temp = min_speed;

    oled.home();
    oled.print(temp);
    oled.print("   ");
    oled.print(bits + 1);
    oled.print(" ");
  }
  if (millis() - timing > 100) pedal_flag = 1;  // защита от дребезга
  if (flag == 1) {
    i = 0;
    tone(soundPin, value + 950);
    delay(delayImp);
    noTone(soundPin);
    delay(59500 / temp);
    if (millis() - timing > 100) pedal_flag = 1;   // защита от дребезга
    while (flag == 1 && i < bits) {
      if (millis() - timing > 100) pedal_flag = 1;  // защита от дребезга
      Serial.println("check");
      tone(soundPin, value);
      delay(delayImp);
      noTone(soundPin);
      delay(59500 / temp);
      i++;
    }
  }
}

/*Напиши код для метронома на Arduino Nano. Нужно чтобы при вращении ручки энкодера менялась скорость темпа от 20 до 200. И это значение нужно выводить на OLED дисплей 128 на 32. Используй для вывода на дисплей библиотеку GyverOLED.h. А для энкодера используй библиотеку GyverEncoder.h. При нажатии кнопки будут запускаться щелчки метронома с заданным темпом. Для генерации сигналов метронома используй функцию tone(). Первый удар должен быть высокой частоты, остальные немного ниже. Также при удержании и повороте энкодера, то есть функция enc.isRightH() и enc.isLeftH(), должно меняться количество ударов - три четверти или четыре четверти. Если была нажата кнопка, то метроном начинает считать. Если кнопку нажали ещё раз, то он останавливается.
Код для метронома с использованием энкодера и OLED дисплея:

C++
Copy code
#include <GyverOLED.h>
#include <GyverEncoder.h>

#define BUTTON_PIN 2
#define SPEAKER_PIN 8
#define OLED_SDA 4
#define OLED_SCL 5
#define OLED_ADDR 0x3C
#define OLED_FONT u8g2_font_crox4hb_tf
#define OLED_FONT_HEIGHT 9

GyverOLED OLED(OLED_SDA, OLED_SCL, OLED_ADDR);
GyverEncoder enc(3, 9);
volatile int buttonPressed = 0;
volatile int bpm = 120;
volatile int beatsPerMeasure = 4;
volatile int beatCount = 0;
volatile long prevTick = 0;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SPEAKER_PIN, OUTPUT);
  enc.setType(TYPE2);
  OLED.begin();
  OLED.setFont(OLED_FONT);
  OLED.clear();
}

void loop() {
  // Обработка события изменения энкодера
  int encVal = enc.getValue() * 0.5 + 20;
  if (encVal != bpm) {
    bpm = encVal;
    OLED.setCursor(0, OLED_FONT_HEIGHT);
    OLED.print("BPM: ");
    OLED.print(bpm);
    OLED.print("  ");
  }

  // Обработка события нажатия кнопки
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && !buttonPressed) {
    buttonPressed = 1;
    beatCount = 0;
    prevTick = micros();
  } else if (buttonState == HIGH && buttonPressed) {
    buttonPressed = 0;
  }

  // Подсчет интервала между ударами и генерация звука
  if (buttonPressed) {
    long curTick = micros();
    long interval = 60000000 / bpm;
    if (curTick - prevTick >= interval) {
      prevTick = curTick;
      beatCount++;

      // Определение частоты звука в зависимости от номера удара
      int toneFreq = 440;
      if (beatCount == 1) {
        toneFreq = 880;
      } else if (beatCount % beatsPerMeasure == 1) {
        toneFreq = 660;
      }
      tone(SPEAKER_PIN, toneFreq, 50);
    }
  }

  // Обработка событий изменения параметра "четверти" и вывод на OLED дисплей
  if (enc.isRightH()) {
    beatsPerMeasure = 4;
    OLED.setCursor(0, OLED_FONT_HEIGHT * 2);
    OLED.print("4/4  ");
  } else if (enc.isLeftH()) {
    beatsPerMeasure = 3;
    OLED.setCursor(0, OLED_FONT_HEIGHT * 2);
    OLED.print("3/4  ");
  }
}*/
