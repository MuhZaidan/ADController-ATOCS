#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_External_EEPROM.h>

ExternalEEPROM eeprom;

struct SensorData {
    uint8_t idSensor;
    char sensorName[20];
};

SensorData sensorData = {2, "POTENTIO"};

void setup(){
    Serial.begin(9600);
    Wire.begin();

    // Inisialisasi EEPROM
    if (eeprom.begin() == false) {
        Serial.println("EEPROM tidak terdeteksi!");
        while (true) delay(100);
    }

    // AT24C256 = 32 KB, page size 64 byte
    eeprom.setMemoryType(256);

    // Tulis konfigurasi sensor mulai alamat 0
    eeprom.put(0, sensorData);

    // Baca kembali konfigurasi
    SensorData readData = {};
    eeprom.get(0, readData);

    Serial.print("Sensor Name: ");
    Serial.println(readData.sensorName);

    Serial.print("Sensor ID: ");
    Serial.println(readData.idSensor);
}

void loop(){

}