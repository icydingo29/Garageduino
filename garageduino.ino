#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>

#define CS_SD 4

const int parkingSpacesCount = 3;
const int lightTreshhold = 400;

int greenLEDPins[parkingSpacesCount] = {8,7,2};
int phResPins[parkingSpacesCount] = {15,16,17};
int lightValues[parkingSpacesCount];
int freeSpaces[parkingSpacesCount];


LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Serial.begin(9600);
  pinMode(10, OUTPUT);

  lcd.begin();
  lcd.setCursor(0,0);

  SPI.begin(); 
  initializeSD();

  for (int i = 0; i < parkingSpacesCount*2; i++) {
    String line = readLine(i);
  }

  for (int i = 0; i < parkingSpacesCount; i++) {
    pinMode(greenLEDPins[i], OUTPUT);
    pinMode(phResPins[i], INPUT);
  }

}

void loop() {
   for (int i = 0; i < parkingSpacesCount; i++) {
    lightValues[i] = analogRead(phResPins[i]);
  }

  for (int i = 0; i < parkingSpacesCount; i++) {
    updateLight(i);
  }
  
  printLotInfo();

  delay(2000);
}


void initializeSD() {
  Serial.println("Initializing SD card...");
  pinMode(CS_SD, OUTPUT);

  if (SD.begin(CS_SD)) {
    Serial.println("SD card is ready to use.");
  } 
  else {
    Serial.println("SD card initialization failed.");
  }
}

String readLine(int lineNumber) {
  File file = SD.open("DATA.txt", FILE_READ);
  
  int currLine = 0;

  char buff;
  while (currLine != lineNumber) {
    buff = file.read();
    if (buff == '\r' || buff == '\n') {
      buff = file.read();
      ++currLine;
    }        
  }
  
  String received = "";
  char ch;
  while (file.available()) {
    ch = file.read();
    if (ch == '\r' || ch == '\n') {
      file.close();
      return String(received);
    }
    else {
      received += ch;
    }
  }
  file.close();
  return "";
}

void updateLight(int index) {
   if (lightValues[index] > lightTreshhold) {
    digitalWrite(greenLEDPins[index], HIGH);

    String str = readLine(index);
    if (str == "not_free\r" || str == "not_free" || str == "not_free\n") {
      String lines[parkingSpacesCount*2];
 
      for (int i = 0; i < parkingSpacesCount*2; i++) {
        lines[i] = readLine(i);
      }
 
      lines[index] = "free";
      lines[parkingSpacesCount + index] = String(lines[parkingSpacesCount + index].toInt() + 1); 
 
      File writeFile = SD.open("DATA.txt", FILE_WRITE | O_TRUNC); 
      for (int i = 0; i < parkingSpacesCount*2; i++) {
        writeFile.println(lines[i]);   
      }   
 
      writeFile.close();     
    }
  }
  else { 
    digitalWrite(greenLEDPins[index], LOW);
    
    String str = readLine(index);
    if (str == "free\r" || str == "free" || str == "free\n") {
      String lines[parkingSpacesCount*2];
      for (int i = 0; i < parkingSpacesCount*2; i++) {
        lines[i] = readLine(i);
      }
 
      lines[index] = "not_free";
 
      File writeFile = SD.open("DATA.txt", FILE_WRITE | O_TRUNC); 
      for (int i = 0; i < parkingSpacesCount*2; i++) {
          writeFile.println(lines[i]);           
      }
 
      writeFile.close();   
    } 
  }
}


int getEmptySpaces() { 
  for (int i = 0; i < parkingSpacesCount; i++) {
    freeSpaces[i] = -1;
  }

  int freeSpacesIndex = 0;
  for (int i = 0; i < parkingSpacesCount; i++) {
    if (lightValues[i] > lightTreshhold) {
      freeSpaces[freeSpacesIndex] = i;
      freeSpacesIndex++;
    }
  }

  return freeSpacesIndex;
}

void printLotInfo() { 
  lcd.setCursor(0,0);
  if (getEmptySpaces() != 0){
    lcd.clear();
    lcd.print("Free spaces : ");
    lcd.print(getEmptySpaces());

    lcd.setCursor(0, 1);
    for (int i = 0; i < parkingSpacesCount; i++){
      if (freeSpaces[i] != -1) {
        lcd.print(char(65+freeSpaces[i]));    
        lcd.print(" ");    
      }
    }
  }
  else {
    lcd.clear();
    lcd.print("Parking lot is full!");
  }  
}
