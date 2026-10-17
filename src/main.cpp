#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SparkFun_External_EEPROM.h>
#include <string.h>

ExternalEEPROM eeprom;

const int rows = 4;
const int cols = 20;
LiquidCrystal_I2C lcd(0x27, cols, rows);

const String title = "A.T.O.C.S";
const String subtitle = "INDONESIA";

const int LimitOption = 4;
int currentOption = 0;

TaskHandle_t SensorHandle;
TaskHandle_t ButtonHandle;

enum SensorType {
  SENSOR_ULTRASONIC,
  SENSOR_TEMP_DS18B20,
  ANALOG_POTENTIO,
};

enum GPIO_DATA {
  PB_BACK = 15,
  PB_LEFT = 2,
  PB_RIGHT = 4,
  PB_CONFIRM = 16,
  PB_LOGIC = 17,
  Q1 = 5,
  Q2 = 18,
  Q3 = 19,
  Q4 = 23,
  POTENTIO_VALUE = 34,
  DI_0_Sensor = 27, // Input Output
  DI_1_Sensor = 32, // Input Output
  DI_2_Sensor = 33, // Input Output
  AI_0_Sensor = 34, // Input Only
  AI_1_Sensor = 35, // Input Only
  AI_2_Sensor = 36, // Input Only
};

OneWire oneWire(DI_2_Sensor);
DallasTemperature sensors(&oneWire);

enum LogicParameter {
  L_GE,
  L_GT,
  L_LE,
  L_LT,
  L_EQ,
  L_NE,
  L_VAL,
};

enum ScreenType {
  S_MAIN,
  S_SELECT_Q,
  S_LOGIC,
  S_MONITOR,
  S_MANUAL,
  S_ABOUT
};

struct SensorData {
  uint8_t idSensor;
  int sensorValue;
  char sensorName[20];
};

struct ScreenData {
  ScreenType screenType;
  int currentOption;
  int currentLogic;
};

struct LogicData {
  bool logicStatus;
  GPIO_DATA outputGPIO;
  LogicParameter logicParams[2];
  int inputValue[2];
};

struct TextData{
  String text[LimitOption];
  short pointerType; // 0, - null, 1 - <, 2 - ^
  bool clearData;
};

TextData options[] = {
  {{"Set Logic", "Monitor", "Manual", "About Us?"}, 1, true},
  {{"Q1", "Q2", "Q3", "Q4"}, 1, true},
  {{"Q#", "RANGE", "(199, 199)", "Back >"}, 2, true},
  {{"Q1 = L Q2 = L", "Q3 = L Q4 = L", "Actual = 199", "Back >"}, 0, true},
  {{"Manual", "Manual", "Manual", "Manual"}, 0, true},
  {{"Programmer - Zaidan", "Designer - Dafi", "Electro - All Team", "Back >"}, 0, true},
};

LogicData logicData[LimitOption] = {
  {false, Q1, {L_VAL, L_VAL}, {0, 0}},
  {false, Q2, {L_VAL, L_VAL}, {0, 0}},
  {false, Q3, {L_VAL, L_VAL}, {0, 0}},
  {false, Q4, {L_VAL, L_VAL}, {0, 0}},
};

int currentLogicData = 0;

volatile ScreenData screens = {S_MONITOR, 1, 0};
volatile ScreenData prevScreens = {S_MONITOR, 1, 0};
SensorData sensorData = {-1, 0, "UNDEFINED"};

void ClearText(int col, int row);
void resetClearData(TextData& textData);
void OptionText(TextData& textData, bool update);
void validateLogic(int n);
void readEEPROM_Sensor(SensorData& sensData);
void setValueSensor(SensorData& sensor, int value);

void ButtonTask(void *parameter){
  for(;;){
    if(digitalRead(PB_RIGHT) == HIGH){
      switch(options[screens.screenType].pointerType){
        case 1:
          if(screens.currentOption < LimitOption-1){
            screens.currentOption++;
            resetClearData(options[screens.screenType]);
          }
          break;
        case 2:
          if(screens.currentLogic == 7){
            screens.currentLogic = 12;
            resetClearData(options[screens.screenType]);
          }
          break;
      }
      vTaskDelay(50 / portTICK_PERIOD_MS);
      while(digitalRead(PB_RIGHT) == HIGH){
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    } 
    else if(digitalRead(PB_LEFT) == HIGH){
      switch(options[screens.screenType].pointerType){
        case 1:
          if(screens.currentOption > 0){
            screens.currentOption--;
            resetClearData(options[screens.screenType]);
          }
          break;
        case 2:
          if(screens.currentLogic == 12){
            screens.currentLogic = 7;
            resetClearData(options[screens.screenType]);
          }
          break;
      }
      
      vTaskDelay(50 / portTICK_PERIOD_MS);
      while(digitalRead(PB_LEFT) == HIGH){
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    }
    else if(digitalRead(PB_CONFIRM) == HIGH){
      if(screens.screenType == S_MAIN){
        switch(screens.currentOption){
          case 0:
            prevScreens.screenType = screens.screenType;
            prevScreens.currentOption = screens.currentOption;
            screens.screenType = S_SELECT_Q;
            screens.currentOption = 0;
            break;
          case 1:
            prevScreens.screenType = screens.screenType;
            prevScreens.currentOption = screens.currentOption;
            screens.screenType = S_MONITOR;
            screens.currentOption = 0;
            break;
          case 2:
            prevScreens.screenType = screens.screenType;
            prevScreens.currentOption = screens.currentOption;
            screens.screenType = S_MANUAL;
            screens.currentOption = 0;
            break;
          case 3:
            prevScreens.screenType = screens.screenType;
            prevScreens.currentOption = screens.currentOption;
            screens.screenType = S_ABOUT;
            screens.currentOption = 0;
            break;
        }
      }
      else if(screens.screenType == S_SELECT_Q){
        options[S_LOGIC].text[0] = "Q" + String(screens.currentOption+1);
        currentLogicData = screens.currentOption;
        prevScreens.screenType = screens.screenType;
        prevScreens.currentOption = screens.currentOption;
        currentLogicData = screens.currentOption;
        screens.screenType = S_LOGIC;
        screens.currentOption = 0;
        screens.currentLogic = 7;
      }
      resetClearData(options[screens.screenType]);
      vTaskDelay(50 / portTICK_PERIOD_MS);
      while(digitalRead(PB_CONFIRM) == HIGH){
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    }
    else if(digitalRead(PB_BACK) == HIGH){
      if(screens.screenType != S_MAIN){
        screens.screenType = prevScreens.screenType;
        screens.currentOption = prevScreens.currentOption;
        prevScreens.screenType = ScreenType(prevScreens.screenType-1);
        resetClearData(options[screens.screenType]);
      }
      vTaskDelay(50 / portTICK_PERIOD_MS);
      while(digitalRead(PB_BACK) == HIGH){
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    }
    else if(digitalRead(PB_LOGIC) == HIGH && screens.screenType == S_LOGIC){
      int logicIdx = (screens.currentLogic == 7) ? 0 : 1;
      logicData[currentLogicData].logicParams[logicIdx] = (logicData[currentLogicData].logicParams[logicIdx] == L_VAL) ? LogicParameter(0) : LogicParameter(logicData[currentLogicData].logicParams[logicIdx] + 1);
      resetClearData(options[screens.screenType]);
      vTaskDelay(50 / portTICK_PERIOD_MS);
      while(digitalRead(PB_LOGIC) == HIGH){
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void setValueSensor(SensorData& sensor, int value){
  sensor.sensorValue = value;
}

void SensorTask(void *parameter){
  bool firstTemp[sizeof(SensorType)];
  int prevSensorId = -1;
  for(;;){
    readEEPROM_Sensor(sensorData);
    switch(sensorData.idSensor){
      case SENSOR_ULTRASONIC:{
        if(sensorData.idSensor != prevSensorId){
          firstTemp[SENSOR_ULTRASONIC] = false;
          prevSensorId = SENSOR_ULTRASONIC;
        }
        if(!firstTemp[SENSOR_ULTRASONIC]){
          pinMode(DI_0_Sensor, OUTPUT);
          pinMode(DI_1_Sensor, INPUT);
          firstTemp[SENSOR_ULTRASONIC] = true;
        }

        digitalWrite(DI_0_Sensor, LOW);
        vTaskDelay(2 / portTICK_PERIOD_MS);
        digitalWrite(DI_0_Sensor, HIGH);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        digitalWrite(DI_0_Sensor, LOW);

        long duration = pulseIn(DI_1_Sensor, HIGH);
        float distance = (duration * 0.034) / 2;
        if(distance > 200) distance = 200;
        setValueSensor(sensorData, distance);
        vTaskDelay(20 / portTICK_PERIOD_MS);
        break;
      }
      case SENSOR_TEMP_DS18B20:{
        if(sensorData.idSensor != prevSensorId){
          firstTemp[SENSOR_TEMP_DS18B20] = false;
          prevSensorId = SENSOR_TEMP_DS18B20;
        }
        if(!firstTemp[SENSOR_TEMP_DS18B20]){
          sensors.begin();
          firstTemp[SENSOR_TEMP_DS18B20] = true;
        }
        sensors.requestTemperatures();
        float temperature = sensors.getTempCByIndex(0);
        setValueSensor(sensorData, int(temperature));
        vTaskDelay(1000 / portTICK_PERIOD_MS);    
        break;
      }
      case ANALOG_POTENTIO:{
        int data = map(analogRead(AI_0_Sensor), 0, 4095, 0, 160);
        setValueSensor(sensorData, data);
        vTaskDelay(20 / portTICK_PERIOD_MS);
        break;
      }
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void setup(){
  Serial.begin(9600);
  Wire.begin();

  // AT24C256 = 32 KB, page size 64 byte
  eeprom.setMemoryType(256);

  lcd.init(21, 22);
  lcd.backlight();
  lcd.setCursor(5, 1);
  lcd.print(title);
  lcd.setCursor(5, 2);
  lcd.print(subtitle);
  delay(2000);
  lcd.clear();

  pinMode(PB_BACK, INPUT);
  pinMode(PB_CONFIRM, INPUT);
  pinMode(PB_LEFT, INPUT);
  pinMode(PB_RIGHT, INPUT);
  pinMode(PB_LOGIC, INPUT);

  pinMode(Q1, OUTPUT);
  pinMode(Q2, OUTPUT);
  pinMode(Q3, OUTPUT);
  pinMode(Q4, OUTPUT);

  xTaskCreatePinnedToCore(
    ButtonTask,         // Task function
    "ButtonTask",       // Task name
    4096,             // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority
    &ButtonHandle,  // Task handle
    1                  // Core 1
  );
  xTaskCreatePinnedToCore(
    SensorTask,         // Task function
    "SensorTask",       // Task name
    4096,             // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority
    &SensorHandle,  // Task handle
    1                  // Core 1
  );
}

bool needClearText = true;

void loop(){
  if((screens.screenType != prevScreens.screenType) && options[screens.screenType].clearData != false){
    resetClearData(options[screens.screenType]);
  }

  if(screens.screenType == S_MAIN){
    OptionText(options[S_MAIN], true);
  } else if(screens.screenType == S_SELECT_Q){
    OptionText(options[S_SELECT_Q], true);
  } else if(screens.screenType == S_LOGIC){
    OptionText(options[S_LOGIC], true);
  } else if(screens.screenType == S_MONITOR){
    OptionText(options[S_MONITOR], true);
  } else if(screens.screenType == S_MANUAL){
    OptionText(options[S_MANUAL], true);
  } else if(screens.screenType == S_ABOUT){
    OptionText(options[S_ABOUT], true);
  }

  for(int i = 0; i < LimitOption; i++){
    validateLogic(i);
    delay(20);
  }

  digitalWrite(Q1, logicData[0].logicStatus);
  digitalWrite(Q2, logicData[1].logicStatus);
  digitalWrite(Q3, logicData[2].logicStatus);
  digitalWrite(Q4, logicData[3].logicStatus);

  delay(50);
}

void resetClearData(TextData& textData){
  textData.clearData = true;
}

void OptionText(TextData& textData, bool update){
  if(screens.screenType == S_MONITOR){
    String dataOutput[LimitOption];
    for(int i = 0; i < LimitOption; i++){
      dataOutput[i] = (logicData[i].logicStatus == true) ? "Q" + String(i+1) + " = H" : "Q" + String(i+1) + " = L";
    }
    textData.text[0] = String(dataOutput[0] + " " + dataOutput[1]);
    textData.text[1] = String(dataOutput[2] + " " + dataOutput[3]);
  }

  if(screens.screenType == S_LOGIC){
    String dataParameter[2];
    int dataSetting = map(analogRead(POTENTIO_VALUE), 0, 4095, 0, 160);
    for(int i = 0; i < 2; i++){
      if(logicData[currentLogicData].logicParams[i] == L_GE){
        dataParameter[i] = "LGE";
      } 
      else if(logicData[currentLogicData].logicParams[i] == L_GT){
        dataParameter[i] = "LGT";
      }
      else if(logicData[currentLogicData].logicParams[i] == L_LE){
        dataParameter[i] = "LLE";
      }
      else if(logicData[currentLogicData].logicParams[i] == L_LT){
        dataParameter[i] = "LLT";
      }
      else if(logicData[currentLogicData].logicParams[i] == L_EQ){
        dataParameter[i] = "LEQ";
      }
      else if(logicData[currentLogicData].logicParams[i] == L_NE){
        dataParameter[i] = "LNE";
      }
      else if(logicData[currentLogicData].logicParams[i] == L_VAL){
        logicData[currentLogicData].inputValue[(screens.currentLogic == 7) ? 0 : 1] = dataSetting;
        if(logicData[currentLogicData].inputValue[i] >= 0 && logicData[currentLogicData].inputValue[i] < 10) {
          dataParameter[i] = String("00" + String(logicData[currentLogicData].inputValue[i]));
        }
        else if(logicData[currentLogicData].inputValue[i] >= 10 && logicData[currentLogicData].inputValue[i] < 100) {
          dataParameter[i] = String("0" + String(logicData[currentLogicData].inputValue[i]));
        }
        else if(logicData[currentLogicData].inputValue[i] > 100 ) {
          dataParameter[i] = String(logicData[currentLogicData].inputValue[i]);
        } 
      }
    }

    textData.text[2] = "(" + dataParameter[0] + ", " + dataParameter[1] + ")";
  }

  if(textData.clearData){ 
    ClearText(-1, -1);
    delay(20);
    textData.clearData = false;
  }

  if(textData.pointerType == 1) {
    lcd.setCursor(cols - 1, screens.currentOption);
    lcd.print("<");
  } else if(textData.pointerType == 2) {
    lcd.setCursor(screens.currentLogic, rows-1);
    lcd.print("^");
  }
  for(int i = 0; i < LimitOption; i++){
    if(textData.text[i] == "Back >"){
      lcd.setCursor(cols-6, i);
      lcd.print(textData.text[i]);
    }
    else if(screens.screenType == S_LOGIC){
      if(i > 0 && i < 3){
        lcd.setCursor(5, i);
        lcd.print(textData.text[i]);
      }
      else{
        lcd.setCursor(0, i);
        lcd.print(textData.text[i]);  
      }
    }
    else if(screens.screenType == S_MONITOR){
      if(i == 2){
        lcd.setCursor(0, i);
        lcd.print(String(sensorData.sensorName));
        lcd.setCursor(8, i);
        lcd.print("[Val] = ");
        lcd.setCursor(15, i);
        lcd.print(String(sensorData.sensorValue));
      }
    }
    else {
      lcd.setCursor(0, i);
      lcd.print(textData.text[i]);
    }
  }
}

void ClearText(int col, int row){
  if(row == -1 && col == -1){
    for(int i = 0; i < rows; i++){
      lcd.setCursor(0, i);
      lcd.print("                                    ");
    }
    return;
  } else {
    lcd.setCursor(col, row);
    lcd.print("                                      ");
  }
}

void validateLogic(int n){
  if(logicData[n].logicParams[0] == L_VAL && logicData[n].logicParams[1] == L_VAL){
    int dataMin = min(logicData[n].inputValue[0], logicData[n].inputValue[1]);
    int dataMax = max(logicData[n].inputValue[0], logicData[n].inputValue[1]);
    logicData[n].logicStatus = (sensorData.sensorValue >= dataMin && sensorData.sensorValue <= dataMax) ? true : false;
  } else if(logicData[n].logicParams[0] != L_VAL && logicData[n].logicParams[1] == L_VAL){
    if(logicData[n].logicParams[0] == L_GE){
      logicData[n].logicStatus = (sensorData.sensorValue >= logicData[n].inputValue[1]) ? true : false;
    } else if(logicData[n].logicParams[0] == L_GT){
      logicData[n].logicStatus = (sensorData.sensorValue > logicData[n].inputValue[1]) ? true : false;
    } else if(logicData[n].logicParams[0] == L_LE){
      logicData[n].logicStatus = (sensorData.sensorValue <= logicData[n].inputValue[1]) ? true : false;
    } else if(logicData[n].logicParams[0] == L_LT){
      logicData[n].logicStatus = (sensorData.sensorValue < logicData[n].inputValue[1]) ? true : false;
    } else if(logicData[n].logicParams[0] == L_EQ){
      logicData[n].logicStatus = (sensorData.sensorValue == logicData[n].inputValue[1]) ? true : false;
    } else if(logicData[n].logicParams[0] == L_NE){
      logicData[n].logicStatus = (sensorData.sensorValue != logicData[n].inputValue[1]) ? true : false;
    }
  } else if(logicData[n].logicParams[0] == L_VAL && logicData[n].logicParams[1] != L_VAL){
    if(logicData[n].logicParams[1] == L_GE){
      logicData[n].logicStatus = (sensorData.sensorValue >= logicData[n].inputValue[0]) ? true : false;
    } else if(logicData[n].logicParams[1] == L_GT){
      logicData[n].logicStatus = (sensorData.sensorValue > logicData[n].inputValue[0]) ? true : false;
    } else if(logicData[n].logicParams[1] == L_LE){
      logicData[n].logicStatus = (sensorData.sensorValue <= logicData[n].inputValue[0]) ? true : false;
    } else if(logicData[n].logicParams[1] == L_LT){
      logicData[n].logicStatus = (sensorData.sensorValue < logicData[n].inputValue[0]) ? true : false;
    } else if(logicData[n].logicParams[1] == L_EQ){
      logicData[n].logicStatus = (sensorData.sensorValue == logicData[n].inputValue[0]) ? true : false;
    } else if(logicData[n].logicParams[1] == L_NE){
      logicData[n].logicStatus = (sensorData.sensorValue != logicData[n].inputValue[0]) ? true : false;
    }
  } else {
    logicData[n].logicStatus = false;
  }
}

void readEEPROM_Sensor(SensorData& sensData) {
  SensorData readData = {};
  eeprom.get(0, readData);

  readData.idSensor = sensData.idSensor;
  strcpy(sensData.sensorName, readData.sensorName);
}