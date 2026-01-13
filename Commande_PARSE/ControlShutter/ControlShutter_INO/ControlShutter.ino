  // File: ContolShutter
  // 3 position shutter
  // 15.11.2023
  
  /* Parse command

  *IDN?
  :POS:NUM n    n: 1 to 4 shutter position
  :POS:DEG n    n: 0 to 180° position

  */
  
  #include <Arduino.h>
  #include <Servo.h>

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

  #define Pos1  0 // Position en degré
  #define Pos2  20
  #define Pos3  42
  #define Pos4  90

  //******************FUNCTION DECLARATIONS**************************
  void POS_NUM();
  void POS_DEG();
  
  //************************ VARIABLES ******************************

  Parse_SCPI scpi;  

  int sys_mode = 0;
  int chan_mode[10] = {0,0,0,0,0,0,0,0,0,0};

  char reply[SIZE_COMMAND];
  char floatStr[16];

  Servo myservo;

  //************************************************************
  //********************* SETUP ********************************
  //************************************************************
  void setup() 
  {
    
    Serial.begin(115200);

    //la reponse au *IDN? se fait automatiquement avec la chaine marqué ici.  
    scpi.Set_IDN("EPFL SB ISIC-GE AECH Parse Model");

    //Marque ici les commandes et le fonction qui devrait être appelé par la commande
    scpi.Register_command(":POS:NUM",&POS_NUM);
    scpi.Register_command(":POS:DEG",&POS_DEG);

    myservo.attach(8);  // attaches the servo on pin 9 to the servo object
    
    Serial.println("Control Shutter");
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


// 4 positions des ouvertures
void POS_NUM()
{
  float _setPoint = scpi.Get_numeric_argument();

    switch(int(_setPoint))
    {
      case 1:
       myservo.write(Pos1);  
      break;
      case 2:
       myservo.write(Pos2);  
      break;
       case 3:
       myservo.write(Pos3);  
      break;
       case 4:
       myservo.write(Pos4);  
      break;
      default:
        myservo.write(Pos1);  
      break;

    }


  
   dtostrf(_setPoint,0,0,floatStr);
  sprintf(reply,":POS:NUM %s\r\n",floatStr);

  scpi.Reply(reply);
}

void POS_DEG()
{
  float _setPoint = scpi.Get_numeric_argument();

   myservo.write(int(_setPoint));  

  dtostrf(_setPoint,0,0,floatStr);
  sprintf(reply,":POS:DEG %s\r\n",floatStr);

  scpi.Reply(reply);
}

