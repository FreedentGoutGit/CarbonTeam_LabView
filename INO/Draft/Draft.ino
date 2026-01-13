// this code receives messages from the Serial monitor of the form N1, N2, N3
// and communicate with 3 DACs (via I²C) to set values of voltages
// respectively for Temperature (a resistor), a fan, and 'valves' via a servo motor.

#include <DFRobot_GP8403.h>
#include <SPI.h>

DFRobot_GP8403 dac1_T(&Wire, 0X5B);  // First DAC instance
DFRobot_GP8403 dac2_T(&Wire, 0x5E);  // First DAC instance

void setup() {
  Serial.begin(115200);
  Serial.println("start");
  while (dac1_T.begin() != 0) {
    Serial.println("Init error");
    delay(1000);
  }

  while (dac2_T.begin() != 0) {
    Serial.println("Init error");
    delay(1000);
  }

  Serial.println("Init succeed: Enter three numbers separated by blank spaces");

  dac1_T.setDACOutRange(dac1_T.eOutputRange10V); // Set the output range as 0-10V for the first DAC
  dac2_T.setDACOutRange(dac2_T.eOutputRange10V); // Set the output range as 0-10V for the first DAC

  analogReference(EXTERNAL); // Set the reference voltage to 


  delay(1000);
}

void loop() {
  if (Serial.available() > 0) {
    // Read the input from the Serial Monitor
    String input = Serial.readStringUntil('\n');
    input.trim(); // Remove any leading/trailing whitespace

    // Split the input into three parts based on commas
    int firstCommaIndex = input.indexOf(',');
    int secondCommaIndex = input.indexOf(',', firstCommaIndex + 1);

    // Extract the three values from the input string
    String value1Str = input.substring(0, firstCommaIndex);
    String value2Str = input.substring(firstCommaIndex + 1, secondCommaIndex);
    String value3Str = input.substring(secondCommaIndex + 1);

    // Convert the extracted strings to integers
    int value1 = value1Str.toInt();
    int value2 = value2Str.toInt();
    int value3 = value3Str.toInt();















    // Parse the two numbers from the input
    //int value1 = input.toInt();   // Value for the first DAC -- resistor

    // Set DAC1 output voltage
    dac1_T.setDACOutVoltage(value1, 0);
    dac1_T.setDACOutVoltage(value2, 1);

    dac2_T.setDACOutVoltage(value1, 0);
    dac2_T.setDACOutVoltage(value1, 1);




    // Print feedback to Serial Monitor for the first DAC
    Serial.println("Set DAC1 output voltage to V on OUT1 channel");
    delay(1000);
  }

  // Read the voltage from the analog input A1
  int rawValue = analogRead(A1);
  Serial.print("cQ_1 ");
  Serial.println(rawValue);
  // Calculate the actual voltage (0-10V range) using the voltage divider ratio
  float actualVoltage = (rawValue * 5.0)/1023.0 ; // Assuming a voltage divider with R1 = R2

  // Print the actual voltage to the serial monitor
  Serial.print("Voltage: ");
  Serial.print(actualVoltage, 2); // Print with 2 decimal places
  Serial.println("V");
  delay(1000); // Optional delay for reading rate

}