#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// 伺服馬達
Servo servo_pin_9;
boolean servoRotationRequested = false; // 用於手動模式下的旋轉控制

// pH、DS18B20水溫
int pHSense = 4;
int samples = 10;
float adc_resolution = 1023.0;


#define ONE_WIRE_BUS 3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);



void setup() {
    Serial.begin(9600);

    // 伺服馬達
    servo_pin_9.attach(9);

    // pH、DS18B20水溫
    sensors.begin();

}


void loop() {
    if (Serial.available() > 0) {
        char incomingChar = Serial.read();
        if (incomingChar == 'r') {
            // 手動模式下的旋轉控制，只有在手動模式下且未有旋轉請求時有效
            if (!servoRotationRequested) {
                servoRotationRequested = true;
                servo_pin_9.write(90); // 以手動方式旋轉伺服馬達到 90 度位置
                delay(3000);
                servo_pin_9.write(0); // 返回到0度
                servoRotationRequested = false;
                Serial.println("Feed: on");
            }
        } else {
            Serial.println("Feed: off");
        }
    }

    Serial.print("/////////////////");
    Serial.println();

    int pHReadings = 0;

    for (int i = 0; i < samples; i++) {
        pHReadings += analogRead(pHSense);
        delay(10);
    }

    float pHVoltage = 5.0 / adc_resolution * pHReadings / samples;
    float calculated_pH = ph(pHVoltage);

    Serial.print("pH: ");
    Serial.println(calculated_pH, 2);

    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);

    Serial.print("Temp: ");
    Serial.print(temperatureC);
    Serial.print(" °C ");
    Serial.println();

    delay(10000);
}

float ph(float voltage) {
    return (5.5 - (voltage - 3.5) * 1.5);
}
