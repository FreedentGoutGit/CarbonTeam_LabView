#include <Arduino.h>
#ifndef __AVR__
#include <Ethernet.h>
#include <SPI.h>
#endif
#include <stdio.h>

#ifdef __AVR__
#define SIZE_COMMAND 48
#define MAX_COMMANDS 9
#else //due
#define SIZE_COMMAND 128
#define MAX_COMMANDS 64
#endif

class Parse_SCPI
{
  protected:
#ifndef __AVR__
  EthernetServer server; //(510); // should normally give the port number here but we will do this
                         // as an initializer in the class definition: server(portNum);
  EthernetClient client;
#endif
  bool EthernetValid; // = false;

  // String serial monitor
  char receivedCharsSerial[SIZE_COMMAND]; 
  int ii;
#ifndef __AVR__
  char receivedCharsEthernet[SIZE_COMMAND]; 
#endif
  int jj;
  
  char cmdChars[SIZE_COMMAND];
  char argChars[SIZE_COMMAND];
  
  bool terminalEcho;
  bool SerialStringComplete;
  bool EthernetStringComplete;
  bool UseNativeUSB;

  int command_indices[10];
  int N_indices;

  int N_commands;
  int Command_index;
  char cmd_list[MAX_COMMANDS][SIZE_COMMAND];
  //typedef void(*FunctionPointer)();
  //FunctionPointer function_list[MAX_COMMANDS];

  char _idn[SIZE_COMMAND];
  

  public:
  typedef void(*FunctionPointer)();
  FunctionPointer function_list[MAX_COMMANDS];
  
  Parse_SCPI(int EthernetServerPort = 510);
  void Set_IDN(const char* ID);
  int Register_command(const char* cmd,FunctionPointer f);
  void Ethernet_begin(byte* mac,int* ip_address);
  void Serial_begin(int baud);
  bool Command_received();
  char* Get_command();
  int Execute_command();
  int Get_command_index(int index = 0);
  float Get_numeric_argument();
  char* Get_text_argument();
  void Reply(char* reply);
  void UseNativePort();
  void UseProgrammingPort();
  
  private:
  void EthernetPoll();
  void SerialPoll();
  int matchCommand(char *cmd,char* cmdRef);
  int IDN();
  int ECHO();
  float parseNumeric(char *numeric);
};
