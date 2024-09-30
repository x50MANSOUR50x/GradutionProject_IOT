#include <Adafruit_Fingerprint.h>

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(2, 3);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial2

#endif


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG    

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__WIFI_CLOUD

#include <WiFi.h>

// RemoteXY connection settings 
#define REMOTEXY_WIFI_SSID "Smart"
#define REMOTEXY_WIFI_PASSWORD "Stsmarttec"
#define REMOTEXY_CLOUD_SERVER "cloud.remotexy.com"
#define REMOTEXY_CLOUD_PORT 6376
#define REMOTEXY_CLOUD_TOKEN "f1aff747ae28bc322d4f94c4a6458611"


#include <RemoteXY.h>

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 36 bytes
  { 255,1,0,0,0,29,0,18,0,0,0,31,1,106,200,1,1,1,0,2,
  22,83,63,31,0,2,26,31,31,79,78,0,79,70,70,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t switch_01; // =1 if switch ON and =0 if OFF

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

#include <Keypad.h>

const int ROW_NUM = 4; //four rows const
const int COLUMN_NUM = 4; //four columns

char keys[ROW_NUM][COLUMN_NUM] = { 
  {'D', 'C', 'B', 'A'},
  {'#', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'*', '7', '4', '1'},
};

byte pin_rows[ROW_NUM] = {32, 33, 25, 26};//connect to the row pinouts 
byte pin_column[COLUMN_NUM] = {27, 14, 12, 13}; //connect to the column pinouts 
Keypad keypad(makeKeymap(keys),pin_rows,pin_column,ROW_NUM, COLUMN_NUM);

#include <LiquidCrystal_I2C.h> // Lcd library 
#include <Wire.h> // I2C communication library
LiquidCrystal_I2C Lcd(0x27, 16, 2);

#define relay 15

const String password = "2580"; // change your password here

String input_password;

int count = 0;

bool relay_switch = false;
bool fingerprint_state = false;
bool  password_state = false;

void setup() 
{
  RemoteXY_Init ();
  
  pinMode(relay, OUTPUT);
  
  input_password.reserve(8);
  Lcd.init();
Lcd.backlight();
Lcd.print("Enter Your pass");
Lcd.setCursor(0, 1);

  Serial.begin(57600); 
    //fingerprint code
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
    
}

void loop() 
{ 
  RemoteXY_Handler ();


  getFingerprintID();

  relay_switch = RemoteXY.switch_01;

char key = keypad.getKey();
if (key)
{
Serial.println(key);
if (key == '*') 
{
if (count > 0)
{ 
count--;
input_password.remove(count);
Lcd.setCursor(count, 1);
Lcd.print (" ");
} }
else if (key == '#') 
{
if (password == input_password) 
{
 password_state = true;
for (int i = 0; i < 3; i++) {
Lcd.setCursor(0, 0);
Lcd.clear();
Lcd.print("pass is correct");
delay(1000);
Lcd.setCursor(0, 0);
Lcd.clear();
 }
Lcd.print("Enter Your pass");
Lcd.setCursor(0, 1);
} 
else {
    password_state = false;
for (int i = 0; i < 3; i++) {
Lcd.setCursor(0, 0);
Lcd.clear();
Lcd.print("pass is Wrong");
delay(1000);
Lcd.setCursor(0, 0);
Lcd.clear(); }
Lcd.print("Enter Your pass");
Lcd.setCursor(0, 1); 
}
input_password = ""; 
count = 0;
}
else 
{
input_password += key; 
Lcd.setCursor(count, 1);
Lcd.print(key);
count++;
Serial.print("count = ");
Serial.println(count);
}
Serial.println(input_password);
} // end of if(key)


  if(fingerprint_state && password_state)
  {

    Lcd.setCursor(0, 0);
    Lcd.clear();
    Lcd.print("Access granted");   
    digitalWrite(relay,relay_switch);
   }

  
  // TODO you loop code
  // use the RemoteXY structure for data transfer
  // do not call delay(), use instead RemoteXY_delay() 


}



uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    fingerprint_state = true;
        Lcd.clear();
    Lcd.setCursor(0, 0);
    Lcd.print("Found");
    Lcd.setCursor(4, 1);
    Lcd.print("fingerprint");
    delay(1000);
    Lcd.clear();
Lcd.setCursor(0, 0);
Lcd.print("Enter Your pass");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    fingerprint_state = false;
    Lcd.clear();
    Lcd.setCursor(0, 0);
    Lcd.print("Access denied");
    delay(1000);
    Lcd.clear();
Lcd.setCursor(0, 0);
Lcd.print("Enter Your pass");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    fingerprint_state = false;
    Lcd.clear();
    Lcd.setCursor(0, 0);
    Lcd.print("Access denied");
    delay(1000);
    Lcd.clear();
Lcd.setCursor(0, 0);
Lcd.print("Enter Your pass");
    return p;
  } else {
    Serial.println("Unknown error");
    fingerprint_state = false;
    Lcd.clear();
    Lcd.setCursor(0, 0);
    Lcd.print("Access denied");
    delay(1000);
    Lcd.clear();
Lcd.setCursor(0, 0);
Lcd.print("Enter Your pass");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}