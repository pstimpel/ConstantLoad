float temperaturealarm = 50.0; //Switch Mosfet PWM down if higher than "temperaturealarm °C"
float voltagealarm = 25.0; //Switch Mosfet PWM down if higher than "voltage V", voltage alarm is not functional yet, but we need the value for limiting the power setting as well
float currentalarm = 0.90; //Switch Mosfet PWM down if higher than "current A"
float voltageMin = 0.005; //do not raise PWM if voltage lower than "voltageMin V"

float poweralarm = voltagealarm * currentalarm;

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_INA219.h>
#include <OneWire.h>
#include <DallasTemperature.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_INA219 ina219;

#define ONE_WIRE_BUS 8

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


#define clk 2
#define dt 3
#define sw 4
#define pwm 9
#define currentsense A0
#define voltagesense A2
char screen = 0;
char arrowpos = 0;
float power = 0;
float current = 0;

int currentStateCLK;
int previousStateCLK;
int counter = 0;
volatile int virtualPosition = 50;
volatile int lastCount = 50;
volatile boolean currentmode = false;

volatile boolean powermode = false;
volatile boolean temperaturemode = false;
volatile boolean TurnDetected = false;
volatile boolean up = false;
volatile boolean button = false;
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
volatile  float shuntvoltage = 0;
volatile  float busvoltage = 0;
volatile  float current_mA = 0;
volatile  float current_A = 0;
volatile  float loadvoltage_V = 0;
volatile  float power_mW = 0;
volatile  float power_W = 0;

volatile float temperature = -999.0;
volatile int temperaturecounter = 0;

volatile float wattarray[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
volatile float voltarray[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
volatile float amparray[5] = {0.0, 0.0, 0.0, 0.0, 0.0};

volatile bool allowignore = true;

byte customChar1[8] = {
  0b10000,
  0b11000,
  0b11100,
  0b11110,
  0b11110,
  0b11100,
  0b11000,
  0b10000
};

byte customChar2[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00000,
  0b00000,
  0b11111,
  0b01110,
  0b00100,
};

byte customChar3[8] = {
  0b00010,
  0b00110,
  0b01110,
  0b11110,
  0b11110,
  0b01110,
  0b00110,
  0b00010,
};

ISR(PCINT2_vect) {
  if (digitalRead(sw) == LOW) {
    button = true;
  }
}

void isrturn ()  {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  if (interruptTime - lastInterruptTime > 3) {
    if (digitalRead(dt) == LOW)
    {
      virtualPosition++ ; // Could be -5 or -10
    }
    else {
      virtualPosition-- ; // Could be +5 or +10
    }
  }
  lastInterruptTime = interruptTime;
}

void setup() {
  pinMode(pwm, OUTPUT);
  digitalWrite(pwm, LOW);

  Serial.begin(115200);
  pinMode(sw, INPUT_PULLUP);
  pinMode(clk, INPUT);
  pinMode(dt, INPUT);
  pinMode(currentsense, INPUT);

  ADCSRA &= ~PS_128;
  ADCSRA |= (1 << ADPS1) | (1 << ADPS0);
  PCICR |= 0b00000100;
  PCMSK2 |= 0b00010000;   // turn o PCINT20(D4)
  attachInterrupt(digitalPinToInterrupt(clk), isrturn, LOW);
  Wire.begin();
  previousStateCLK = digitalRead(clk);

  TCCR1A = 0;
  TCCR1A = (1 << COM1A1) | (1 << WGM11);
  TCCR1B = 0;
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);
  ICR1 = 2047;
  OCR1A = 0;
  Serial.println("Init Display");

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, customChar1);
  lcd.createChar(1, customChar2);
  lcd.createChar(2, customChar3);
  lcd.clear();

  Serial.println("Display done");
  Serial.println("Init 1Wire");
  sensors.begin();
  Serial.println("1Wire done");
  Serial.println("Init INA219");

  if (! ina219.begin()) {
    lcd.print(" Sensor failed");

    while (1) {
      delay(10);
    }

  }
  ina219.setCalibration_32V_1A();
  
  Serial.println("INA219 done");
  lcd.print(" ADJ CONST LOAD");
  lcd.setCursor(0, 1);
  lcd.print("ENHANCED VERSION");

  delay(1000);
  screen0();
  lcd.setCursor(0, 0);
  lcd.write((uint8_t)0);
  Serial.println("Setup done");
}

void loop() {
  delayMicroseconds(1000);

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  //power_mW = ina219.getPower_mW();
  //power_W = power_mW / 1000.0;
  current_A = current_mA / 1000.0;

  bool zeroedvales=false;
  loadvoltage_V = busvoltage + (shuntvoltage / 1000);
  if (loadvoltage_V < 0.0 || current_A < 0.0) {
    /*
    Serial.print("zeroing values ");
    Serial.print(loadvoltage_V);
    Serial.print(" V ");
    Serial.print(current_A);
    Serial.println(" A");
    */
    loadvoltage_V = 0.0;
    current_A = 0.0;
    zeroedvales = true;
    
  }
  //if (shuntvoltage < 0.05) {
  //  loadvoltage_V = 0.0;
  //  Serial.println("zeroing shuntvoltage");
  //}
  /*
  Serial.print("A ");
  Serial.print(current_A);
  Serial.print(" V ");
  Serial.print(loadvoltage_V);
  Serial.print(" W ");
  Serial.print(power_W);
  */
  power_W = loadvoltage_V * current_A;

  bool doprocess=true;
  
  if(zeroedvales == true && allowignore == true) {
    allowignore = false;
    doprocess = false;
    //Serial.print(" forced ignore ");
  } else if (zeroedvales == false) {
    allowignore=true;
  }

  if(doprocess == true) {
    
    //build averages
    wattarray[0] = wattarray[1];
    wattarray[1] = wattarray[2];
    wattarray[2] = wattarray[3];
    wattarray[3] = wattarray[4];
    wattarray[4] = power_W;
  
    power_W = (wattarray[0] + wattarray[1] + wattarray[2] + wattarray[3] + wattarray[4]) / 5;
  
    voltarray[0] = voltarray[1];
    voltarray[1] = voltarray[2];
    voltarray[2] = voltarray[3];
    voltarray[3] = voltarray[4];
    voltarray[4] = loadvoltage_V;
  
    loadvoltage_V = (voltarray[0] + voltarray[1] + voltarray[2] + voltarray[3] + voltarray[4]) / 5;
  
    amparray[0] = amparray[1];
    amparray[1] = amparray[2];
    amparray[2] = amparray[3];
    amparray[3] = amparray[4];
    amparray[4] = current_A;
  
    current_A = (amparray[0] + amparray[1] + amparray[2] + amparray[3] + amparray[4]) / 5;
    /*
    Serial.print(" As ");
    Serial.print(current_A);
    Serial.print(" Vs ");
    Serial.print(loadvoltage_V);
    Serial.print(" Ws ");
    Serial.println(power_W);
    */
  }

  temperaturecounter++;
  if (screen != 7 && screen != 6 && screen != 3) {
    if (temperaturecounter == 100) {
      lcd.setCursor(15, 1);
      lcd.print(".");
    } else if (temperaturecounter == 200) {
      lcd.setCursor(15, 1);
      lcd.print(" ");
    }
  }
  if (temperaturecounter > 200) {
    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);
    temperaturecounter = 0;
    /*
      Serial.print("C ");
      Serial.println(temperature);
      Serial.print("A ");
      Serial.println(current_A);
      Serial.print("V ");
      Serial.println(loadvoltage_V);
      Serial.print("SV ");
      Serial.println((shuntvoltage / 1000));
      Serial.print("BV ");
      Serial.println(busvoltage);
      Serial.print("W ");
      Serial.println(power_W);
      Serial.print("OCR1A ");
      Serial.println(OCR1A);
    */
  }




  if ((currentalarm < current_A && loadvoltage_V > 0.05) || currentalarm < current_A) {
    OCR1A = 0;
    screen10();
    screen = 10;
    Serial.print("ocr1a ");
    Serial.println(OCR1A);
    Serial.println("Operation stopped due current, waiting for switch off");
    while (1) {
      delay(10);
    }
  }
  /*
    if ((voltagealarm < loadvoltage_V && current_A > 0.2) || 26.0 < loadvoltage_V) {
    OCR1A = 0;
    screen9();
    screen = 9;
    Serial.print("ocr1a ");
    Serial.println(OCR1A);
    Serial.println("Operation stopped due voltage, waiting for switch off");
    while (1) {
      delay(10);
    }
    }
  */
  if (temperaturealarm < temperature) {
    OCR1A = 0;
    screen8();
    screen = 8;
    Serial.print("ocr1a ");
    Serial.println(OCR1A);
    Serial.println("Operation stopped due temperature, waiting for switch off");
    while (1) {
      delay(10);
    }
  }

  if (temperaturemode) {
    if (counter == 200) {
      lcd.setCursor(6, 0);
      lcd.print(temperature);
      lcd.print("C   ");
      counter = 0;
    }
    counter++;
  }

  if (currentmode) {
    /*
      Serial.print("A  ");
      Serial.println(current_A);
      Serial.print("V  ");
      Serial.println(loadvoltage_V);
      Serial.print("W  ");
      Serial.println(power_W);
    */
    if (counter == 100) {
      lcd.setCursor(4, 0);
      lcd.print(current_A);
      lcd.print("A@");
      lcd.print(loadvoltage_V);
      lcd.print("V");
      counter = 0;
    }
    if (current_A < current && loadvoltage_V > voltageMin) {
      if(current_A < 0.1 && OCR1A < 2000) {
        OCR1A = OCR1A + 10;
      } else {
        OCR1A = OCR1A + 1;
      }
    }
    else {
      if (OCR1A > 0) {
        OCR1A = OCR1A - 1;
      }
    }
    if (OCR1A < 0) {
      OCR1A = 0;
    }
    Serial.print("ocr1a ");
    Serial.println(OCR1A);
    counter++;
    delayMicroseconds(100);
  }

  if (powermode) {
    /*
        Serial.print("A  ");
        Serial.println(current_A);
        Serial.print("V  ");
        Serial.println(loadvoltage_V);
        Serial.print("W  ");
        Serial.println(power_W);
    */
    if (counter == 100) {
      lcd.setCursor(4, 0);
      lcd.print(power_W);
      lcd.print("W@");
      lcd.print(loadvoltage_V);
      lcd.print("V");
      counter = 0;
    }
    if (power_W < power && loadvoltage_V > voltageMin) {
      if(power_W < 0.1 && OCR1A < 2000) {
        OCR1A = OCR1A + 10;
      } else {
        OCR1A = OCR1A + 1;
      }
    }
    else {
      if (OCR1A > 0) {
        OCR1A = OCR1A - 1;
      } else {
        OCR1A = 0;
      }
    }
    if (OCR1A < 0) {
      OCR1A = 0;
    }
    Serial.print("ocr1a ");
    Serial.println(OCR1A);
    counter++;
    delayMicroseconds(100);
  }

  if (virtualPosition != lastCount) {

    if (virtualPosition > lastCount) {
      //UP
      TurnDetected = true;
      up = true;
    } else {
      //DOWN
      TurnDetected = true;
      up = false;
    }

    lastCount = virtualPosition ;

  }

  if (TurnDetected) {
    //delay(50);
    switch (screen) {
      case 0:
        switch (arrowpos) {
          case 0:
            if (!up) {
              screen0();
              lcd.setCursor(7, 0);
              lcd.write((uint8_t)0);
              arrowpos = 1;
            }
            break;
          case 1:
            if (!up) {
              screen0();
              lcd.setCursor(0, 1);
              lcd.write((uint8_t)0);
              arrowpos = 2;
            } else {
              screen0();
              lcd.setCursor(0, 0);
              lcd.write((uint8_t)0);
              arrowpos = 0;
            }
            break;
          case 2:
            if (up) {
              screen0();
              lcd.setCursor(7, 0);
              lcd.write((uint8_t)0);
              arrowpos = 1;
            }
            break;
        }
        break;
      case 1:
        switch (arrowpos) {
          case 0:
            if (!up) {
              screen1();
              lcd.setCursor(0, 1);
              lcd.write((uint8_t)0);
              arrowpos = 1;
            }
            break;
          case 1:
            if (up) {
              screen1();
              lcd.setCursor(0, 0);
              lcd.write((uint8_t)0);
              arrowpos = 0;
            }
            else {
              screen1();
              lcd.setCursor(7, 1);
              lcd.write((uint8_t)0);
              arrowpos = 2;
            }
            break;
          case 2:
            if (up) {
              screen1();
              lcd.setCursor(0, 1);
              lcd.write((uint8_t)0);
              arrowpos = 1;
            }
            break;
        }
        break;
      case 2:
        if (up) {
          power = power - 0.1;
          if (power < 0) {
            power = 0;
          }
          lcd.setCursor(7, 0);

          lcd.print(power);
          lcd.print("W");
          lcd.write((uint8_t)1);
          lcd.print("  ");
        }
        else {
          power = power + 0.1;
          if (power > poweralarm) {
            power = poweralarm;
          }
          lcd.setCursor(7, 0);

          lcd.print(power);
          lcd.print("W");
          lcd.write((uint8_t)1);
          lcd.print("  ");
        }
        break;
      case 4:
        switch (arrowpos) {
          case 0:
            if (!up) {
              screen4();
              lcd.setCursor(0, 1);
              lcd.write((uint8_t)0);
              arrowpos = 1;
            }
            break;
          case 1:
            if (up) {
              screen4();
              lcd.setCursor(0, 0);
              lcd.write((uint8_t)0);
              arrowpos = 0;
            }
            else {
              screen4();
              lcd.setCursor(7, 1);
              lcd.write((uint8_t)0);
              arrowpos = 2;
            }
            break;
          case 2:
            if (up) {
              screen4();
              lcd.setCursor(0, 1);
              lcd.write((uint8_t)0);
              arrowpos = 1;
            }
            break;
        }
        break;
      case 5:
        if (up) {
          current = current - 0.05;
          if (current < 0) {
            current = 0;
          }
          lcd.setCursor(9, 0);
          lcd.print(current);
          lcd.print("A");
          lcd.write((uint8_t)1);
          lcd.print(" ");
        }
        else {
          current = current + 0.05;
          if (current > currentalarm) {
            current = currentalarm;
          }
          lcd.setCursor(9, 0);
          lcd.print(current);
          lcd.print("A");
          lcd.write((uint8_t)1);
          lcd.print(" ");
        }
        break;
    }
    TurnDetected = false;

  }

  if (button) {
    delay(100);
    switch (screen) {
      case 0:
        if (arrowpos == 0) {
          screen = 1;
          screen1();
          lcd.setCursor(0, 0);
          lcd.write((uint8_t)0);
        } else if (arrowpos == 1) {
          screen = 4;
          screen4();
          lcd.setCursor(0, 0);
          lcd.write((uint8_t)0);
        }
        else {
          temperaturemode = true;
          screen = 7;
          screen7();
          lcd.setCursor(11, 1);
          lcd.write((uint8_t)0);
        }
        break;
      case 1:
        switch (arrowpos) {
          case 0:
            screen = 2;
            screen2();
            break;
          case 1:
            if (power > 0.0) {
              powermode = true;
              OCR1A = 0;
              screen = 3;
              screen3();
            } else {

              screen = 2;
              screen2();
            }
            break;
          case 2:
            screen = 0;
            screen0();
            lcd.setCursor(0, 0);
            lcd.write((uint8_t)0);
            break;
        }
        break;
      case 2:
        screen = 1;
        screen1();
        lcd.setCursor(0, 0);
        lcd.write((uint8_t)0);
        break;
      case 3:
        powermode = false;
        OCR1A = 0;
        counter = 0;
        screen = 1;
        screen1();
        lcd.setCursor(0, 0);
        lcd.write((uint8_t)0);
        break;
      case 4:
        switch (arrowpos) {
          case 0:
            screen = 5;
            screen5();
            break;
          case 1:
            if (current > 0.0) {
              OCR1A = 0;
              screen = 6;
              screen6();
              currentmode = true;
              counter = 0;
            } else {
              screen = 5;
              screen5();

            }
            break;
          case 2:
            screen = 0;
            screen0();
            lcd.setCursor(0, 0);
            lcd.write((uint8_t)0);
            break;
        }
        break;
      case 5:
        screen = 4;
        screen4();
        lcd.setCursor(0, 0);
        lcd.write((uint8_t)0);
        break;
      case 6:
        screen = 4;
        screen4();
        lcd.setCursor(0, 0);
        lcd.write((uint8_t)0);
        currentmode = false;
        OCR1A = 0;
        break;
      case 7:
        temperaturemode = false;
        screen = 0;
        screen0();
        lcd.setCursor(0, 0);
        lcd.write((uint8_t)0);
        break;
    }
    arrowpos = 0;
    button = false;
  }
}

void screen0() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Power  ");
  lcd.print("Current");
  lcd.setCursor(1, 1);
  lcd.print("Temperature");

}

void screen1() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Power:");
  lcd.print(power);
  lcd.print("W");
  lcd.setCursor(1, 1);
  lcd.print("Start");
  lcd.setCursor(8, 1);
  lcd.print("Back");
}

void screen2() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write((uint8_t)2);
  lcd.print("Power:");
  lcd.print(power);
  lcd.print("W");
  lcd.write((uint8_t)1);
}

void screen3() {
  lcd.clear();
  lcd.print("Cur:");
  lcd.print(power_W);
  lcd.print("W");
  lcd.setCursor(0, 1);
  lcd.print("Set:");
  lcd.print(power);
  lcd.print("W");
  lcd.setCursor(11, 1);
  lcd.write((uint8_t)0);
  lcd.print("STOP");
}

void screen4() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Current:");
  lcd.print(current);
  lcd.print("A");
  lcd.setCursor(1, 1);
  lcd.print("Start");
  lcd.setCursor(8, 1);
  lcd.print("Back");
}

void screen5() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write((uint8_t)2);
  lcd.print("Current:");
  lcd.print(current);
  lcd.print("A");
  lcd.write((uint8_t)1);
}

void screen6() {
  lcd.clear();
  lcd.print("Cur:");
  lcd.print(current_A);
  lcd.print("A");
  lcd.setCursor(0, 1);
  lcd.print("Set:");
  lcd.print(current);
  lcd.print("A");
  lcd.setCursor(11, 1);
  lcd.write((uint8_t)0);
  lcd.print("STOP");
}

void screen7() {
  lcd.clear();
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("< ");
  lcd.print(temperaturealarm);
  lcd.print("C");
  lcd.setCursor(11, 1);
  lcd.write((uint8_t)0);
  lcd.print("Back");
}

void screen8() {
  lcd.clear();
  lcd.print("!! TEMP ALARM !! ");
  lcd.setCursor(0, 1);
  lcd.print("Shutdown! ");
  lcd.print(temperature);
  lcd.print("C");

}

void screen9() {
  lcd.clear();
  lcd.print("OV Warn ");
  lcd.print(OCR1A);
  lcd.setCursor(0, 1);
  lcd.print("Shutdown! ");
  lcd.print(loadvoltage_V);
  lcd.print("V");

}

void screen10() {
  lcd.clear();
  lcd.print("OC Warn ");
  lcd.print(OCR1A);
  lcd.setCursor(0, 1);
  lcd.print("Shutdown! ");
  lcd.print(current_A);
  lcd.print("A");

}
