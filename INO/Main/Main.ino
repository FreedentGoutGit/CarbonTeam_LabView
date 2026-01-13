
  #include <Arduino.h>
  #include <DFRobot_GP8403.h>
  #include <SPI.h>
  #include <Wire.h>

  //Librarie Parse se trouves ici: Y:\AECH Libraries\Arduino\libraries\Parse_SCPI
  #ifndef PARSE_SCPI_H
  #include <Parse_SCPI.h>
  #define PARSE_SCPI_H
  #endif
  
  //************************ DEFINE *********************************
  #define N_SYS_MODE 3
  const char *sys_mode_list[N_SYS_MODE] = {"OFF","NORM","TEST"};
  #define N_CHAN_MODE 3
  const char *chan_mode_list[N_SYS_MODE] = {"OFF","NORM","TEST"};

  
  DFRobot_GP8403 dac1_T(&Wire, 0x58);  // First DAC instance
  DFRobot_GP8403 dac2_T(&Wire, 0x5E);  // Second DAC instance
  
  const int analogPin = A9; // Pin connected to the thermistor/voltage divider
  const int analogPin2 = A1; // Pin connected to the thermistor/voltage divider
  const int analogPin3 = A8; // Pin connected to the thermistor/voltage divider
  const int analogPin4 = A0; //Pin connected to the MFS_out
  const float referenceVoltage = 5; // Reference voltage of Arduino (in volts)
  const float referenceVoltageVPX = 10; // Reference voltage of Senso VPX (in volts)
  const float R_known = 100.0; // Known resistor value in Ohms (change if using a different resistor)
  const float R_thermistor_at_25C = 109.74; // Resistance of thermistor at 25°C (change as per your thermistor specification)
  const float MF_UPPER_RANGE = 1000;
  const float MF_LOWER_RANGE = 10;
  const char message_GFS[20] = "r";
  const float MFS_offset = 1.0;         //offset to convert the voltage from the MFS_out
  const float MFS_coefficient = 0.2;    //coefficient to convert the voltage from the MFS_out

  //******************FUNCTION DECLARATIONS**************************
  void SERVO_DEG_IN();
  void SERVO_DEG_OUT_1();
  void SERVO_DEG_OUT_2();
  void FAN_OMEGA();
  void CTRL_TEMP_1();
  void CTRL_TEMP_2();
  void CTRL_MASS_FLOW_IN();
  void CTRL_MASS_FLOW_OUT();
  void CTRL_CO2();
  //************************ VARIABLES ******************************

  Parse_SCPI scpi;  

  int sys_mode = 0;
  int chan_mode[10] = {0,0,0,0,0,0,0,0,0,0};

  char reply[SIZE_COMMAND];
  char floatStr[16];
  char check[16];

  //************************************************************
  //********************* SETUP ********************************
  //************************************************************
  void setup() 
  {
    
    Serial.begin(9600);
    Wire.begin();
    Wire.setClock(100000);  // Extend communication distance of I2C
    pinMode(analogPin4, INPUT);  //Set analogPin4 as the input for the MFS_out
    //SCPI
    //la reponse au *IDN? se fait automatiquement avec la chaine marqué ici.  
    scpi.Set_IDN("EPFL SB ISIC-GE AECH Parse Model");
    //Marque ici les commandes et le fonction qui devrait être appelé par la commande
    
    scpi.Register_command(":SERVO:DEG:IN",&SERVO_DEG_IN);
    scpi.Register_command(":SERVO:DEG:OUT:1",&SERVO_DEG_OUT_1);
    scpi.Register_command(":SERVO:DEG:OUT:2",&SERVO_DEG_OUT_2);
    scpi.Register_command(":FAN:OMEGA",&FAN_OMEGA);
    scpi.Register_command(":CTRL:TEMP:1",&CTRL_TEMP_1);
    scpi.Register_command(":CTRL:TEMP:2",&CTRL_TEMP_2);
    scpi.Register_command(":CTRL:MASS:FLOW:IN",&CTRL_MASS_FLOW_IN); //CUrrent range 10-1000 m^3/h
    scpi.Register_command(":CTRL:MASS:FLOW:OUT",&CTRL_MASS_FLOW_OUT);
    scpi.Register_command(":CTRL:CO2",&CTRL_CO2);
    Serial.println("SCPI ok");

    //DAC
    
    while (dac1_T.begin() != 0) {
    Serial.println("Init error");
    delay(1000);
    }
    while (dac2_T.begin() != 0) {
      Serial.println("Init error");
      delay(1000);
    }

    dac1_T.setDACOutRange(dac1_T.eOutputRange10V); // Set the output range as 0-10V for the first DAC
    dac2_T.setDACOutRange(dac2_T.eOutputRange10V); // Set the output range as 0-10V for the first DAC
    Serial.println("DAC ok");
    
    delay(1000);
  }
  
  //************************************************************
  //********************* LOOP *********************************
  //************************************************************
  void loop() 
  {
    if(scpi.Command_received())
    {
      scpi.Execute_command();
    }
    delay(10);
  }

//************************************************************
//********************* FUNCTION *****************************
//************************************************************

//// DAC
int conversion_DAC (float _setPoint)
{
  int setPoint = int(_setPoint);
  if (setPoint > 90) {
    Serial.println("Error: Outside of possible range");
    return 5000;
  }
  int modif_setPoint = map(setPoint, 0, 90, 0, 10000);
  return modif_setPoint;
}

void CTRL_TEMP_1()
{ 
  // CTRL_TEMP read temperature from analog sensor
  float sensorValue = analogRead(analogPin);
  float voltage = (sensorValue / 1023.0) * referenceVoltage;
  // Calculate the thermistor resistance
  float R_thermistor = (R_known * (referenceVoltage - voltage)) / voltage;
  // Add code here to convert resistance to temperature
  float Temperature = 1.0 / (3850 * pow(10,-6)) * ((R_thermistor / 100) - 1.0) - 8; // -8 Is cause we can't get the temp sensors to work well with the formula 
  dtostrf(Temperature, 2, 2, floatStr);
  sprintf(reply,":CTRL:TEMP:1 %s \r\n",floatStr);
  scpi.Reply(reply);
}

void CTRL_TEMP_2()
{ 
  // CTRL_TEMP read temperature from analog sensor
  float sensorValue = analogRead(analogPin2);
  float voltage = (sensorValue / 1023.0) * referenceVoltage;

  // Calculate the thermistor resistance
  float R_thermistor = (R_known * (referenceVoltage - voltage)) / voltage;
  // Add code here to convert resistance to temperature
  float Temperature = 1.0 / (3850 * pow(10,-6)) * ((R_thermistor / 100) - 1.0) - 8; // -8 Is cause we can't get the temp sensors to work well with the formula 
  dtostrf(Temperature, 2, 2, floatStr);
  sprintf(reply,":CTRL:TEMP:2 %s \r\n",floatStr);
  scpi.Reply(reply);
}

void CTRL_MASS_FLOW_IN()
{ 
  // CTRL_TEMP read temperature from analog sensor
  float sensorValue = analogRead(analogPin3);
  float voltage = (sensorValue / 1023.0) * referenceVoltageVPX;
  float Massflow = map(voltage, 0, 10, 0, MF_UPPER_RANGE - MF_LOWER_RANGE);
  float MassFlow = MF_LOWER_RANGE + Massflow;
  dtostrf(MassFlow, 4, 4, floatStr);
  sprintf(reply,":CTRL:MASS:FLOW:IN %s \r\n", floatStr);
  scpi.Reply(reply);
}

void CTRL_MASS_FLOW_OUT()
{ 
  // CTRL_TEMP read temperature from analog sensor
  float sensorValue = (float) analogRead(analogPin4);
  float voltage = (sensorValue / 1023.0);
  //float MassFlow = map(voltage, 0, 5, -5, 20); //see if it work, if not use the next line
  float MassFlow = (voltage - MFS_offset) / MFS_coefficient;
  dtostrf(MassFlow, 4, 4, floatStr);
  sprintf(reply,":CTRL:MASS:FLOW:OUT %s \r\n", floatStr);
  scpi.Reply(reply);
}

void CTRL_CO2()
{ 
  Wire.beginTransmission(100);
  Wire.write(message_GFS);
  Wire.endTransmission();

  delay(600);

  Wire.requestFrom(100, 16, 1);
  byte state = Wire.read();
  byte valueRead;

  switch(state) {
    case 1:
      int len = Wire.available();
      for (int i=0; i<len; i++) {
        valueRead = Wire.read();
        floatStr[i] = valueRead;
      }
      sprintf(reply,":CTRL:CO2 %s \r\n", floatStr);
      break;
    case 2:
      strcpy(reply, ":CTRL:CO2 Failed \r\n");
      //strcpy(floatStr, "Failed \0");
      //sprintf(reply,":CTRL:CO2 Failed \r\n");
      break;
    case 254:
      strcpy(reply, ":CTRL:CO2 Pending \r\n");
      //strcpy(floatStr, "Pending \0");
      //sprintf(reply,":CTRL:CO2 Pending \r\n");
      break;
    case 255:
      strcpy(reply, ":CTRL:CO2 No Data \r\n");
      //strcpy(floatStr, "No data \0");
      //sprintf(reply,":CTRL:CO2 No Data \r\n");
      break;
  }
  //sprintf(reply,":CTRL:CO2 %s \r\n", floatStr);
  scpi.Reply(reply);
}

void SERVO_DEG_OUT_1()
{
  // :SERVO:DEG:OUT:1 0 to 180° position, for the moment enter voltage from 0000 to 10000 (0-10V)
  float _setPoint = scpi.Get_numeric_argument();
  int setPoint = conversion_DAC(_setPoint);
   dac2_T.setDACOutVoltage(setPoint, 1);

  dtostrf(_setPoint, 2, 2, floatStr);
  sprintf(reply, ":SERVO:DEG:OUT:1 %s\r\n", floatStr);

  scpi.Reply(reply);
}

void SERVO_DEG_OUT_2()
{
  // :SERVO:DEG:OUT:2 0 to 180° position, for the moment enter voltage from 0000 to 10000 (0-10V)
  float _setPoint = scpi.Get_numeric_argument();
  int setPoint = conversion_DAC(_setPoint);
   dac2_T.setDACOutVoltage(setPoint, 0);

  dtostrf(_setPoint, 2, 2, floatStr);
  sprintf(reply, ":SERVO:DEG:OUT:2 %s\r\n", floatStr);

  scpi.Reply(reply);
}

void SERVO_DEG_IN()
{
  // :SERVO:DEG:IN 0 to 180° position, for the moment enter voltage from 0000 to 10000 (0-10V)
  float _setPoint = scpi.Get_numeric_argument();
  int setPoint = conversion_DAC(_setPoint);
   dac1_T.setDACOutVoltage(setPoint, 0);

  dtostrf(_setPoint, 2, 2, floatStr);
  sprintf(reply, ":SERVO:DEG:IN %s\r\n", floatStr);

  scpi.Reply(reply);
}

void FAN_OMEGA()
{
  // :FAN:OMEGA 0 to 180° position, for the moment enter voltage from 0000 to 10000 (0-10V)
  float _setPoint = scpi.Get_numeric_argument();
  int setPoint = _setPoint;
   dac1_T.setDACOutVoltage(setPoint, 1);

  dtostrf(_setPoint, 2, 2, floatStr);
  sprintf(reply, ":FAN:OMEGA %s\r\n", floatStr);

  scpi.Reply(reply);
}
