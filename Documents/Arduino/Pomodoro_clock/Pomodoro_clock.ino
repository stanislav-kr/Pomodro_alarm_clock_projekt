#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Піни кнопок і бузера ---
#define BUTTON_HOUR A0
#define BUTTON_MINUTE A1
#define BUTTON_MODE A2
#define BUZZER_PIN 7

// --- Режими ---
enum Mode {
  MODE_CLOCK,
  MODE_SET_ALARM,
  MODE_POMODORO,
};
Mode mode = MODE_CLOCK;

// --- Змінні для будильника ---
int alarmHour = 7;
int alarmMinute = 0;

// --- Змінні для pomodoro ---
unsigned long pomodoroStartTime = 0;
bool pomodoroWorkPhase = true;

// --- Антидребезг ---
unsigned long lastDebounceTime = 0;
const int debounceDelay = 200;

// --- Кнопки: час останнього натискання ---
unsigned long lastPressTimeHour = 0;
unsigned long lastPressTimeMinute = 0;
int pressCountHour = 0;
int pressCountMinute = 0;

void playAlarmSound() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 1000, 500); // 1000 Гц
    delay(500);
    noTone(BUZZER_PIN);
    delay(500);
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  rtc.begin();
  rtc.adjust(DateTime(2025, 7, 8, 14, 13, 0));

  pinMode(BUTTON_HOUR, INPUT_PULLUP);
  pinMode(BUTTON_MINUTE, INPUT_PULLUP);
  pinMode(BUTTON_MODE, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
}


void loop() {
  DateTime now = rtc.now();
  int currentHour = now.hour();
  int currentMinute = now.minute();
  int currentSecond = now.second();

  if (currentHour == alarmHour && currentMinute == alarmMinute && currentSecond == 0) {
    playAlarmSound();
  }

  handleButtons();

  switch (mode) {
    case MODE_CLOCK:
      updateClockDisplay();
      break;
    case MODE_SET_ALARM:
      updateAlarmSetting();
      break;
    case MODE_POMODORO:
      updatePomodoro();
      break;
  }
}

// --- Обробка всіх кнопок ---
void handleButtons() {
  if (millis() - lastDebounceTime > debounceDelay) {
    if (digitalRead(BUTTON_HOUR) == LOW) {
      handleHourButton();
      lastDebounceTime = millis();
    } else if (digitalRead(BUTTON_MINUTE) == LOW) {
      handleMinuteButton();
      lastDebounceTime = millis();
    } else if (digitalRead(BUTTON_MODE) == LOW) {
      handleModeButton();
      lastDebounceTime = millis();
    }
  }
}

// --- Обробка кнопки "Hour" ---
void handleHourButton() {
  if (mode == MODE_SET_ALARM) {
    alarmHour = (alarmHour + 1) % 24;
  }
}

// --- Обробка кнопки "Minute" ---
void handleMinuteButton() {
  if (mode == MODE_SET_ALARM) {
    alarmMinute = (alarmMinute + 1) % 60;
  }
}

// --- Обробка кнопки "Mode" ---
void handleModeButton() {
  mode = static_cast<Mode>((mode + 1) % 3);
  lcd.clear();
}

// --- Оновлення дисплея у режимі годинника ---
void updateClockDisplay() {
  DateTime now = rtc.now();
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  printTwoDigits(now.hour());
  lcd.print(":");
  printTwoDigits(now.minute());

  lcd.setCursor(0, 1);
  lcd.print("Alarm: ");
  printTwoDigits(alarmHour);
  lcd.print(":");
  printTwoDigits(alarmMinute);
}

// --- Оновлення дисплея у режимі налаштування будильника ---
void updateAlarmSetting() {
  lcd.setCursor(0, 0);
  lcd.print("Set Alarm Time");
  lcd.setCursor(0, 1);
  printTwoDigits(alarmHour);
  lcd.print(":");
  printTwoDigits(alarmMinute);
}

// --- Оновлення логіки Pomodoro ---
void updatePomodoro() {
  unsigned long currentTime = millis();
  unsigned long elapsed = (currentTime - pomodoroStartTime) / 1000;
  int duration = pomodoroWorkPhase ? 25 * 60 : 5 * 60;

  if (elapsed >= duration) {
    pomodoroWorkPhase = !pomodoroWorkPhase;
    pomodoroStartTime = currentTime;
    tone(BUZZER_PIN, 1000, 500);
  }

  int remaining = duration - elapsed;
  int minutes = remaining / 60;
  int seconds = remaining % 60;

  lcd.setCursor(0, 0);
  lcd.print(pomodoroWorkPhase ? "Work!" : "Break!");
  lcd.setCursor(0, 1);
  printTwoDigits(minutes);
  lcd.print(":");
  printTwoDigits(seconds);
}

// --- Допоміжна функція для виводу 2-значних чисел ---
void printTwoDigits(int number) {
  if (number < 10) lcd.print("0");
  lcd.print(number);
}
