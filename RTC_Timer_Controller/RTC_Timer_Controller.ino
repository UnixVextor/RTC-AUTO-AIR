/*
  RTC Timer Air Controller 
  Author  : UnixVextor
  Date    : 15/7/2566 
*/

// Real Time Clock library
#include <ThreeWire.h>
#include <RtcDS1302.h>

// LCD Library
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


ThreeWire myWire(9, 10, 8);  // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

LiquidCrystal_I2C lcd(0x27, 16, 2);  // set LCD address to 0x27 LCD 16 x 2

uint8_t month, day, hour, minute, second;
uint16_t year;

unsigned long long lastTime = 0;

// button
int buttons[3] = { 2, 3, 4 };
unsigned long long debounceDelay = 50;
unsigned long long lastDebounceTime[3];
int lastButtonState[3] = { HIGH, HIGH, HIGH };
int buttonState[3];
int reading[3];
int count = 0;

// Time
// int setTimeAir[2][3] = { { 7, 0, 0 }, { 16, 0, 0 } };  //Air 1
// int setTimeAir[2][3] = {{12,0,0}, {7,0,0}};  //Air 2
int setTimeAir[2][3] = {{2,0,0}, {21,0,0}};  //Air 3

// Relay
int RelayPin = 5;

// Mode
int mode = 0;

// Status
bool SystemStatus = false;  //System
bool AirStatus = false;

void setup() {
  Serial.begin(57600);
  pinMode(RelayPin, OUTPUT);

  /*********************** RTC Setting *********************/
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);  // Get Real Time clock from computer

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled) Rtc.SetDateTime(compiled);

  // LCD Setting
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  strCenter(0, "Welcome");
  strCenter(1, "RTC Auto Air");
  delay(3000);
  lcd.clear();
}

void loop() {

  RtcDateTime now = Rtc.GetDateTime();
  if (isButtonPress(0)) {
    SystemStatus = !SystemStatus;
    if (checkRangeTime(now, setTimeAir[0][0], setTimeAir[1][0])) {  //check range time
      TurnAirOn();
    }
  }
  if (isButtonPress(2) && SystemStatus == true) {
    StartRelay();
    AirStatus = !AirStatus;
  }
  if (millis() - lastTime >= 1000) {
    if ((CompareTime(now, setTimeAir[0][0], setTimeAir[0][1], setTimeAir[0][2])) && SystemStatus == true && AirStatus == false) {  // Turn On
      TurnAirOn();
    }

    if ((CompareTime(now, setTimeAir[1][0], setTimeAir[1][1], setTimeAir[1][2])) && SystemStatus == true && AirStatus == true) {  // Turn off
      TurnAirOff();
    }
    lastTime = millis();
  }
  if (isButtonPress(1)) {
    mode++;
    lcd.clear();
    if (mode > 1) {
      mode = 0;
    }
  }

  if (SystemStatus == false) {
    if (AirStatus == true) {
      StartRelay();
    }
    AirStatus = false;
  }


  if (mode == 0) {
    DisplayStatusSystem(SystemStatus);
    DisplayStatusAirMachine(AirStatus);
    printTime(1, now);
  } else if (mode == 1) {
    printDate(0, now);
  }
}

void TurnAirOn() {
  StartRelay();
  AirStatus = true;
}

void TurnAirOff() {
  StartRelay();
  AirStatus = false;
}

void StartRelay() {
  digitalWrite(RelayPin, HIGH);
  delay(100);
  digitalWrite(RelayPin, LOW);
}

void DisplayStatusAirMachine(bool status) {
  if (status) {
    lcd.setCursor(9, 0);
    lcd.print("SET ON ");
  } else {
    lcd.setCursor(9, 0);
    lcd.print("SET OFF");
  }
}

void DisplayStatusSystem(bool status) {
  if (status) {
    lcd.setCursor(0, 0);
    lcd.print("POW ON ");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("POW OFF");
  }
}


bool isButtonPress(int i) {
  int isChange = false;
  reading[i] = digitalRead(buttons[i]);
  if (reading[i] != lastButtonState[i]) {
    lastDebounceTime[i] = millis();
  }
  if ((millis() - lastDebounceTime[i]) > debounceDelay) {
    if (reading[i] != buttonState[i]) {
      buttonState[i] = reading[i];
      isChange = true;
    }
  }
  lastButtonState[i] = reading[i];
  if (isChange && !digitalRead(buttons[i])) {
    return true;
  }
  return false;
}

bool CompareTime(const RtcDateTime& dt, int hour, int minute, int second) {
  uint8_t Hour, Minute, Second;
  Hour = (int)dt.Hour();
  Minute = (int)dt.Minute();
  Second = (int)dt.Second();

  if (Hour == hour && Minute == minute && Second == second) {
    return true;
  } else {
    return false;
  }
}

void strCenter(int row, String str) {
  lcd.setCursor(8 - floor(str.length() / 2), row);
  lcd.print(str);
}

void printTime(int row, const RtcDateTime& dt) {
  hour = (int)dt.Hour();
  minute = (int)dt.Minute();
  second = (int)dt.Second();

  String time_label = "Time: ";

  //แสดงค่าเวลา
  String str = String(hour) + ":" + String(minute) + ":" + String(second);
  // Serial.println(str);

  String str_time = (String(hour).length() != 2 ? "0" + String(hour) : String(hour)) + ":" + (String(minute).length() != 2 ? "0" + String(minute) : String(minute)) + ":" + (String(second).length() != 2 ? "0" + String(second) : String(second));
  Serial.println(str_time);
  lcd.setCursor(0, row);
  lcd.print(time_label);
  lcd.setCursor(time_label.length(), row);
  lcd.print(str_time);
}

void printDate(int row, const RtcDateTime& dt) {
  year = (int)dt.Year();
  month = (int)dt.Month();
  day = (int)dt.Day();

  String date_label = "Date: ";

  //แสดงค่าวัน
  String str = String(day) + "/" + String(month) + "/" + String(year + 543);
  Serial.println(str);

  String str_date = (String(day).length() != 2 ? "0" + String(day) : String(day)) + "/" + (String(month).length() != 2 ? "0" + String(month) : String(month)) + "/" + String(year + 543);
  lcd.setCursor(0, row);
  lcd.print(date_label);
  lcd.setCursor(date_label.length(), row);
  lcd.print(str_date);
}

bool checkRangeTime(const RtcDateTime& dt, int init, int end) {
  hour = (int)dt.Hour();
  for (int i = init; i <= init + 19; i++) {
    if (hour == i % 24) {
      Serial.println(i % 24);
      return true;
    }
  }
  return false;
}