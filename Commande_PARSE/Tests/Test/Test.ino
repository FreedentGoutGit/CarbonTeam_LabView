  // File: ContolShutter
  // 3 position shutter
  // 15.11.2023
  
  /* Parse command

  *IDN?
  :POS:NUM n    n: 1 to 4 shutter position
  :POS:DEG n    n: 0 to 180° position

  */
  
  #include <Arduino.h>
  #include <DFRobot_GP8403.h>
  #include <SPI.h>

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
  
  const int analogPin = A15; // Pin connected to the thermistor/voltage divider
  const float referenceVoltage = 5.0; // Reference voltage of Arduino (in volts)
  const float R_known = 220.0; // Known resistor value in Ohms (change if using a different resistor)
  const float R_thermistor_at_25C = 100.0; // Resistance of thermistor at 25°C (change as per your thermistor specification)

  //******************FUNCTION DECLARATIONS**************************
  /*
  void SERVO_DEG_IN();
  
  void SERVO_DEG_OUT_2();
  void FAN_OMEGA();
  */
  void SERVO_DEG_OUT_1();
  void CTRL_TEMP();
  //************************ VARIABLES ******************************

  Parse_SCPI scpi;  

  int sys_mode = 0;
  int chan_mode[10] = {0,0,0,0,0,0,0,0,0,0};

  char reply[SIZE_COMMAND];
  char floatStr[16];

  //************************************************************
  //********************* SETUP ********************************
  //************************************************************
  void setup() 
  {
    
    Serial.begin(115200);

    //SCPI
    //la reponse au *IDN? se fait automatiquement avec la chaine marqué ici.  
    scpi.Set_IDN("EPFL SB ISIC-GE AECH Parse Model");
    //Marque ici les commandes et le fonction qui devrait être appelé par la commande
    /*
    scpi.Register_command(":SERVO:DEG:IN",&SERVO_DEG_IN);
    scpi.Register_command(":SERVO:DEG:OUT:2",&SERVO_DEG_OUT_2);
    scpi.Register_command(":FAN:OMEGA",&FAN_OMEGA);
    */
    scpi.Register_command(":SERVO:DEG:OUT:1",&SERVO_DEG_OUT_1);
    scpi.Register_command(":CTRL:TEMP",&CTRL_TEMP);
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
  if (setPoint > 10) {
    Serial.println("Error: Outside of possible range");
    return 0;
  }
  int modif_setPoint = map(setPoint, 0, 10, 0, 10000);
  return modif_setPoint;
}

void CTRL_TEMP()
{ 
  // CTRL_TEMP read temperature from analog sensor
  float sensorValue = analogRead(analogPin);
  float voltage = (sensorValue / 1023.0) * referenceVoltage;
  // Calculate the thermistor resistance
  float R_thermistor = (R_known * (referenceVoltage - voltage)) / voltage;
  // Add code here to convert resistance to temperature
  float Temperature = 1.0 / (3850 * pow(10,-6)) * ((R_thermistor / 100) - 1.0);
  // float Test = 1.0;

//#ifdef __AVR__
  //dtostrf(Temperature,5,3,floatStr);
  dtostrf(Temperature, 0, 0, floatStr);
  sprintf(reply,":CTRL:TEMP %s \r\n",floatStr);
//#else
  //sprintf(Temperature,":CTRL:TEMP %f\r\n", Temperature);
//#endif

  scpi.Reply(reply);

}


void SERVO_DEG_OUT_1()
{
  // :SERVO:DEG:OUT:1 0 to 180° position, for the moment enter voltage from 0000 to 10000 (0-10V)
  float _setPoint = scpi.Get_numeric_argument();
  int setPoint = conversion_DAC(_setPoint);
   dac2_T.setDACOutVoltage(setPoint, 1);

  dtostrf(setPoint, 0, 0, floatStr);
  sprintf(reply, ":SERVO:DEG:OUT:1 %s\r\n", floatStr);

  scpi.Reply(reply);
}

/*
void SERVO_DEG_IN()
{
  // :SERVO:DEG:IN 0 to 180° position, for the moment enter voltage from 0000 to 10000 (0-10V)
  float _setPoint = scpi.Get_numeric_argument();
  int setPoint = conversion_DAC(_setPoint);
   dac1_T.setDACOutVoltage(setPoint, 0);

  dtostrf(setPoint, 0, 0, floatStr);
  sprintf(reply, ":SERVO:DEG:IN %s\r\n", floatStr);

  scpi.Reply(reply);
}

void FAN_OMEGA()
{
  // :FAN:OMEGA 0 to 180° position, for the moment enter voltage from 0000 to 10000 (0-10V)
  float _setPoint = scpi.Get_numeric_argument();
  int setPoint = conversion_DAC(_setPoint);
   dac1_T.setDACOutVoltage(setPoint, 1);

  dtostrf(setPoint, 0, 0, floatStr);
  sprintf(reply, ":FAN:OMEGA %s\r\n", floatStr);

  scpi.Reply(reply);
}

void SERVO_DEG_OUT_2()
{
  // :SERVO:DEG:OUT:2 0 to 180° position, for the moment enter voltage from 0000 to 10000 (0-10V)
  float _setPoint = scpi.Get_numeric_argument();
  int setPoint = conversion_DAC(_setPoint);
   dac2_T.setDACOutVoltage(setPoint, 0);

  dtostrf(setPoint, 0, 0, floatStr);
  sprintf(reply, ":SERVO:DEG:OUT:2 %s\r\n", floatStr);

  scpi.Reply(reply);
}
*/
