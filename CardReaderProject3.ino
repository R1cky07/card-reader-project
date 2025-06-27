#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Fishino.h>
#include <ArduinoJson.h>

#ifndef __MY_NETWORK_H

#define MY_SSID "Dartmoon"
#define MY_PASS "Dartmoon2021!"  //Wifi

#endif

#ifdef IPADDR
IPAddress ip(IPADDR);
#ifdef GATEWAY
IPAddress gw(GATEWAY);
#else
IPAddress gw(ip[0], ip[1], ip[2], 1);
#endif
#ifdef NETMASK
IPAddress nm(NETMASK);
#else
IPAddress nm(255, 255, 255, 0);
#endif
#endif

FishinoClient client;

char server[] = "192.168.1.3";

unsigned long lastConnectionTime = 0;

const unsigned long postingInterval = 2L * 1000L;


MCUFRIEND_kbv tft;  //Display
const int XP = 6, YP = A1, XM = A2, YM = 7;
TouchScreen ts(XP, YP, XM, YM, 300);
const int MINPRESSURE = 10, MAXPRESSURE = 1000;

#define RST_PIN 5  //RFID reader
#define SS_PIN 53
MFRC522 mfrc522(SS_PIN, RST_PIN);

int width, height;  //Width and height of the display

#define BLACK 0x0000
#define WHITE 0xFFFF  //Color
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define ORANGE 0xFD20
#define CYAN 0x07FF
#define LIGHTGREY 0xC618
#define DARKGREY 0x7BE0
#define LIGHTBLUE 0xAFFF
#define NAVY 0x000F
#define TEAL 0x0410
#define LIME 0x07E0
#define FORESTGREEN 0x0400
#define OLIVE 0x8400
#define PINK 0xFC9F
#define MAROON 0x7800
#define YELLOW 0xFFE0
#define BROWN 0xA145
#define PURPLE 0x8010
#define MAGENTA 0xF81F

#define PORTRAIT_LEFT 114
#define PORTRAIT_RT 923
#define PORTRAIT_TOP 955
#define PORTRAIT_BOT 99


struct Button {
  int x, y, w, h;
  String label;
  uint16_t borderColor;
  void (*action)(String uid = "");
};

String dump_byte_array(byte* buffer, byte bufferSize) {
  String code = "";
  for (byte i = 0; i < bufferSize; i++) {
    if (buffer[i] < 0x10) code += "0";
    code += String(buffer[i], HEX);
  }
  code.toUpperCase();
  return code;
}

void doReplaceDevice(String& uid);
void doTestCard(String uid);
void doShowUsers(String uid);
void showUserstoRegister(String uid);
void showUsersToReplace(String UID);
void doRegisterEntry(String uid);
void doRegisterExit(String uid);
void setTFT(int atWidth, int atHeight, int textSize = 1, uint16_t textColor = WHITE, uint16_t bgcolor = BLACK);
String readCard();
void doExitAccount();


Button menuButtonsAdmin[] = {
  { 0, 0, 0, 0, "Register device", WHITE, showUsersToRegister },
  { 0, 0, 0, 0, "Replace device", BLUE, showUsersToReplace },
  { 0, 0, 0, 0, "Test card", ORANGE, doTestCard },
  { 0, 0, 0, 0, "Register Entry", GREEN, doRegisterEntry },
  { 0, 0, 0, 0, "Register Exit", YELLOW, doRegisterExit },
  { 0, 0, 0, 0, "Exit", RED, doExitAccount }
};

Button menuButtonsUsers[] = {
  { 0, 0, 0, 0, "Register Entry", GREEN, doRegisterEntry },
  { 0, 0, 0, 0, "Register Exit", YELLOW, doRegisterExit },
  { 0, 0, 0, 0, "Exit", RED, doExitAccount }
};

const int NUM_ADMIN_BUTTONS = sizeof(menuButtonsAdmin) / sizeof(menuButtonsAdmin[0]);
const int NUM_USERS_BUTTONS = sizeof(menuButtonsUsers) / sizeof(menuButtonsUsers[0]);

//String names[] = { "Luca", "Marco", "Giulia", "Anna", "Francesco", "Roberta", "Giulio" };
//String surnames[] = { "Rossi", "Bianchi", "Verdi", "Neri", "Ferrari", "Gallo", "Esposito" };

class User {
private:
  String UID, name, surname, type;
  bool isWorking;
  int workedHours;

public:
  User() {
    //name = names[random(0, 7)];
    //surname = surnames[random(0, 7)];
    UID = "";
    isWorking = false;
    workedHours = 0;
  };

  User(const User& anotherUser) {
    name = anotherUser.getName();
    surname = anotherUser.getSurname();
    UID = anotherUser.getUID();
    type = anotherUser.getType();
  }

  bool checkUID(String scannedUID) {
    return scannedUID != "" && scannedUID == UID;
  }

  void registerEntry() {
    if (!isWorking) {
      isWorking = true;
      setTFT(10, height / 2, 2, BLACK, GREEN);
      tft.print("Good job\n " + name + " " + surname);
    } else {
      setTFT(10, height / 2, 2, BLACK, GREEN);
      tft.print("You're already in");
    }
    delay(1000);
  }

  void registerExit() {
    if (isWorking) {
      isWorking = false;
      workedHours += 4;
      setTFT(10, height / 2, 2, BLACK, RED);
      tft.print("Bye bye\n " + name + " " + surname);
    } else {
      setTFT(10, height / 2, 2, BLACK, RED);
      tft.print("You're already out\n");
    }
    delay(1000);
  }

  void setInfo(String name1, String surname1) {
    name = name1;
    surname = surname1;
  }

  bool isEmpty() {
    return UID == "";
  }

  String getUID() {
    return UID;
  }

  String getName() {
    return name;
  }

  /*String getStatus() {
    String status = "";
    if (isWorking) {
      status += "Is working\n";
    } else {
      status += "Is not working\n";
    }

    status += "Worked for ";
    status += workedHours;
    status += " hours";

    return status;
  }*/

  String getTotalName() {
    return name + " " + surname;
  }
  String getSurname() {
    return surname;
  }

  String getType() {
    return type;
  }

  void setUser(String scannedUID) {
    UID = scannedUID;
    type = "User";
  }

  void setAdmin(String scannedUID) {
    UID = scannedUID;
    type = "Admin";
  }

  void set(String newUID) {
    UID = newUID;
  }
};


void setup() {
  Serial.begin(9600);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  Fishino.reset();
  Fishino.setMode(STATION_MODE);

  mfrc522.PCD_Reset();
  mfrc522.PCD_Init();


  tft.reset();
  uint16_t ID = tft.readID();
  tft.begin(ID);
  width = tft.width();
  height = tft.height();
  tft.setRotation(0);
  setTFT(5, height / 2, 1);

  Serial.print("Initializing");
  tft.print("Initializing");
  while (!Fishino.begin(MY_SSID, MY_PASS)) {
    Serial.print(".");
  }
#ifdef IPADDR
  Fishino.config(ip, gw, nm);
#else
  Fishino.staStartDHCP();
#endif

  while (Fishino.status() != STATION_GOT_IP) {
    delay(500);
    Serial << ".";
    tft.print(".");
  }
  Serial.println();
}


bool ExitFromLoop;
bool isAdminInitialized = false;
int LoopCycles = 0;
User Users[3];
const int numUsers = sizeof(Users)/sizeof(Users[0]);


void loop() {
  ExitFromLoop = false;

  String UID = "";

  if (!isAdminInitialized) {  //Check only one time if there is an admin, if there isn't register a new admin
    initializeAdmin();
    isAdminInitialized = true;
  }

  if (LoopCycles == 0) {
    setTFT(10, height / 2, 2);
    tft.println("Scan a card");
  }
  UID = readCard();
  if (UID == "") {
    LoopCycles++;
    return;
  }

  getSavedUsers(UID);
  int idx = findFromUID(UID);
  if (idx == -1) {
    UserNotFound();
    return;
  }
  adminBoot(idx);
  ShowFunctions(UID);

  waitForTouch(idx);
}


void waitForTouch(int idx) {        //This function allow my application to wait for a click when the user interface is shown
  LoopCycles = 0;
  while (!ExitFromLoop) {
    TSPoint p = ts.getPoint();
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
      int x = map(p.x, PORTRAIT_LEFT, PORTRAIT_RT, 0, width);
      int y = map(p.y, PORTRAIT_TOP, PORTRAIT_BOT, 0, height);

      if (Users[idx].getType() == "User") {
        for (int i = 0; i < NUM_USERS_BUTTONS; i++) {
          if (x >= menuButtonsUsers[i].x && x <= menuButtonsUsers[i].x + menuButtonsUsers[i].w && y >= menuButtonsUsers[i].y && y <= menuButtonsUsers[i].y + menuButtonsUsers[i].h) {
            menuButtonsUsers[i].action(Users[idx].getUID());
            return;
          }
        }
      } else {
        for (int i = 0; i < NUM_ADMIN_BUTTONS; i++) {
          if (x >= menuButtonsAdmin[i].x && x <= menuButtonsAdmin[i].x + menuButtonsAdmin[i].w && y >= menuButtonsAdmin[i].y && y <= menuButtonsAdmin[i].y + menuButtonsAdmin[i].h) {
            menuButtonsAdmin[i].action(Users[0].getUID());
            return;
          }
        }
      }
    }
  }
}

void getSavedUsers(String UID) {              //With this function, when a registered user on server scan his card, my application save him in an array
  for (int i = 1; i < numUsers; i++) {
    if (Users[i].isEmpty()) {
      if (getUserFromServer(UID) == Users[i].getTotalName()) {
        Users[i].setUser(UID);
      }
    }
  }
}

void initializeAdmin() {                    //In this function my app initialize the admin, by find if he is already registered or not
  String adminUID = RequestAdminExistance();
  if (adminUID == "null" || adminUID == "false") {
    setTFT(5, height / 2, 2);
    tft.print("Admin not found");
    delay(1000);
    registerAdminOnApp();

  } else {
    AdminAlreadyExist(adminUID);
  }
  getUserList();
}


void registerAdminOnApp() {
  setTFT(10, height / 2, 2);
  tft.print("Register the admin: ");
  String UID = readCard();
  while (UID == "") {
    UID = readCard();
  }
  registerAdmin(UID);
  Users[0].setAdmin(UID);
  setTFT(10, height / 2, 2, BLACK, GREEN);
  tft.print("Admin\n Registered");
  delay(1000);
}


void AdminAlreadyExist(String UID) {
  Users[0].setAdmin(UID);
  setTFT(5, height / 2, 2, BLACK, GREEN);
  tft.println("Admin Found");
  delay(1000);
}


int findUserFromNameSurname(String totalName) {  //An useful function to find an user from name + surname
  for (int i = 0; i < numUsers; i++) {
    if (totalName == Users[i].getTotalName()) {
      return i;
    }
  }
  return -1;
}


int findFromUID(String uid) {  //Another useful function to find a user from the UID
  for (int i = 0; i < numUsers; i++) {
    if (Users[i].getUID() == uid) {
      return i;
    }
  }
  return -1;
}


String readCard() {  //Read a new card with the RFID reader
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return "";
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return "";
  }

  String newUID = "";
  newUID = dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  return newUID;
}


void doRegisterDevice(String totalName) {  //Register a new device. This function is called from a selected user to register
  int idx = findUserFromNameSurname(totalName);

  setTFT(5, height / 2, 2);
  tft.print("Scan your new card");

  String newUID = readCard();

  while (newUID == "") {
    newUID = readCard();
  }

  for (int i = 0; i < numUsers; i++) {
    if (newUID == Users[i].getUID()) {
      setTFT(10, height / 2, 2, BLACK, RED);
      tft.print("This UID is\n already taken");
      delay(1500);
      return;
    }
  }
  registerUserOnServer(newUID, Users[idx].getName(), Users[idx].getSurname());
  Users[idx].setUser(newUID);
  setTFT(10, height / 2, 2, BLACK, GREEN);
  tft.print("New User\n " + Users[idx].getName() + " " + Users[idx].getSurname() + "\n Registered");
  delay(1500);
}


void doRegisterEntry(String uid) {  //Register a new entry for the current user
  int idx = findFromUID(uid);

  if (idx == -1) {
    return;
  }
  Users[idx].registerEntry();
  RegisterEntryOnServer(uid);
}


void doExitAccount() {  //Exit from the interface
  ExitFromLoop = true;
}


void adminBoot(int idx) {               //The boot animation when the admin access
  if (Users[idx].getType() == "Admin") {
    setTFT(5, height / 2, 3, BLACK, CYAN);
    tft.println("WELCOME ADMIN");
    delay(1000);
  }
}


void UserNotFound() {             //What to show if a user isn't found
  setTFT(10, height / 2, 2, BLACK, RED);
  tft.print("Unknown User!!!!");
  delay(1000);
}


void doRegisterExit(String uid) {  //Register a new exit from the current user
  int idx = findFromUID(uid);

  if (idx == -1) {
    return;
  }
  Users[idx].registerExit();
  RegisterExitOnServer(uid);
}


void doTestCard(String uid) {  //Test if a card work
  setTFT(width, height / 2, 2);

  tft.print("Test your card");
  int cycles = 0;
  uid = "";
  while (uid == "" && cycles < 1000) {
    uid = readCard();
    cycles++;
  }

  if (uid != "") {
    setTFT(width, height / 2, 2, BLACK, GREEN);
    tft.print("Your card works");
  } else {
    setTFT(width, height / 2, 2, BLACK, RED);
    tft.print("Your card doesn't work");
  }
  delay(1000);
}

void doReplaceDevice(String UID) {  //Replace a registered device with a new one
  int idx = findFromUID(UID);

  setTFT(5, height / 2, 2);
  tft.print("Scan your new card");

  String newUID = readCard();

  while (newUID == "") {
    newUID = readCard();
  }

  for (int i = 0; i < numUsers; i++) {
    if (i == idx) {
      continue;
    }
    if (newUID == Users[i].getUID()) {
      setTFT(10, height / 2, 2, BLACK, RED);
      tft.print("This UID is\n already taken");
      delay(1500);
      return;
    }
  }

  registerUserOnServer(newUID, Users[idx].getName(), Users[idx].getSurname());
  Users[idx].set(newUID);
  setTFT(10, height / 2, 2, BLACK, GREEN);
  tft.print("Card of " + Users[idx].getName() + " " + Users[idx].getSurname() + "\n Changed");
  delay(1500);
}


void showUsersToReplace(String uid) {  //Shows a list of registered user to replace their devices
  tft.fillScreen(BLACK);
  tft.setTextSize(2);

  Button totalUsers[numUsers + 1] = { 0, 0, 0, 0, "", GREEN, doReplaceDevice };
  int y = 10;
  String Exit = "false";

  for (int i = 0; i < numUsers + 1; i++) {
    int16_t x1, y1;
    uint16_t w, h;
    if (i == numUsers) {
      totalUsers[i].label = "Exit";
      totalUsers[i].borderColor = RED;
      totalUsers[i].action = NULL;
      tft.getTextBounds(totalUsers[i].label, 0, 0, &x1, &y1, &w, &h);
      totalUsers[i].w = w + 20;
      totalUsers[i].h = h + 10;
      totalUsers[i].x = (width - totalUsers[i].w) / 2;
      totalUsers[i].y = y;
      tft.setCursor(totalUsers[i].x + 10, totalUsers[i].y + 5);
      tft.setTextColor(WHITE);
      tft.print(totalUsers[i].label);
      tft.drawRect(totalUsers[i].x, totalUsers[i].y, totalUsers[i].w, totalUsers[i].h, totalUsers[i].borderColor);
      y += totalUsers[i].h + 15;
    } else {
      if (Users[i].isEmpty() || Users[i].getUID() == uid) {
        continue;
      }
      totalUsers[i].borderColor = GREEN;
      totalUsers[i].action = doReplaceDevice;
      totalUsers[i].label = Users[i].getName() + " " + Users[i].getSurname();
      tft.getTextBounds(totalUsers[i].label, 0, 0, &x1, &y1, &w, &h);
      totalUsers[i].w = w + 20;
      totalUsers[i].h = h + 10;
      totalUsers[i].x = (width - totalUsers[i].w) / 2;
      totalUsers[i].y = y;
      tft.setCursor(totalUsers[i].x + 10, totalUsers[i].y + 5);
      tft.setTextColor(WHITE);
      tft.print(totalUsers[i].label);
      tft.drawRect(totalUsers[i].x, totalUsers[i].y, totalUsers[i].w, totalUsers[i].h, totalUsers[i].borderColor);
      y += totalUsers[i].h + 15;
    }
  }

  while (Exit != "true") {
    TSPoint p = ts.getPoint();
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
      int x = map(p.x, PORTRAIT_LEFT, PORTRAIT_RT, 0, width);
      int y = map(p.y, PORTRAIT_TOP, PORTRAIT_BOT, 0, height);

      for (int i = 0; i < numUsers + 1; i++) {
        if (i == numUsers) {
          if (x >= totalUsers[i].x && x <= totalUsers[i].x + totalUsers[i].w && y >= totalUsers[i].y && y <= totalUsers[i].y + totalUsers[i].h) {
            Exit = "true";
            break;
          }
        }
        if (x >= totalUsers[i].x && x <= totalUsers[i].x + totalUsers[i].w && y >= totalUsers[i].y && y <= totalUsers[i].y + totalUsers[i].h) {
          totalUsers[i].action(Users[i].getUID());
          Exit = "true";
          break;
        }
      }
    }
  }
}


void showUsersToRegister(String uid) {  //Show a list of unregistered user to let them register
  tft.fillScreen(BLACK);
  tft.setTextSize(2);

  Button totalUsers[numUsers + 1] = { 0, 0, 0, 0, "", GREEN, doRegisterDevice };
  int y = 10;
  String Exit = "false";

  for (int i = 0; i < numUsers + 1; i++) {
    int16_t x1, y1;
    uint16_t w, h;
    if (i == numUsers) {
      totalUsers[i].label = "Exit";
      totalUsers[i].borderColor = RED;
      totalUsers[i].action = NULL;
      tft.getTextBounds(totalUsers[i].label, 0, 0, &x1, &y1, &w, &h);
      totalUsers[i].w = w + 20;
      totalUsers[i].h = h + 10;
      totalUsers[i].x = (width - totalUsers[i].w) / 2;
      totalUsers[i].y = y;
      tft.setCursor(totalUsers[i].x + 10, totalUsers[i].y + 5);
      tft.setTextColor(WHITE);
      tft.print(totalUsers[i].label);
      tft.drawRect(totalUsers[i].x, totalUsers[i].y, totalUsers[i].w, totalUsers[i].h, totalUsers[i].borderColor);
      y += totalUsers[i].h + 15;
    } else {
      if (Users[i].checkUID(uid) || !Users[i].isEmpty()) {
        continue;
      }
      totalUsers[i].borderColor = GREEN;
      totalUsers[i].action = doRegisterDevice;
      totalUsers[i].label = Users[i].getName() + " " + Users[i].getSurname();
      tft.getTextBounds(totalUsers[i].label, 0, 0, &x1, &y1, &w, &h);
      totalUsers[i].w = w + 20;
      totalUsers[i].h = h + 10;
      totalUsers[i].x = (width - totalUsers[i].w) / 2;
      totalUsers[i].y = y;
      tft.setCursor(totalUsers[i].x + 10, totalUsers[i].y + 5);
      tft.setTextColor(WHITE);
      tft.print(totalUsers[i].label);
      tft.drawRect(totalUsers[i].x, totalUsers[i].y, totalUsers[i].w, totalUsers[i].h, totalUsers[i].borderColor);
      y += totalUsers[i].h + 15;
    }
  }

  while (Exit != "true") {
    TSPoint p = ts.getPoint();
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
      int x = map(p.x, PORTRAIT_LEFT, PORTRAIT_RT, 0, width);
      int y = map(p.y, PORTRAIT_TOP, PORTRAIT_BOT, 0, height);

      for (int i = 0; i < numUsers + 1; i++) {
        if (i == numUsers) {
          if (x >= totalUsers[i].x && x <= totalUsers[i].x + totalUsers[i].w && y >= totalUsers[i].y && y <= totalUsers[i].y + totalUsers[i].h) {
            Exit = "true";
            break;
          }
        }
        if (x >= totalUsers[i].x && x <= totalUsers[i].x + totalUsers[i].w && y >= totalUsers[i].y && y <= totalUsers[i].y + totalUsers[i].h) {
          totalUsers[i].action(Users[i].getTotalName());
          Exit = "true";
          break;
        }
      }
    }
  }
}


void setTFT(int atWidth, int atHeight, int textSize = 1, uint16_t textColor = WHITE, uint16_t bgcolor = BLACK) {  //Set the display with width, height, text size, text color and display color
  tft.setCursor(atWidth, atHeight);
  tft.setTextSize(textSize);
  tft.setTextColor(textColor);
  tft.fillScreen(bgcolor);
}


void ShowFunctions(String uid) {  //Show the functions interface
  tft.fillScreen(BLACK);
  tft.setTextSize(2);

  int idx = findFromUID(uid);
  if (idx == -1) {
    return;
  }

  User thisUser = Users[idx];
  int y = 10;

  if (!isAdmin(uid)) {
    for (int i = 0; i < NUM_USERS_BUTTONS; i++) {
      int16_t x1, y1;
      uint16_t w, h;
      tft.getTextBounds(menuButtonsUsers[i].label, 0, 0, &x1, &y1, &w, &h);
      menuButtonsUsers[i].w = w + 20;
      menuButtonsUsers[i].h = h + 10;
      menuButtonsUsers[i].x = (width - menuButtonsUsers[i].w) / 2;
      menuButtonsUsers[i].y = y;
      tft.setCursor(menuButtonsUsers[i].x + 10, menuButtonsUsers[i].y + 5);
      tft.setTextColor(WHITE);
      tft.print(menuButtonsUsers[i].label);
      tft.drawRect(menuButtonsUsers[i].x, menuButtonsUsers[i].y, menuButtonsUsers[i].w, menuButtonsUsers[i].h, menuButtonsUsers[i].borderColor);
      y += menuButtonsUsers[i].h + 15;
    }
  }

  else {
    for (int i = 0; i < NUM_ADMIN_BUTTONS; i++) {
      int16_t x1, y1;
      uint16_t w, h;
      tft.getTextBounds(menuButtonsAdmin[i].label, 0, 0, &x1, &y1, &w, &h);
      menuButtonsAdmin[i].w = w + 20;
      menuButtonsAdmin[i].h = h + 10;
      menuButtonsAdmin[i].x = (width - menuButtonsAdmin[i].w) / 2;
      menuButtonsAdmin[i].y = y;
      tft.setCursor(menuButtonsAdmin[i].x + 10, menuButtonsAdmin[i].y + 5);
      tft.setTextColor(WHITE);
      tft.print(menuButtonsAdmin[i].label);
      tft.drawRect(menuButtonsAdmin[i].x, menuButtonsAdmin[i].y, menuButtonsAdmin[i].w, menuButtonsAdmin[i].h, menuButtonsAdmin[i].borderColor);
      y += menuButtonsAdmin[i].h + 15;
    }
  }
  tft.setTextSize(1);
  tft.setCursor(5, height - 20);
  tft.print(thisUser.getType() + " - " + thisUser.getName() + " " + thisUser.getSurname() + " - UID = " + thisUser.getUID());
}

String RequestAdminExistance() {  //This function checks if there is already a saved admin

  client.stop();

  if (client.connect(server, 8000)) {
    Serial.println("connecting...");

    client << F("GET /admin/exist HTTP/1.1\r\n");
    client << F("Host: 192.168.1.3\r\n");
    client << F("User-Agent: ArduinoWiFi/1.1\r\n");
    client << F("Connection: close\r\n");
    client.println();

  } else {
    Serial.println("connection failed");
  }

  lastConnectionTime = millis();

  String jsonMessage = "";
  bool jsonStarted = false;
  int braceCount = 0;
  unsigned long timeout = millis();

  while (millis() - timeout < 3000) {
    while (client.available()) {
      char c = client.read();


      if (c == '{') {
        if (!jsonStarted) {
          jsonStarted = true;
          jsonMessage = "";
        }
        braceCount++;
      }

      if (jsonStarted) {
        jsonMessage += c;
      }

      if (c == '}') {
        braceCount--;
        if (braceCount == 0 && jsonStarted) {
          jsonStarted = false;

          StaticJsonDocument<256> doc;
          DeserializationError error = deserializeJson(doc, jsonMessage);

          if (!error) {
            return doc["data"]["token"];
          }

          if (error) {
            Serial.print("Errore nel parsing JSON: ");
            Serial.println(error.c_str());
            return "";
          }
        }
      }
    }
  }

  Serial.println("Timeout lettura JSON");
  return "";
}

void registerAdmin(String UID) {
  client.stop();

  if (client.connect(server, 8000)) {
    Serial.println("connecting...");


    String request = "GET /admin/register?token=" + UID + " HTTP/1.1\r\n";
    client << (request);
    client << F("Host: 192.168.1.3\r\n");
    client << F("User-Agent: ArduinoWiFi/1.1\r\n");
    client << F("Connection: close\r\n");
    client.println();

  } else {
    Serial.println("connection failed");
  }

  lastConnectionTime = millis();

  String jsonMessage = "";
  bool jsonStarted = false;
  int braceCount = 0;
  unsigned long timeout = millis();

  while (millis() - timeout < 3000) {
    while (client.available()) {
      char c = client.read();


      if (c == '{') {
        if (!jsonStarted) {
          jsonStarted = true;
          jsonMessage = "";
        }
        braceCount++;
      }

      if (jsonStarted) {
        jsonMessage += c;
      }

      if (c == '}') {
        braceCount--;
        if (braceCount == 0 && jsonStarted) {
          jsonStarted = false;

          StaticJsonDocument<256> doc;
          DeserializationError error = deserializeJson(doc, jsonMessage);
        }
      }
    }
  }

  Serial.println("Timeout lettura JSON");
}

bool isAdmin(String UID) {  //Check if a determinated UID is associated to the admin
  client.stop();

  if (client.connect(server, 8000)) {
    Serial.println("connecting...");


    String request = "GET /user/token?token=" + UID + " HTTP/1.1\r\n";
    client << (request);
    client << F("Host: 192.168.1.3\r\n");
    client << F("User-Agent: ArduinoWiFi/1.1\r\n");
    client << F("Connection: close\r\n");
    client.println();

  } else {
    Serial.println("connection failed");
  }

  lastConnectionTime = millis();

  String jsonMessage = "";
  bool jsonStarted = false;
  int braceCount = 0;
  unsigned long timeout = millis();

  while (millis() - timeout < 3000) {
    while (client.available()) {
      char c = client.read();


      if (c == '{') {
        if (!jsonStarted) {
          jsonStarted = true;
          jsonMessage = "";
        }
        braceCount++;
      }

      if (jsonStarted) {
        jsonMessage += c;
      }

      if (c == '}') {
        braceCount--;
        if (braceCount == 0 && jsonStarted) {
          jsonStarted = false;

          StaticJsonDocument<256> doc;
          DeserializationError error = deserializeJson(doc, jsonMessage);
          if (!error) {
            return doc["data"]["isAdmin"];
          }
        }
      }
    }
  }

  Serial.println("Timeout lettura JSON");
}

void registerUserOnServer(String UID, String name, String surname) {  //Register an user on the server
  client.stop();

  if (client.connect(server, 8000)) {
    Serial.println("connecting...");


    String request = "GET /user/register?userToken=" + UID + "&userName=" + name + "&userSurname=" + surname + " HTTP/1.1\r\n";
    client << (request);
    client << F("Host: 192.168.1.3\r\n");
    client << F("User-Agent: ArduinoWiFi/1.1\r\n");
    client << F("Connection: close\r\n");
    client.println();

  } else {
    Serial.println("connection failed");
  }

  lastConnectionTime = millis();

  String jsonMessage = "";
  bool jsonStarted = false;
  int braceCount = 0;
  unsigned long timeout = millis();

  while (millis() - timeout < 3000) {
    while (client.available()) {
      char c = client.read();


      if (c == '{') {
        if (!jsonStarted) {
          jsonStarted = true;
          jsonMessage = "";
        }
        braceCount++;
      }

      if (jsonStarted) {
        jsonMessage += c;
      }

      if (c == '}') {
        braceCount--;
        if (braceCount == 0 && jsonStarted) {
          jsonStarted = false;

          StaticJsonDocument<256> doc;
          DeserializationError error = deserializeJson(doc, jsonMessage);
          if (!error) {
            return;
          }
        }
      }
    }
  }

  Serial.println("Timeout lettura JSON");
}

void RegisterEntryOnServer(String UID) {  //Register an entry on the server
  client.stop();

  if (client.connect(server, 8000)) {
    Serial.println("connecting...");


    String request = "GET /user/checkin?token=" + UID + " HTTP/1.1\r\n";
    client << (request);
    client << F("Host: 192.168.1.3\r\n");
    client << F("User-Agent: ArduinoWiFi/1.1\r\n");
    client << F("Connection: close\r\n");
    client.println();

  } else {
    Serial.println("connection failed");
  }

  lastConnectionTime = millis();

  String jsonMessage = "";
  bool jsonStarted = false;
  int braceCount = 0;
  unsigned long timeout = millis();

  while (millis() - timeout < 3000) {
    while (client.available()) {
      char c = client.read();


      if (c == '{') {
        if (!jsonStarted) {
          jsonStarted = true;
          jsonMessage = "";
        }
        braceCount++;
      }

      if (jsonStarted) {
        jsonMessage += c;
      }

      if (c == '}') {
        braceCount--;
        if (braceCount == 0 && jsonStarted) {
          jsonStarted = false;

          StaticJsonDocument<256> doc;
          DeserializationError error = deserializeJson(doc, jsonMessage);

          if (error) {
            Serial.print("Errore nel parsing JSON: ");
            Serial.println(error.c_str());
            return;
          }
        }
      }
    }
  }

  Serial.println("Timeout lettura JSON");
  return;
}

void RegisterExitOnServer(String UID) {  //Register an exit on the server
  client.stop();

  if (client.connect(server, 8000)) {
    Serial.println("connecting...");


    String request = "GET /user/checkout?token=" + UID + " HTTP/1.1\r\n";
    client << (request);
    client << F("Host: 192.168.1.3\r\n");
    client << F("User-Agent: ArduinoWiFi/1.1\r\n");
    client << F("Connection: close\r\n");
    client.println();

  } else {
    Serial.println("connection failed");
  }

  lastConnectionTime = millis();

  String jsonMessage = "";
  bool jsonStarted = false;
  int braceCount = 0;
  unsigned long timeout = millis();

  while (millis() - timeout < 3000) {
    while (client.available()) {
      char c = client.read();


      if (c == '{') {
        if (!jsonStarted) {
          jsonStarted = true;
          jsonMessage = "";
        }
        braceCount++;
      }

      if (jsonStarted) {
        jsonMessage += c;
      }

      if (c == '}') {
        braceCount--;
        if (braceCount == 0 && jsonStarted) {
          jsonStarted = false;

          StaticJsonDocument<256> doc;
          DeserializationError error = deserializeJson(doc, jsonMessage);

          if (error) {
            Serial.print("Errore nel parsing JSON: ");
            Serial.println(error.c_str());
            return;
          } else
            return;
        }
      }
    }
  }

  Serial.println("Timeout lettura JSON");
  return;
}

String getUserFromServer(String UID) {  //This function is used to find a reference of an user in the server
  client.stop();

  if (client.connect(server, 8000)) {
    Serial.println("connecting...");


    String request = "GET /user/get?token=" + UID + " HTTP/1.1\r\n";
    client << (request);
    client << F("Host: 192.168.1.3\r\n");
    client << F("User-Agent: ArduinoWiFi/1.1\r\n");
    client << F("Connection: close\r\n");
    client.println();

  } else {
    Serial.println("connection failed");
  }

  lastConnectionTime = millis();

  String jsonMessage = "";
  bool jsonStarted = false;
  int braceCount = 0;
  unsigned long timeout = millis();

  while (millis() - timeout < 3000) {
    while (client.available()) {
      char c = client.read();


      if (c == '{') {
        if (!jsonStarted) {
          jsonStarted = true;
          jsonMessage = "";
        }
        braceCount++;
      }

      if (jsonStarted) {
        jsonMessage += c;
      }

      if (c == '}') {
        braceCount--;
        if (braceCount == 0 && jsonStarted) {
          jsonStarted = false;

          StaticJsonDocument<256> doc;
          DeserializationError error = deserializeJson(doc, jsonMessage);

          if (!error) {
            if (doc["data"]["userToken"] == UID) {
              return doc["data"]["totalName"];
            }
          }
          if (error) {
            Serial.print("Errore nel parsing JSON: ");
            Serial.println(error.c_str());
            return "";
          }
        }
      }
    }
  }

  Serial.println("Timeout lettura JSON");
  return "";
}


void getUserList() {  //This function is used to get a list of the saved Users on the server
  client.stop();

  if (client.connect(server, 8000)) {
    Serial.println("connecting...");


    client << ("GET /user/list HTTP/1.1\r\n");
    client << F("Host: 192.168.1.3\r\n");
    client << F("User-Agent: ArduinoWiFi/1.1\r\n");
    client << F("Connection: close\r\n");
    client.println();

  } else {
    Serial.println("connection failed");
  }

  lastConnectionTime = millis();

  String jsonMessage = "";
  bool jsonStarted = false;
  int braceCount = 0;
  unsigned long timeout = millis();

  while (millis() - timeout < 3000) {
    while (client.available()) {
      char c = client.read();

      if (c == '{') {
        if (!jsonStarted) {
          jsonStarted = true;
          jsonMessage = "";
        }
        braceCount++;
      }

      if (jsonStarted) {
        jsonMessage += c;
      }

      if (c == '}') {
        braceCount--;
        if (braceCount == 0 && jsonStarted) {
          jsonStarted = false;

          DynamicJsonDocument doc(2048);
          DeserializationError error = deserializeJson(doc, jsonMessage);

          if (!error) {
            for (int i = 0; i < numUsers; i++) {
              String info = "User" + String(i + 1);
              Users[i].setInfo(doc["data"][info]["userName"], doc["data"][info]["userSurname"]);
            }
            return;
          }
          if (error) {
            Serial.print("Errore nel parsing JSON: ");
            Serial.println(error.c_str());
            return;
          }
        }
      }
    }
  }

  Serial.println("Timeout lettura JSON");
  return;
}