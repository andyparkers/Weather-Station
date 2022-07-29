#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

const float SEA_LEVEL_PRESSURE_HPA = 1013.25;
const int BUTTON_PIN = 5;
const int TIMER_UPDATE_PERIOD = 20000;  // ms

                                        // 8 pin is VCC for Display; A0 - RX, A1 - TX for MH-Z19B;
Adafruit_BME280 bme;                    // SCL - I2C clock (A5), SDA - I2C data (A4), same wires with Display
LiquidCrystal_I2C lcd(0x27, 20, 4);     // SCL - I2C clock (A5), SDA - I2C data (A4)
SoftwareSerial mySerial(0, 1);

class WeatherStation final {
  private:
    enum Errors {
      INVALID_PPM = -777,
    };

    int32_t counter_ = 1;
    int internal_button_value_ = 0;
    byte zero_for_celcius_[8] = {0x0C, 0x12, 0x12, 0x0C, 0x00, 0x00, 0x00, 0x00};
    byte reading_command_[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    byte result_[9] = { 0 };
    byte line_[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int hour_ppm_values_[24] { 0 };
    int hour_temperature_values_[24] { 0 };
    int hour_moisture_values_[24] { 0 };
    int hour_pressure_values_[24] { 0 };
    int ppm_hour_index_ = 0;
    int temperature_hour_index_ = 0;
    int moisture_hour_index_ = 0;
    int pressure_hour_index_ = 0;
    const int max_ppm_{ 1500 }, max_temperature_{ 30 }, max_moisture_{ 65 }, max_pressure_{ 770 };
    int32_t ppm_summ_ = 0;
    int temperature_summ_ = 0;
    int moisture_summ_ = 0;
    int32_t pressure_summ_ = 0;
    uint64_t timer_ = 0;
    int button_state_ = 0;
    bool previous_state_ = 0;

    void DrawSteps(int height, int x, int y) const;

    void ClearColumn(int x) const;

    void DrawColumn(int x, int height) const;

  public:
    WeatherStation() = default;
    ~WeatherStation() = default;

    void PrintScreen(const int button_status);

    void CustomSymbolsCreation();

    int CalculatePpm();

    void PrintTips(const String& ppm, const String& temp, const String& moisture) const;

    void FirstScreen() const;

    void ShiftAndInsert(int* arr_values, int insert_val) const;

    template <typename T1>
    void CalculateAvg(int* arr_hour, T1& summ, int hour_period, int& hour_index) {
      if (counter_ % hour_period == 0) {
        int32_t for_insertion = summ / hour_period;
        if (hour_index <= 23) {
          *(arr_hour + hour_index) = for_insertion;
          summ = 0;
          ++hour_index;
        }
        else {
          ShiftAndInsert(arr_hour, for_insertion);
          summ = 0;
        }
      }
    }

    void CalculateAvgVals();

    uint64_t GetTimerValue() const;

    void UpdateTimer();

    template <typename T>
    void Print(int x, int y, T val) const {
      lcd.setCursor(x, y);
      lcd.print(val);
    }

    int UpdateButtonState(int pin);

    void DrawGraph(int* arr, const int array_size, int min_val, int max_val, const String type) const;

    int* AccessHourPpmVals();

    int* AccessHourTemperatureVals();

    int* AccessHourMoistureVals();

    int* AccessHourPressureVals();

    int GetMaxPpm() const;

    int GetMaxTemperature() const;

    int GetMaxMoisture() const;

    int GetMaxPressure() const;

    int& AccessPpmHourIndex();

    int& AccessTemperatureHourIndex();

    int& AccessMoistureHourIndex();

    int& AccessPressureHourIndex();

    int32_t& AccessPpmSumm();

    int& AccessTemperatureSumm();

    int& AccessMoistureSumm();

    int32_t& AccessPressureSumm();

    void IncreaseCounter();

    int GetCounter() const;

    int GetInternalButtonValueState() const;

    void SetInternalButtonValueState(int val);

    void SummValues();
};

WeatherStation weather_station;

void WeatherStation::DrawSteps(int height, int x, int y) const {
  lcd.setCursor(x, y);
  if (height > 0) {
    lcd.write(height - 1);
  }
}

void WeatherStation::ClearColumn(int x) const {
  for (int i = 0; i < 4; ++i) {
    lcd.setCursor(x, i);
    lcd.print(" ");
  }
}

void WeatherStation::DrawColumn(int x, int height) const {
  ClearColumn(x);
  if (height >= 0) {
    int integer = height / 8;
    int y_pos;
    for (y_pos = 3; integer > 0; --integer) {
      DrawSteps(8, x, y_pos);
      --y_pos;
    }
    DrawSteps(height % 8, x, y_pos);
  }
}

void WeatherStation::PrintScreen(const int button_status) {
  switch (button_status) {
    case 0:
      weather_station.FirstScreen();
      break;
    case 1:
      weather_station.DrawGraph(weather_station.AccessHourPpmVals(), 24, 400, 1000, "ppm");
      break;
    case 2:
      weather_station.DrawGraph(weather_station.AccessHourTemperatureVals(), 24, 20, 27, "temperature");
      break;
    case 3:
      weather_station.DrawGraph(weather_station.AccessHourMoistureVals(), 24, 25, 30, "moisture");
      break;
    case 4:
      weather_station.DrawGraph(weather_station.AccessHourPressureVals(), 24, 750, 755, "pressure");
      break;
    default:
      weather_station.FirstScreen();
      break;
  }
}

void WeatherStation::CalculateAvgVals() {
  CalculateAvg(AccessHourPpmVals(), AccessPpmSumm(), 180, AccessPpmHourIndex());
  CalculateAvg(AccessHourTemperatureVals(), AccessTemperatureSumm(), 180, AccessTemperatureHourIndex());  // period is 1 hour
  CalculateAvg(AccessHourMoistureVals(), AccessMoistureSumm(), 180, AccessMoistureHourIndex());
  CalculateAvg(AccessHourPressureVals(), AccessPressureSumm(), 180, AccessPressureHourIndex());
}

void WeatherStation::CustomSymbolsCreation() {
  for (int i = 0; i < 8; ++i) {
    line_[7 - i] = 0x1F;
    lcd.createChar(i, line_);
  }
}

int WeatherStation::CalculatePpm() {
  mySerial.write(reading_command_, 9);
  mySerial.readBytes(result_, 9);
  int ppm = (int)result_[2] * 256 + (int)result_[3];
  byte control_summ = ~(result_[1] + result_[2] + result_[3] + result_[4] + result_[5] + result_[6] + result_[7]) + 1;
  if (control_summ  == result_[8]) {
    return ppm;
  }
  return INVALID_PPM;
}

void WeatherStation::PrintTips(const String& ppm, const String& temp, const String& moisture) const {
  String high_ppm = "You should immideatelly open the window";
  String medium_ppm = "It'd be better to slighty open the window";
  String high_temperature = "It's too hot, cool the room";
  String low_temperature = "Too cold, turn on the heating";
  String high_moisture = "Too wet, dry the air";
  String low_moisture = "Turn on the humidifier, very dry air";
  String everything_is_good = "Air conditions are cool!";
  lcd.setCursor(0, 1);
  if (ppm > "1500") {
    lcd.print(high_ppm);
  }
  else if (ppm > "1100") {
    lcd.print(medium_ppm);
  }
  if (temp > "27") {
    lcd.print(high_temperature);
  }
  if (temp < "18") {
    lcd.print(low_temperature);
  }
  if (moisture > "65") {
    lcd.print(high_moisture);
  }
  else if (moisture < "35") {
    lcd.print(low_moisture);
  }
}

void WeatherStation::FirstScreen() const {
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Learn harder!");
  lcd.setCursor(8, 0);
  lcd.print("Tips:");
  lcd.setCursor(1, 2);
  String temp = (String)bme.readTemperature();
  temp.remove(4);
  lcd.print(temp + "\xDF" + "C");
  lcd.setCursor(8, 2);
  String humid = (String)bme.readHumidity();
  humid.remove(2);
  lcd.print(humid + "%");
  lcd.setCursor(12, 2);
  String ppm = (String)(CalculatePpm() == INVALID_PPM ? "---" : ppm);
  lcd.print(ppm);
  lcd.setCursor(16, 2);
  lcd.print("ppm");
  lcd.setCursor(1, 3);
  lcd.print(String(bme.readAltitude(SEA_LEVEL_PRESSURE_HPA)) + "m");
  lcd.setCursor(10, 3);
  String pressure = (String)(bme.readPressure() / 133.321F);
  pressure.remove(5);
  lcd.print(pressure + "mmHg");
}

void WeatherStation::ShiftAndInsert(int* arr_values, int insert_val) const {
  for (int i = 0; i < 23; ++i) {
    *(arr_values + i) = *(arr_values + i + 1);
  }
  *(arr_values + 23) = insert_val;
}

uint64_t WeatherStation::GetTimerValue() const {
  return timer_;
}

void WeatherStation::UpdateTimer() {
  timer_ = millis();
}

int WeatherStation::UpdateButtonState(int pin) {
  bool current_state = digitalRead(pin);
  if (current_state == 1 && current_state != previous_state_) {
    if (button_state_ == 4) {
      button_state_ = -1;
    }
    previous_state_ = 1;
    button_state_ += 1;
  }
  else if (current_state == 0 && previous_state_ == 1) {
    previous_state_ = 0;
  }
  return button_state_;
}

void WeatherStation::DrawGraph(int* arr, const int array_size, int min_val, int max_val, const String type) const {
  lcd.clear();
  int8_t shifter = 0;
  for (int i = 16; i <= 23; ++i) {
    if (*(arr + i) != 0) {
      shifter = i - 15;
    }
  }
  int maximum = max_val;
  int minimum = min_val;
  for (int i = shifter; i < 16 + shifter; ++i) {
    int for_comp = *(arr + i);
    if (for_comp > maximum) {
      maximum = for_comp;
    }
    if (for_comp != 0 && for_comp < min_val) {
      minimum = (minimum == 400 ? 400 : for_comp - (4 + for_comp / 100));
    }
  }
  Print(16, 0, maximum);
  Print(16, 3, minimum);
  if (type == "ppm") {
    Print(16, 2, " PPM");
  }
  else if (type == "temperature") {
    Print(16, 2, "TEMP");
    lcd.setCursor(18, 0);
    lcd.print("\xDF""C");
    lcd.setCursor(18, 3);
    lcd.print("\xDF""C");
  }
  else if (type == "moisture") {
    Print(16, 1, "MOIS");
    Print(16, 2, "TURE");
    lcd.setCursor(18, 0);
    lcd.print("%");
    lcd.setCursor(18, 3);
    lcd.print("%");
  }
  else if (type == "pressure") {
    Print(16, 1, "PRES");
    Print(16, 2, "SURE");
  }
  for (int i = 0; i < 16; ++i) {
    DrawColumn(i, map(*(arr + i + shifter), minimum, maximum, 0, 32));
  }
}

int* WeatherStation::AccessHourPpmVals() {
  return hour_ppm_values_;
}

int* WeatherStation::AccessHourTemperatureVals() {
  return hour_temperature_values_;
}

int* WeatherStation::AccessHourMoistureVals() {
  return hour_moisture_values_;
}

int* WeatherStation::AccessHourPressureVals() {
  return hour_pressure_values_;
}

int WeatherStation::GetMaxPpm() const {
  return max_ppm_;
}

int WeatherStation::GetMaxTemperature() const {
  return max_temperature_;
}

int WeatherStation::GetMaxMoisture() const {
  return max_moisture_;
}

int WeatherStation::GetMaxPressure() const {
  return max_pressure_;
}

int& WeatherStation::AccessPpmHourIndex() {
  return ppm_hour_index_;
}

int& WeatherStation::AccessTemperatureHourIndex() {
  return temperature_hour_index_;
}

int& WeatherStation::AccessMoistureHourIndex() {
  return moisture_hour_index_;
}

int& WeatherStation::AccessPressureHourIndex() {
  return pressure_hour_index_;
}

int32_t& WeatherStation::AccessPpmSumm() {
  return ppm_summ_;
}

int& WeatherStation::AccessTemperatureSumm() {
  return temperature_summ_;
}

int& WeatherStation::AccessMoistureSumm() {
  return moisture_summ_;
}

int32_t& WeatherStation::AccessPressureSumm() {
  return pressure_summ_;
}

void WeatherStation::IncreaseCounter() {
  ++counter_;
}

int WeatherStation::GetCounter() const {
  return counter_;
}

int WeatherStation::GetInternalButtonValueState() const {
  return internal_button_value_;
}

void WeatherStation::SetInternalButtonValueState(int val) {
  internal_button_value_ = val;
}

void WeatherStation::SummValues() {
  ppm_summ_ += CalculatePpm();
  temperature_summ_ += static_cast<int>(bme.readTemperature());
  moisture_summ_ += static_cast<int>(bme.readHumidity());
  pressure_summ_ += static_cast<int>(bme.readPressure() / 133.321);
}

void setup() {
  bme.begin();
  lcd.begin();
  mySerial.begin(9600);
  lcd.backlight();
  weather_station.CustomSymbolsCreation();
  weather_station.FirstScreen();
}

void loop() {
  int8_t button_status = weather_station.UpdateButtonState(BUTTON_PIN);
  if (weather_station.GetInternalButtonValueState() != button_status) {
    weather_station.PrintScreen(button_status);
    weather_station.SetInternalButtonValueState(button_status);
  }
  if (millis() - weather_station.GetTimerValue() >= TIMER_UPDATE_PERIOD) {
    weather_station.SummValues();
    weather_station.CalculateAvgVals();
    if (button_status == 0) {
      weather_station.PrintScreen(button_status);
    }
    weather_station.IncreaseCounter();
    weather_station.UpdateTimer();
  }
}
