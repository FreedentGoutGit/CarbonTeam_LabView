const int analogPin = A15; // Pin connected to the thermistor/voltage divider
const float referenceVoltage = 5.0; // Reference voltage of Arduino (in volts)
const float R_known = 220.0; // Known resistor value in Ohms (change if using a different resistor)
const float R_thermistor_at_25C = 100.0; // Resistance of thermistor at 25°C (change as per your thermistor specification)



void setup() {
  Serial.begin(9600);
}

void loop() {
  int sensorValue = analogRead(analogPin);
  float voltage = (sensorValue / 1023.0) * referenceVoltage;

  // Calculate the thermistor resistance
  float R_thermistor = (R_known * (referenceVoltage - voltage)) / voltage;

  // You can then convert this resistance to temperature using the thermistor's datasheet or a thermistor equation.
  // For a simple approximation, you can use the Steinhart-Hart equation or a lookup table.

  Serial.print("Resistance (Ohms): ");
  Serial.println(R_thermistor, 10);
  // Add code here to convert resistance to temperature
  float Temperature = 1.0 / (3850 * pow(10,-6)) * ((R_thermistor / 100) - 1.0);
  // float Temperature = (1.0/(1.0/298.15 + (1.0/3499.0) * log(R_thermistor/1800.0)))-273.15;
  Serial.println(Temperature, 10);


  delay(1000);
}

