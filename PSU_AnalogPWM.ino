//This version uses analogWrite for PWM control.

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <LiquidCrystal_I2C.h>
#include <Bounce2.h>
#include <Encoder.h>

// Initialize the INA219 current sensor with its I2C address
Adafruit_INA219 ina219(0x40);

// Initialize the LCD display with I2C address 0x27, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);  

// Rotary encoder and button pin definitions
const int encoderPinA = 7; 
const int encoderPinB = 8; 
const int encoderButtonPin = 9; 

// PWM control pin for feedback adjustment on LM2596
const int controlPin = 10;  

// Debounce object for the encoder button
Bounce encoderButton = Bounce(); 

// Create encoder object
Encoder myEnc(encoderPinA, encoderPinB); 

// Press count for cycling through current limit presets
int pressCount = 0;

// Variables to store voltage, current, and set current limit
float lastVoltage = 0.0;
float lastCurrent_mA = 0.0; 
float currentLimit_mA = 2000.0; // Default current limit set to 2000mA (2A)

unsigned long lastDisplayTime = 0; // Timer for display updates
int pwm = 1; // Initial PWM duty cycle

// Function to display a startup message on the LCD
void showLoadingScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Powering Up.....");
  
  lcd.setCursor(0, 1);
  lcd.print(" [18.9 at 2.2A] "); // Fixed display of power source info

  delay(2000);  // Show loading screen for 2 seconds
  lcd.clear();  // Clear the screen after loading
}

void setup() {
  // Set Timer1 for fast PWM mode to increase PWM frequency
  TCCR1B = TCCR1B & B11111000 | B00000001;
  pinMode(controlPin, OUTPUT);

  Serial.begin(115200);
  while (!Serial) {
    delay(1); // Wait until serial monitor is opened
  }

  // Initialize INA219 sensor and retry if not found
  while (!ina219.begin()) {
    Serial.println(F("Failed to find INA219 chip, retrying..."));
    delay(5000);
  }

  // Initialize LCD display
  lcd.begin(16, 2); 
  lcd.backlight();  

  // Set up encoder button with debounce
  encoderButton.attach(encoderButtonPin, INPUT_PULLUP); 
  encoderButton.interval(20);

  // Set initial encoder position
  pressCount = 4;
  long initialPosition = 2000;
  myEnc.write((initialPosition / 5) * 4);

  showLoadingScreen(); // Display startup screen
}

void loop() {
  unsigned long currentMillis = millis();

  // Handle user input and current regulation
  handleEncoderInput();
  constantCurrent(); 
  measureCurrent();
  measureVoltage(); 

  // Update display every 300ms
  if (currentMillis - lastDisplayTime >= 300) {
    lastDisplayTime = currentMillis;
    displayData();
  }
}

// Function to measure voltage using INA219
void measureVoltage() {
  float shuntvoltage = ina219.getShuntVoltage_mV();
  float busvoltage = ina219.getBusVoltage_V();
  float loadvoltage = busvoltage + (shuntvoltage / 1000);

  if (loadvoltage != lastVoltage) {
    lastVoltage = loadvoltage;
  }

  Serial.print("Voltage: ");
  Serial.println(lastVoltage);
}

// Function to measure current using INA219
void measureCurrent() {
  float current_mA = ina219.getCurrent_mA();

  // Ignore small noise currents below 1.5mA
  if (current_mA < 1.5) {
    current_mA = 0.0; // Treat as zero
  }

  if (current_mA != lastCurrent_mA) {
    lastCurrent_mA = current_mA; 
  }
}

// Function to display voltage, current, and set current limit on LCD
void displayData() {
  lcd.setCursor(0, 0);
  lcd.print("V:");
  lcd.print(lastVoltage, 2); // Show voltage with 2 decimal places
  lcd.print("V,");

  lcd.print("I:");
  if (lastCurrent_mA < 10.00)
    lcd.print(lastCurrent_mA, 2); // 2 decimal places for small currents
  else if (lastCurrent_mA < 100.00)
    lcd.print(lastCurrent_mA, 1); // 1 decimal place for medium currents
  else
    lcd.print(lastCurrent_mA, 0); // No decimal places for large currents
  lcd.print("mA   ");

  lcd.setCursor(0, 1);
  lcd.print("I-Set:");
  lcd.print(currentLimit_mA, 0); // Show current limit in mA
  lcd.print("mA   ");
}

// Function to handle rotary encoder input for current limit setting
void handleEncoderInput() {
  long newPosition = myEnc.read() / 4;
  static long oldPosition = -1;

  // Adjust current limit when encoder is turned
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    currentLimit_mA = constrain(newPosition * 5, 0, 2200); // Adjust range between 0-2500mA
  }

  // Check if encoder button is pressed to cycle through preset current limits
  encoderButton.update(); 
  if (encoderButton.fell()) { 
    pressCount++;

    // Cycle through 5 preset current limits (0A, 0.5A, 1A, 1.5A, 2A)
    if (pressCount > 4) {
      pressCount = 0;
    }

    // Assign preset current limits based on press count
    switch (pressCount) {
      case 0: currentLimit_mA = 0; break;
      case 1: currentLimit_mA = 500; break;
      case 2: currentLimit_mA = 1000; break;
      case 3: currentLimit_mA = 1500; break;
      case 4: currentLimit_mA = 2000; break;
    }
    
    // Update encoder position to match selected current limit
    myEnc.write((currentLimit_mA / 5) * 4); 
  }
}

// Function to implement software-based constant current control
void constantCurrent() {
  // Read current from INA219 sensor
  float current_mA = ina219.getCurrent_mA();
  
  // Adjust PWM signal based on current limit
  if (current_mA > currentLimit_mA - 0.25) {
    pwm = constrain(pwm + 1, 0, 255); // Increase PWM to lower current
  } else {
    pwm = constrain(pwm - 1, 0, 255); // Decrease PWM to allow more current
  }

  // Apply PWM signal to LM2596 feedback pin
  analogWrite(controlPin, pwm);
}
