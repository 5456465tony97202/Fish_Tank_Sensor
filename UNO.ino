#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1_bc.h>

int pHSense = 4;            // pH 傳感器連接到模擬輸入 A0
#define ONE_WIRE_BUS 3       // DS18B20 傳感器連接的數據引腳

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int samples = A1;
float adc_resolution = 1023.0;

void setup() {
    Serial.begin(9600);
    sensors.begin();
}

float ph(float voltage) {
    return (5.5 - (voltage - 3.5) * 1.5);
}

void loop() {
    int pHReadings = 0;

    for (int i = 0; i < samples; i++) {
        pHReadings += analogRead(pHSense);
        delay(10);
    }

    float pHVoltage = 5.0 / adc_resolution * pHReadings / samples;
    float calculated_pH = ph(pHVoltage);

    Serial.print("pH: "); // 添加 pH 标签
    Serial.println(calculated_pH, 2);

    // 请求温度数据
    sensors.requestTemperatures();

    // 读取温度数据
    float temperatureC = sensors.getTempCByIndex(0);

    Serial.print("Temp: ");
    Serial.print(temperatureC);
    Serial.print(" °C ");
    Serial.println();

    //int input = analogRead(A1);

    //Serial.print("WaterLevel: "); // 添加水位标签
    //double waterLevelPercentage = (input / 1023.0) * 100;
    //Serial.println(waterLevelPercentage);

    delay(10000);
}
