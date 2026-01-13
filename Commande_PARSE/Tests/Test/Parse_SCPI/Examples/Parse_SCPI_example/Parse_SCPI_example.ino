  // File: Model_Parse_class
  // Model de parse
  // 10.09.2021
  
  // Model de base pour l'utilisation de commande de type PARSE
  
  /* Structure du programme
  
 
  */
  
  #include <Arduino.h>
  
  #ifndef PARSE_SCPI_H
  #include "Parse_SCPI.h"
  #define PARSE_SCPI_H
  #endif
  
  //************************ DEFINE *********************************
  #define N_SYS_MODE 3
  const char *sys_mode_list[N_SYS_MODE] = {"OFF","NORM","TEST"};
  #define N_CHAN_MODE 3
  const char *chan_mode_list[N_SYS_MODE] = {"OFF","NORM","TEST"};

  //******************FUNCTION DECLARATIONS**************************
  void VOLT_MEAS();
  void VOLT_SET();
  void SYS_MODE();
  void CHAN_MODE();

  //************************ VARIABLES ******************************

#ifndef __AVR__
  byte mac[6] = {0xA8, 0x61, 0x0A, 0xAE, 0x72, 0x71};
  int ip_address[4] = {10, 1, 1, 12};
#endif
  int EthernetPort = 510;
  
  Parse_SCPI scpi(EthernetPort); 
  float voltage = 1.234;
  int sys_mode = 0;
  int chan_mode[10] = {0,0,0,0,0,0,0,0,0,0};

  char reply[SIZE_COMMAND];
  char floatStr[16];

  //************************************************************
  //********************* SETUP ********************************
  //************************************************************
  void setup() 
  {
    //Pour le Due on peut choisir le port USB pour la communication
	//Le 'programming port' est le valeur par défaut.
    //scpi.UseNativePort();
    //scpi.UseProgrammingPort();
    
    // Config Serial Monitor
    scpi.Serial_begin(115200);
	Serial.begin(115200);

    //pas d'ethernet pour le UNO
#ifndef __AVR__
    //initialize ethernet if using
    scpi.Ethernet_begin(mac, ip_address);
#endif

    //Set the ID that will be returned upon *IDN? query
    scpi.Set_IDN("EPFL SB ISIC-GE AECH Parse Model");

    scpi.Register_command(":VOLT:MEAS",&VOLT_MEAS);
    scpi.Register_command(":VOLT:SET",&VOLT_SET);
    scpi.Register_command(":SYS:MODE",&SYS_MODE);
    scpi.Register_command(":CHAN#:MODE",&CHAN_MODE);

    Serial.println("Model_Parse_class setup complete");
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

void VOLT_MEAS()
{
  float _voltage = 0.;
  
  //code to read voltage goes here...
  _voltage = voltage;
  
  
#ifdef __AVR__
  dtostrf(_voltage,5,3,floatStr);
  sprintf(reply,":VOLT:MEAS %s\r\n",floatStr);
#else
  sprintf(reply,":VOLT:MEAS %f\r\n",_voltage);
#endif
  
  scpi.Reply(reply);
}

void VOLT_SET()
{
  float _setPoint = scpi.Get_numeric_argument();

  //code to set voltage goes here...
  voltage = _setPoint;
  
  //char* reply;
  //sprintf ne marche pas avec AVR (UNO, nano) mais dtostrf ne marche pas avec les autres (Due)....
#ifdef __AVR__
  dtostrf(_setPoint,5,3,floatStr);
  sprintf(reply,":VOLT:SET %s\r\n",floatStr);
#else
  sprintf(reply,":VOLT:SET %f\r\n",_setPoint);
#endif
  scpi.Reply(reply);
}

void SYS_MODE()
{
  char* _mode = scpi.Get_text_argument();
  int i = 0;
  
  //is this command a query?
  if(!strcmp(_mode,"?")) {
    sprintf(reply,":SYS:MODE %s\r\n",sys_mode_list[sys_mode]);
  }else {
    //search sys_mode_list for a match to what was sent.
    for(i=0; i<N_SYS_MODE; i++) {
      if(!strcmp(_mode,sys_mode_list[i])) break;
    }
    if(i<N_SYS_MODE) {
      sys_mode = i;
      sprintf(reply,":SYS:MODE %s\r\n",sys_mode_list[sys_mode]);
    }else {
      sprintf(reply,"ERR: MODE %s not valid\r\n",_mode);
    }
  }
  
  scpi.Reply(reply);
}

void CHAN_MODE()
{
  int i = 0;
  char* _mode = scpi.Get_text_argument();
  int _chan = scpi.Get_command_index();
  if(_chan < 0 or _chan > 9) 
  {
    sprintf(reply,"ERR: Channel index out of range\r\n");
    scpi.Reply(reply);
    return;
  }
  //is this command a query?
  if(!strcmp(_mode,"?")) {
    sprintf(reply,":CHAN%d:MODE %s\r\n",_chan,sys_mode_list[chan_mode[_chan]]);
  }else {
    //search sys_mode_list for a match to what was sent.
    for(i=0; i<N_CHAN_MODE; i++) {
      if(!strcmp(_mode,chan_mode_list[i])) break;
    }
    if(i<N_CHAN_MODE) {
      chan_mode[_chan] = i;
      sprintf(reply,":CHAN%d:MODE %s\r\n",_chan,sys_mode_list[chan_mode[_chan]]);
    }else {
      sprintf(reply,"ERR: MODE %s not valid\r\n",_mode);
    }
  }
  
  scpi.Reply(reply);
}
