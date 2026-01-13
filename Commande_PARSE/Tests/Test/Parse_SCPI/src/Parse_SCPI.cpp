#include "Parse_SCPI.h"

#ifndef ARDUINO
#define ARDUINO  //should already be defined by IDE
#endif

#ifndef __SAM3X8E__ 
#ifndef __SAMD21G18A__
#define SerialUSB Serial
#endif
#endif

//public functions
#ifndef __AVR__
Parse_SCPI::Parse_SCPI(int EthernetServerPort) : server(EthernetServerPort)
#else 
Parse_SCPI::Parse_SCPI(int EthernetServerPort)
#endif
{
  N_indices = 0;
  N_commands = 0;
  Command_index = -1;
  terminalEcho = false;
  EthernetValid = false;
  SerialStringComplete = false;
  EthernetStringComplete = false;
  UseNativeUSB = false;
  jj = 0;
  ii = 0;
  
}

void Parse_SCPI::Set_IDN(const char* ID) 
{
  sprintf(_idn,"%s\r\n",ID);
}

int Parse_SCPI::Register_command(const char* cmd,FunctionPointer func)
{
  if(N_commands < MAX_COMMANDS) {
    strcpy(cmd_list[N_commands],cmd);
    function_list[N_commands] = func;
//    Serial.print("Adding command ");
//    Serial.print(cmd);
//    Serial.print(" at index ");
//    Serial.print(N_commands);
//    Serial.print(" with function at address ");
//    Serial.println((unsigned long)function_list[N_commands]);
    N_commands++;
  }
  else
  {
	Serial.print("ERROR: number of registered commands exceeds maximum of ");
	Serial.println(MAX_COMMANDS);
	Serial.print("The command ");
	Serial.print(cmd);
	Serial.println(" will not be registered");

    return -1;
  }
}

void Parse_SCPI::Ethernet_begin(byte* mac,int* ip_address)
{
#ifndef __AVR__
  IPAddress ip(ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
  Ethernet.begin(mac, ip);
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Disabling Ethernet Communication");
    EthernetValid = false;
  }else EthernetValid = true;
#endif
}

void Parse_SCPI::Serial_begin(int baud)
{
  if(UseNativeUSB) {
	  SerialUSB.begin(baud);
	  while(!SerialUSB);
  }else {
	  Serial.begin(baud);
  }
}

bool Parse_SCPI::Command_received()
{
  char *token;
  //check for messages first on the serial interface
  this->SerialPoll();
  if(SerialStringComplete) {
    token = strtok(receivedCharsSerial," \n\r");
  }else {  //if there is nothing on the serial line, check the ethernet
#ifndef __AVR__
    this->EthernetPoll();
    if(EthernetStringComplete) {
      token = strtok(receivedCharsEthernet," \n\r");
    }
#endif
  }
  
  if(SerialStringComplete or EthernetStringComplete)
  {
    //split the received string into command and argument strings
    //make a local copy for strtok to work on
    strcpy(cmdChars,token);
    token = strtok(NULL,"\n\r");
    strcpy(argChars,token);

    //check if the command is either IDN or ECHO
    if(!strcmp(cmdChars,"*IDN?")) {this->IDN(); return false;}
    if(!strcmp(cmdChars,"ECHO")) {this->ECHO(); return false;}
    
    //Check if the command is properly formatted
    for(int i=0; i<N_commands; i++) {
      if(this->matchCommand(cmdChars,cmd_list[i]))
      {
        Command_index = i;
        return true;
      }
    }
    //if we get here then none of the registered commands matched the received one.
    this->Reply("Err00\r\n");
    return false;
  }
  return false;
}

char* Parse_SCPI::Get_command() 
{
  return cmdChars;
}

int Parse_SCPI::Execute_command()
{
  if(Command_index >= 0 && Command_index <= N_commands)
  {
    //call the function from the function list
//    Serial.print("Executing function at address ");
//    Serial.println((unsigned long)function_list[Command_index]);
//    Serial.println("...get ready for it...");
    function_list[Command_index]();
    return 0;
  }
  else return -1;
}

int Parse_SCPI::Get_command_index(int index)
{
  if(index <= (N_indices-1)) return command_indices[index];
  else return -1;
}

float Parse_SCPI::Get_numeric_argument()
{
  return this->parseNumeric(argChars);
}

char* Parse_SCPI::Get_text_argument()
{
  return argChars;
}

void Parse_SCPI::Reply(char* reply)
{
  if (SerialStringComplete)
  { 
	if(UseNativeUSB) SerialUSB.write(reply);
    else Serial.write(reply);
    
    //clear the buffer
    for (int i = 0; i < SIZE_COMMAND; i++) receivedCharsSerial[i] = '\0';
    ii = 0;
    SerialStringComplete = false;
  }

#ifndef __AVR__
  if(EthernetStringComplete)
  {
    server.write(reply);
      
    //clear the buffer
    for (int i = 0; i < SIZE_COMMAND; i++) receivedCharsEthernet[i] = '\0';
    jj = 0;
    EthernetStringComplete = false;
  }
#endif
}

void Parse_SCPI::UseNativePort()
{
	UseNativeUSB = true;
}

void Parse_SCPI::UseProgrammingPort()
{
	UseNativeUSB = false;
}

//protected functions
void Parse_SCPI::EthernetPoll()
{
#ifndef __AVR__
  if(!EthernetStringComplete and EthernetValid)
  {
    EthernetClient client = server.available();
  
    if(client) {
      //Serial.println("Client Available");
      while(client.available() > 0) {
        
        // read the bytes incoming from the client:
        char thisChar = client.read();
        
        if (jj < SIZE_COMMAND)
        {
          receivedCharsEthernet[jj] = thisChar;
          jj++;
          receivedCharsEthernet[jj] = '\0';
        }
        //buffer is full.  trash it
        else
        {
          for (int i = 0; i < SIZE_COMMAND; i++) receivedCharsEthernet[i] = '\0';
          jj = 0;
          client.flush();
          break;
        }
  
        if(thisChar == '\n') {
          EthernetStringComplete = true;
          break;
        }
      }
    }
    
  } //end if client
#endif
}

void Parse_SCPI::SerialPoll()
{
  if(!SerialStringComplete)
  {
	bool serialAvailable = false;
	if(UseNativeUSB) serialAvailable = (SerialUSB.available() > 0);
	else serialAvailable = (Serial.available() > 0);
    while (serialAvailable)
    {
      // get the new byte:
	  char inChar;
	  if(UseNativeUSB) inChar = (char)SerialUSB.read();
	  else inChar = (char)Serial.read();
      if (terminalEcho) {
		  if(UseNativeUSB) SerialUSB.print(inChar);
		  else Serial.print(inChar);
	  }
      // add it to the inputString:
  
      if (ii < SIZE_COMMAND)
      {
        receivedCharsSerial[ii] = inChar;
        ii++;
        receivedCharsSerial[ii] = '\0';
      }
      //buffer is full.  trash it
      else
      {
        for (int i = 0; i < SIZE_COMMAND; i++) receivedCharsSerial[i] = '\0';
        ii = 0;
      }
      // if the incoming character is a newline then the command is complete.
      // set a flag so we know to pass it to the parse
      if (inChar == '\n' || inChar == '\r')
      {
        SerialStringComplete = true;
        if (terminalEcho) {
			if(UseNativeUSB) SerialUSB.print("\r\n");
			else Serial.print("\r\n");
		}
		break;
      }
	  //check if there are more characters to read
	  if(UseNativeUSB) serialAvailable = (SerialUSB.available() > 0);
	  else serialAvailable = (Serial.available() > 0);
    }
  }
}

//Determine if cmd matches cmdRef
//cmdRef can have # as a wildcard indicating that there is a number embedded.  
//Extract the number and put it into 'data'
//currently only supports single numbers from 0 to 9
int Parse_SCPI::matchCommand(char *cmd,char* cmdRef)
{
  int len;
  int lenRef;
  int _numeric;
  len = strlen(cmd);
  lenRef = strlen(cmdRef);
  N_indices = 0;
  
  if(len != lenRef) {
    return 0;
  }
  
  for(int j=0; j<len; j++) {
    if(cmdRef[j] == '#') {
      _numeric = (int)cmd[j] - 48;
      if(_numeric < 0 or _numeric > 9) {
        return 0; //character in # position not a number from 0 to 9.  
      }
      command_indices[N_indices] = _numeric;
      N_indices++;
    }else if(cmd[j] != cmdRef[j]) {
      return 0; //character j does not match.
    }
  }
  return 1;
}

int Parse_SCPI::IDN()
{
  this->Reply(_idn);
  return 0;
}

int Parse_SCPI::ECHO()
{
  char *token = strtok(argChars," \n\r");
  if(!strcmp(token,"ON")) {
    terminalEcho = true;
    this->Reply("ECHO ON\r\n");
  }
  else if(!strcmp(token,"OFF")) {
    terminalEcho = false;
    this->Reply("ECHO OFF\r\n");
  }
  else {
    return 1;  
  }
  return 0;
}

float Parse_SCPI::parseNumeric(char *numeric)
{
  //Serial.print(numeric);
  char *token;
  char numericLocal[16];
  //make a copy for strtok
  if(strlen(numeric) <= 16) strcpy(numericLocal,numeric);
    int i;
  token = strtok(numericLocal," :\n\rnumV");
  float setVal = strtod(token,NULL);
  if(abs(setVal) < 1e-12) //atof retourne 0.0 comme erreur, alors assurer que les zeros sont vrai. 
  {
    for(i=0; i<strlen(token); i++) //il ne devrait pas avoir d'autres caractères pour un vrai zero
      if(token[i] != '0' && token[i] != '.' && token[i] != '+' && token[i] != '-' && token[i] != 'e' && token[i] != '\0') return -99999.9;
  }
  //les unités...
  //strtok enlève le valeur qui nous concerne alors il faut le chercher autrement...
  char unitchar;
  unitchar = numeric[strlen(token)];
  if(unitchar=='m') setVal *= 1e-3;
  if(unitchar=='n') setVal *= 1e-9;
  if(unitchar=='u') setVal *= 1e-6;
  
  return setVal;
}
