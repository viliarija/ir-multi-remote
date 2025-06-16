#include <SdFat.h>
#include <U8x8lib.h>
#include <SPI.h>
#include <IRremote.h>

#define BTN_UP 3
#define BTN_ENTER 7
#define BTN_DOWN 5
#define BTN_LEFT 6
#define BTN_RIGHT 4

const int ledPin = 2;
const int chipSelect = 53;

IRsend irsend(ledPin);

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8;
SdFat SD;

const int maxFolders = 8;
String path = "/";
int index = 0;
int totalFolders = 0;
int selectedIndex = 1;  // Start at index 1, which will be the first folder
uint16_t* readArray = nullptr;
uint8_t size = 0;

#define MAX_NAME_LENGTH 20
#define MAX_SCREEN_WIDTH 14

char folderNames[maxFolders][MAX_NAME_LENGTH];

// Function to read IR timings from a file on the SD card
bool readIRFromFile(const char* filePath) {
    // Open the file on the SD card
    SdFile myFile;
    if (myFile.open(filePath, O_READ)) {
        // Read array size (first byte)
        myFile.read(&size, sizeof(uint8_t));

        // Allocate memory for the array based on size
        readArray = (uint16_t *)malloc(size * sizeof(uint16_t)); // Allocate memory for timings
        if (readArray == nullptr) {
            Serial.println("Memory allocation failed!");
            myFile.close();
            return false;
        }

        // Read the raw data from the file into the allocated array
        myFile.read((byte*)readArray, size * sizeof(uint16_t));
        myFile.close();

        // Debugging: print the timings data
        Serial.print("Raw data read (size ");
        Serial.print(size);
        Serial.println("):");
        for (uint8_t i = 0; i < size; i++) {
            Serial.print(readArray[i]);
            Serial.print(' ');
        }
        Serial.println();

        return true;
    } else {
        Serial.println("Error opening file!");
        return false;
    }
}

// Function to send IR pulse using raw timings
void sendIRPulse() {
    irsend.sendRaw(readArray, size, 38);  // 38 kHz carrier frequency, adjust if needed
}

// Function to read IR file and send IR pulse based on the file path
void readAndSendIR(const char* path) {
    readArray = nullptr;
    size = 0;

    // Read IR timings from the file
    if (readIRFromFile(path)) {
        // If reading is successful, send the IR pulse
        sendIRPulse();

        // Free the allocated memory after use
        free(readArray);
    } else {
        Serial.println("Failed to read IR timings from file.");
    }
}

void clearFolderNames() {
  for (int i = 0; i < maxFolders; i++) {
    folderNames[i][0] = '\0';
  }
}

void getFolders(const char* path) {
  File root = SD.open(path);
  if (!root) {
    Serial.println("Failed to open directory.");
    return;
  }

  clearFolderNames();
  int count = 0, arrayIndex = 1;  // Start at index 1 for folder names
  totalFolders = 0;
  root.rewindDirectory();

  // Reserve the first entry for the "Return" option
  strncpy(folderNames[0], "Return", MAX_NAME_LENGTH);  // Set Return button text

  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;
      if (count >= index && arrayIndex < maxFolders) {  // Skip the first slot for "Return"
        entry.getName(folderNames[arrayIndex], MAX_NAME_LENGTH);
        arrayIndex++;
      }
      totalFolders++;
    
    count++;
    entry.close();
  }
}

void display() {
  u8x8.clear();
  
  // Display the Return button
  u8x8.setCursor(0, 0);
  if (selectedIndex == 0) u8x8.print("> Return");
  else u8x8.print("  Return");

  // Display the folder names
  for (int i = 1; i < maxFolders; i++) {  // Exclude the Return option from the folder list
    u8x8.setCursor(0, i);  // Start from row 1
    if (i == selectedIndex) u8x8.print("> ");
    else u8x8.print("  ");

    char trimmedName[MAX_SCREEN_WIDTH + 1];
    strncpy(trimmedName, folderNames[i], MAX_SCREEN_WIDTH);
    trimmedName[MAX_SCREEN_WIDTH] = '\0';

    u8x8.print(trimmedName);
  }
}

void handleInput() {
  static bool lastButtonState[5] = {false, false, false, false, false};
  static bool screenUpdated = false;

  bool buttonState[5] = {
    digitalRead(BTN_UP),
    digitalRead(BTN_ENTER),
    digitalRead(BTN_DOWN),
    digitalRead(BTN_LEFT),
    digitalRead(BTN_RIGHT)
  };

  if (buttonState[0] && !lastButtonState[0]) {  // UP button
    if (selectedIndex > 0) {  // Allow moving to Return
      selectedIndex--;
    } else if (index > 0) {
      index -= maxFolders - 1;  // Move up by a full page minus one for the Return button
      getFolders(path.c_str());
      selectedIndex = maxFolders - 1;  // Select last item in the previous page
    }
    screenUpdated = true;
  }

  if (buttonState[2] && !lastButtonState[2]) {  // DOWN button
    if (selectedIndex < maxFolders - 1) {  // Avoid going beyond the last folder
      selectedIndex++;
    } else if (index + maxFolders - 1 < totalFolders) {
      index += maxFolders - 1;  // Move down by a full page minus one for the Return button
      getFolders(path.c_str());
      selectedIndex = 1;  // Select first item on the new page
    }
    screenUpdated = true;
  }

  if (buttonState[3] && !lastButtonState[3] && index + maxFolders - 1 < totalFolders) {  // Left button
    index += maxFolders - 1;  // Move down by a full page minus one for the Return button
    selectedIndex = 1;  // Start at the first folder on the new page
    getFolders(path.c_str());
    screenUpdated = true;
  }

  if (buttonState[4] && !lastButtonState[4] && index - (maxFolders - 1) >= 0) {  // Right button
    index -= maxFolders - 1;  // Move up by a full page minus one for the Return button
    selectedIndex = 1;  // Start at the first folder on the new page
    getFolders(path.c_str());
    screenUpdated = true;
  }

  if (buttonState[1] && !lastButtonState[1]) {  // ENTER button
    if (selectedIndex == 0) {  // Return button
      path = "/";  // Go back to root directory
      index = 0;
      selectedIndex = 1;  // Start from the first folder in root
      clearFolderNames();
      getFolders(path.c_str());
      screenUpdated = true;
    } else {  // Folder selected
      if (strchr(folderNames[selectedIndex], '.') != nullptr) {
        // If the filename contains a dot
        Serial.println("Contains a dot.");
        String bin = path + String(folderNames[selectedIndex]);
        readAndSendIR(bin.c_str());
      } else {
        // If the filename doesn't contain a dot
        Serial.println("Does not contain a dot.");
        path += String(folderNames[selectedIndex]) + "/"; // Append selected folder
        index = 0; // Reset index for new directory
        selectedIndex = 1;  // Start from the first folder in the new directory
        clearFolderNames();
        getFolders(path.c_str());
        screenUpdated = true;
      }
    }
  }

  for (int i = 0; i < 5; i++) lastButtonState[i] = buttonState[i];

  if (screenUpdated) {
    display();
    screenUpdated = false;
  }
}

void setup() {
  Serial.begin(9600);
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  pinMode(BTN_UP, INPUT);
  pinMode(BTN_ENTER, INPUT);
  pinMode(BTN_DOWN, INPUT);
  pinMode(BTN_LEFT, INPUT);
  pinMode(BTN_RIGHT, INPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD Initialization failed!");
    return;
  }

  getFolders(path.c_str());
  display();
}

void loop() {
  handleInput();
}
