/*
 * Правки, внесённые начиная с 2016-06-27, Мельниковым А.В., 
 * помечены комментарием с тегом @inzheneher и числом внесения изменения.
 * Пример:
 * @inzheneher: 2016-06-27
 */
 
#include <SimpleModbusSlave.h>

#define   SetBit(reg, bit)         reg |= (1<<bit)            
#define   ClearBit(reg, bit)       reg &= (~(1<<bit))
#define   InvBit(reg, bit)         reg ^= (1<<bit)
#define   BitIsSet(reg, bit)       ((reg & (1<<bit)) != 0)
#define   BitIsClear(reg, bit)     ((reg & (1<<bit)) == 0)

//Константа ШИМ выходя для управления освещением
const int LIGTH_MODE_PIN   = 8;

//Константы номеров битов входов в пакете (номер вывода ARDUINO +22)
const int I_UF_BUTTON   = 0;
const int I_PLUG_PUZZLE = 1;
const int I_CEP_PUZZLE  = 2;
const int I_CEP_LED     = 3;
const int I_PALEC_SENS  = 4;
const int I_JAMP_HAND   = 5;
const int I_CODE_PANEL1 = 6;
const int I_NISHA_SHOK  = 7;
const int I_CODE_PANEL2 = 8;
const int I_KILL_BUTTON = 9;
const int I_DOOR3_SENS  = 10;

//Константы номеров битов выходов в пакете (номер вывода ARDUINO +38)
const int O_UF_LAMP      = 0;
const int O_EM_CEILBOX   = 1;
const int O_EM_KUKLA     = 2;
const int O_LIGHT_KUKLA  = 3; //Не работает вероятно релюха !!!! - 2016-07-09: убрал коммент, нужно проверить
const int O_EM_DOOR2     = 4;
const int O_LIGHT_KUKLA  = 5;
const int O_LIGHT_YAMA   = 6;
const int O_EM_CODEBOX   = 7;
const int O_EM_DOOR3     = 8;
const int O_EM_DOOR1     = 9;
const int O_LIGHT_DOOR1  = 10;
const int O_LIGHT_KILLBUT= 11;
const int O_SMOKE_MACH   = 12;
const int O_CEP_WORK     = 13;

//Константы номеров битов допсигналов управления
const int W_START_GAME   = 0;
const int W_PLAYER_STATE = 1;
const int W_RESET_GAME   = 2;
const int W_BLOK_DOOR1   = 3;
const int W_BLOK_DOOR2   = 4;
const int W_BLOK_DOOR3   = 5;
const int W_NEXT_LEVEL   = 6;

//Константы режимов освещения
const int LM_DEFAULT = 0;
const int LM_GAME    = 1;
const int LM_MUTED   = 2;
const int LM_OFF     = 3;

//Константы номеров выходов для управления питанием дыммашины и монитора
const int P_SMOKE_MACH = 6; //Желтый провод
const int P_MONITOR_POWER = 7; //Синий провод


enum 
{     
  PACKET_1,         //Пакет битов №1 (сигналы входов)
  PACKET_2,         //Пакет битов №2 (сигналы выходов)
  PACKET_3,         //Пакет битов №3 (доп сигналы управления игрой)
  GAME_TIMER,       //Значение таймера игры
  GAME_LEVEL,       //Текущий этап игры
  LIGHT_MODE,       //Значение режима освещения
  VIDEO_NUM,        //Номер видео-ролика для плеера
  HOLDING_REGS_SIZE  // leave this one
  // total number of registers for function 3 and 16 share the same register array
  // i.e. the same address space
};


//Переменные для хранения состоянния пакетов битов
unsigned int Packet1     = 0;
unsigned int Old_Packet1 = 0;
unsigned int Packet2     = 0;
unsigned int Old_Packet2 = 0;
unsigned int Packet3     = 0;
unsigned int Old_Packet3 = 0;

//Переменные для хранения состояния 16-битных значений
int GameTimer = 0;
int Old_GameTimer = 0;
int GameLevel = 0;
int Old_GameLevel = 0;
int LightMode = 0;
int Old_LightMode = 0;
int VideoNum  = 0;
int Old_VideoNum  = 0;

//Счетчик для задержки
int DelayTimer1 = 0;
int DelayTimer2 = 0;
int ResetTimer = 0;
int LightTimer = 0;
int VideoTimer = 0;
int SmokeTimer = 0;


//Текущее значение ШИМ света
int PWM_Light = 0;
int Old_PWM_Light = 0;

int NextLevel = 0;

//Определяем массив регистров
unsigned int holdingRegs[HOLDING_REGS_SIZE]; // function 3 and 16 register array


void setdefaultmode()
{
   GameLevel = 0;
   LightMode = LM_DEFAULT; 
   VideoNum = 0;
   Old_VideoNum!=VideoNum;
   
   ClearBit(Packet2, O_UF_LAMP);
   SetBit(Packet2, O_EM_CEILBOX);
   SetBit(Packet2, O_EM_KUKLA);
   ClearBit(Packet2, O_LIGHT_KUKLA);
   SetBit(Packet2, O_EM_DOOR2);      //!!!! Закроем по умолчанию
   ClearBit(Packet2, O_LIGHT_YAMA);
   SetBit(Packet2, O_EM_CODEBOX);
   SetBit(Packet2, O_EM_DOOR3);      //!!!! Закроем по умолчанию
   SetBit(Packet2, O_EM_DOOR1);      //!!!! Закроем по умолчанию
   SetBit(Packet2, O_LIGHT_DOOR1);
   ClearBit(Packet2, O_LIGHT_KILLBUT);
   ClearBit(Packet2, O_SMOKE_MACH);
   ClearBit(Packet2, O_CEP_WORK );
   
   ClearBit(Packet3, W_START_GAME);
   ClearBit(Packet3, W_PLAYER_STATE);
   ClearBit(Packet3, W_RESET_GAME);
   
   SetBit(Packet3, W_BLOK_DOOR1);
   SetBit(Packet3, W_BLOK_DOOR2);
   SetBit(Packet3, W_BLOK_DOOR3);
   
   holdingRegs[PACKET_3] = Packet3;
   
   analogWrite(LIGTH_MODE_PIN , 255);
   
   digitalWrite(P_MONITOR_POWER, LOW); //Выключили монитор
   digitalWrite(P_SMOKE_MACH, LOW); //Выключили дыммашину
   
   delay(500);
}

void setstartmode()
{
   GameLevel = 1;
   LightMode = LM_GAME; 
   VideoNum = 0;
   
   ClearBit(Packet2, O_UF_LAMP);
   SetBit(Packet2, O_EM_CEILBOX);
   SetBit(Packet2, O_EM_KUKLA);
   ClearBit(Packet2, O_LIGHT_KUKLA);
   SetBit(Packet2, O_EM_DOOR2);
   ClearBit(Packet2, O_LIGHT_YAMA);
   SetBit(Packet2, O_EM_CODEBOX);
   SetBit(Packet2, O_EM_DOOR3);
   SetBit(Packet2, O_EM_DOOR1);
   ClearBit(Packet2, O_LIGHT_DOOR1);
   ClearBit(Packet2, O_LIGHT_KILLBUT);
   ClearBit(Packet2, O_SMOKE_MACH);

   
   //ClearBit(Packet1, W_START_GAME);
   //ClearBit(Packet1, W_PLAYER_STATE);
   //ClearBit(Packet1, W_RESET_GAME);
   
   //analogWrite(LIGTH_MODE_PIN , 255);
   
}

void setup()
{
  //Инициализируем и настроиваем MODBUS
  /* Valid modbus byte formats are:
     SERIAL_8N2: 1 start bit, 8 data bits, 2 stop bits
     SERIAL_8E1: 1 start bit, 8 data bits, 1 Even parity bit, 1 stop bit
     SERIAL_8O1: 1 start bit, 8 data bits, 1 Odd parity bit, 1 stop bit
     
     You can obviously use SERIAL_8N1 but this does not adhere to the
     Modbus specifications. That said, I have tested the SERIAL_8N1 option 
     on various commercial masters and slaves that were suppose to adhere
     to this specification and was always able to communicate... Go figure.
     
     These byte formats are already defined in the Arduino global name space. 
  */
  modbus_configure(&Serial, 9600, SERIAL_8N2, 1, 2, HOLDING_REGS_SIZE, holdingRegs);    
  
  //Инициализация входов/выходов
  
  //Входы с 22 по 33 с подключением подтягивающих резисторов
  for (int i=0; i <= 11; i++){
    /*
     pinMode(22+i, INPUT);      
     digitalWrite(22+i, HIGH);
     */
     //@inzheneher: 2016-06-27, не используем INPUT_PULLUP, потому что входа уже притянуты по-умолчанию
     pinMode(22+i, INPUT);
  } 
  
  //Выходы с 36 по 49 на реле управления
  for (int i=0; i <= 13; i++){
    /*
     pinMode(38+i, OUTPUT);
     */
     //@inzheneher: 2016-06-27, используем свои реле, которые начинаются с 36 пина, количество то же - 14 шт.
     pinMode(36+i, OUTPUT);
  }
  //@inzheneher: 2016-06-27
  pinMode(P_SMOKE_MACH, OUTPUT);
  pinMode(P_MONITOR_POWER, OUTPUT);
  /*
  pinMode(8, OUTPUT); //ШИМ на усилитель управления освещением 1-й комнаты
  */
  //@inzheneher: 2016-06-27, замена числовой константы на строковую переменную
  pinMode(LIGTH_MODE_PIN, OUTPUT); //ШИМ на усилитель управления освещением 1-й комнаты
  
  setdefaultmode(); //Включаем режим ожидания
  
  //GameLevel = 1; //ВРЕМЕННО !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
  //LightMode = 1; //ВРЕМЕННО !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //SetBit(Packet2, O_CEP_WORK ); //ВРЕМЕННО !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  Old_GameLevel = GameLevel;
  GameTimer=3600;
  holdingRegs[GAME_TIMER]=GameTimer;
  ResetTimer=0;
  
  PWM_Light=255;            
  Old_PWM_Light=PWM_Light;  
  DelayTimer1=5000;
}

void loop()
{
  //Считываем из MODBUS пакет управления;
  Packet3 = holdingRegs[PACKET_3];
  GameTimer = holdingRegs[GAME_TIMER];
  
  //Запоминаем старый значения пакетов
  Old_Packet1 = Packet1;
  Old_Packet2 = Packet2;
  Old_Packet3 = Packet3;
  
  //Считываем состояние входов
  for (int i=0; i <= 11; i++){
    if (digitalRead(22+i) == HIGH) SetBit(Packet1, i); else ClearBit(Packet1, i);
  }
  
  
  
   //Обработка режима освещения ----------------------------------------------------------------------------------------------------------------
  switch (LightMode)
  {
    case 0: 
        //if (Old_LightMode!=LightMode) {
          Old_LightMode = LightMode;
          analogWrite(LIGTH_MODE_PIN , 255);
          //Old_PWM_Light=PWM_Light;
          //PWM_Light=255;         
        //}
        break;
   
   case 1: 
   
        //Сделать плавное включение света !!!!
        
        if (BitIsSet(Packet1, I_CEP_LED)) {
           analogWrite(LIGTH_MODE_PIN , 100);
        }  else {
           analogWrite(LIGTH_MODE_PIN , 30);
        } 
        break;   
        
   case 2: 
        //if (Old_LightMode!=LightMode) {
          Old_LightMode = LightMode;
          analogWrite(LIGTH_MODE_PIN , 20); 
          //Old_PWM_Light=PWM_Light;
          //PWM_Light=70;          
        //}
        break;
  
   case 3: 
        //if (Old_LightMode!=LightMode) {
          Old_LightMode = LightMode;
          analogWrite(LIGTH_MODE_PIN , 0);
          //PWM_Light=0;
          //Old_PWM_Light=PWM_Light;       
        //}
        break;  
        
    default:
        break;
  }


  /*if (Old_PWM_Light!=PWM_Light) {
     LightTimer++;
     if (LightTimer>400) {
        if (Old_PWM_Light<PWM_Light) PWM_Light++;
        if (Old_PWM_Light>PWM_Light) PWM_Light--;
        LightTimer=0; 
        analogWrite(LIGTH_MODE_PIN , PWM_Light);
     }
  }*/
  


  //Реализуем логику игры =====================================================================================================================
 
  //Обработка команды сброса игры
  if (BitIsSet(Packet3, W_RESET_GAME)) {
    ResetTimer++;
    if (ResetTimer>=500) {
       setdefaultmode();
       VideoNum=0;
       VideoTimer=0;
       ClearBit(Packet3, W_RESET_GAME );
       ResetTimer=0;
       //@inzheneher: 2016-06-27
       digitalWrite(P_MONITOR_POWER, LOW); //Выключили монитор
       digitalWrite(P_SMOKE_MACH, LOW); //Выключили дыммашину
    }
  }
 
  //Обработка команды Перейти на следующий этап
  if (BitIsSet(Packet3, W_NEXT_LEVEL)) {
     NextLevel = 1;
  }
  
  switch (GameLevel)
  {
    case 0: //Режми игры в ожидании
        
        if (BitIsSet(Packet3, W_START_GAME)) {
            setstartmode();
            LightMode = LM_OFF;  
            DelayTimer1=2000;             
            Old_GameLevel = GameLevel; GameLevel=1;                 
          }  
        
        break;

    case 1: 

        if (DelayTimer1>0) {
             DelayTimer1--;
          } else {  
             VideoNum = 1;
             SetBit(Packet3, W_PLAYER_STATE);                    
             Old_GameLevel = GameLevel; GameLevel=2;      
          }  
        
        break;
 
    case 2:
   

           if (BitIsClear(Packet3, W_PLAYER_STATE)) {
              SetBit(Packet2, O_CEP_WORK );
              LightMode = LM_GAME; 
              if (BitIsClear(Packet1, I_UF_BUTTON)) {
                SetBit(Packet2, O_UF_LAMP);
                LightMode = LM_OFF;
              } else {
                ClearBit(Packet2, O_UF_LAMP);
                LightMode = LM_GAME;
              }
              
              if ( (BitIsClear(Packet1, I_PLUG_PUZZLE)) || (NextLevel==1) ) {
                ClearBit(Packet2, O_EM_CEILBOX);      
                NextLevel = 0;        
                Old_GameLevel = GameLevel; GameLevel=3;
              } 
           
        }
        break;
    
    case 3:
        if ( (BitIsClear(Packet1, I_CEP_PUZZLE)) || (NextLevel==1) ) {
          ClearBit(Packet2, O_CEP_WORK );
          VideoNum = 2;
          SetBit(Packet3, W_PLAYER_STATE);
          LightMode = LM_MUTED;
          DelayTimer1 = 5000;
          DelayTimer2 = 2000;
          NextLevel = 0;
          Old_GameLevel = GameLevel; GameLevel=4;
        }
        break;
     
     case 4:
         if (BitIsClear(Packet3, W_PLAYER_STATE)) {
           if (DelayTimer1>0) {
              DelayTimer1--;
           } else {
              LightMode = LM_OFF;
              ClearBit(Packet2, O_EM_KUKLA );
              SetBit(Packet2, O_LIGHT_KUKLA );
              if (DelayTimer2>0) {
                 DelayTimer2--;
              } else {
                 LightMode = LM_GAME;                
                 Old_GameLevel = GameLevel; GameLevel=5;
              }
           }
         }  
        break;
        
     case 5:
        if ( (BitIsSet(Packet1, I_PALEC_SENS)) || (NextLevel==1) ) {
           ClearBit(Packet2, O_EM_DOOR2);
           NextLevel = 0;
           Old_GameLevel = GameLevel; GameLevel=6;
        }      
        break;
        
     case 6:
        if ( (BitIsSet(Packet1, I_JAMP_HAND)) || (NextLevel==1) ) {
           SetBit(Packet2, O_LIGHT_YAMA);
           ClearBit(Packet2, O_EM_CODEBOX);
           //@inzheneher: 2016-06-27, замена числовой константы на строковую
           digitalWrite(P_SMOKE_MACH, HIGH); //Включили дыммашину
           NextLevel = 0;
           Old_GameLevel = GameLevel; GameLevel=7;
        }      
        break;       
 
     case 7:
        if ( (BitIsClear(Packet1, I_CODE_PANEL1)) || (NextLevel==1) ) {
          
          //Вероятно поставить проверку на оставшееся время
          //и если его осталось меньше минуты то нафиг видео про друга!
          
           VideoNum = 3;
           SetBit(Packet3, W_PLAYER_STATE);
           LightMode = LM_MUTED;
           NextLevel = 0;  
           Old_GameLevel = GameLevel; GameLevel=8;
        }      
        break;   
 
     case 8:
        if (BitIsClear(Packet3, W_PLAYER_STATE)) {
           ClearBit(Packet2, O_EM_DOOR3);
           //@inzheneher: 2016-06-27, замена числовой константы на строковую
           digitalWrite(P_MONITOR_POWER, HIGH); //Включили монитор
           
           Old_GameLevel = GameLevel; GameLevel=9;
           LightMode = LM_GAME;  
        }     
        break;
 
     case 9:
        if (BitIsClear(Packet1, I_DOOR3_SENS)) {
           SetBit(Packet2, O_EM_DOOR3);
           Old_GameLevel = GameLevel; GameLevel=10;
        }     
        break;
 
     case 10:
        if (BitIsClear(Packet1, I_NISHA_SHOK)) {
           LightMode = LM_OFF;
        } else {
           LightMode = LM_DEFAULT;
        }
        if (BitIsClear(Packet1, I_CODE_PANEL2)) {
           ClearBit(Packet2, O_EM_DOOR3);
           VideoNum = 4;
           SetBit(Packet3, W_PLAYER_STATE);
           LightMode = LM_MUTED;  
           Old_GameLevel = GameLevel; GameLevel=11;
        }     
        break;

     case 11: //Общий конец игры
        if (BitIsClear(Packet3, W_PLAYER_STATE)) {
            ClearBit(Packet2, O_EM_DOOR3);
            ClearBit(Packet2, O_EM_DOOR1);
            SetBit(Packet2, O_LIGHT_DOOR1);  
            //@inzheneher: 2016-06-27, замена числовой константы на строковую        
            digitalWrite(P_SMOKE_MACH, LOW); //Выключили дыммашину
            ClearBit(Packet3, W_START_GAME);
         }     
        break;
              
     case 12: //Дл конца осталось 5 мин (нужно убить друга или погибнуть всем)
        if (BitIsClear(Packet3, W_PLAYER_STATE)) {
            SetBit(Packet2, O_LIGHT_KILLBUT);
            
            //Продолжаем проверять последний код
            if (BitIsClear(Packet1, I_NISHA_SHOK)) {
               LightMode = LM_OFF;
            } else {
               LightMode = LM_DEFAULT;
            }
            if (BitIsClear(Packet1, I_CODE_PANEL2)) {
               ClearBit(Packet2, O_EM_DOOR3);
               VideoNum = 4;
               SetBit(Packet3, W_PLAYER_STATE);
               LightMode = LM_MUTED;  
               Old_GameLevel = GameLevel; GameLevel=11;
            }
            
            
            if (BitIsClear(Packet1, I_KILL_BUTTON)) {
               SetBit(Packet2, O_SMOKE_MACH);
               SmokeTimer = 1500;
               //@inzheneher: 2016-06-27, замена числовой константы на строковую
               digitalWrite(P_MONITOR_POWER, LOW); //Выключили монитор
               VideoNum = 7;
               SetBit(Packet3, W_PLAYER_STATE);
               LightMode = LM_MUTED;  
               Old_GameLevel = GameLevel; GameLevel=11;         
            }
            
         }     
        break;       
 
     
       
    default:
        break;    
  }

  //Ручной переход на следующий этап
  if (BitIsSet(Packet3, W_NEXT_LEVEL)) {
     ClearBit(Packet3, W_NEXT_LEVEL);
     NextLevel = 0;
  }

  //Выключение дыммашины с задержкой
  if (SmokeTimer>1) {
    SmokeTimer--;
    if (SmokeTimer == 1) {
       ClearBit(Packet2, O_SMOKE_MACH);
       SmokeTimer = 0;
    }
  }

  //Обработка таймера
  //Осталось 5 мин 5MIN (нужно убить друга или погибнуть всем)
  if ((GameTimer==300) && (GameLevel==10)) {
     VideoNum = 6;
     SetBit(Packet3, W_PLAYER_STATE);
     LightMode = LM_MUTED;
     Old_GameLevel = GameLevel; GameLevel=12;  
  }
  
  //Время истекло FAIL2 (но друга не убили)
  if ((GameTimer==0) && (GameLevel==12)) {
     VideoNum = 8;
     SetBit(Packet3, W_PLAYER_STATE);
     LightMode = LM_MUTED;
     Old_GameLevel = GameLevel; GameLevel=11;  
  }
  
  //Время истекло, но до 3 комнаты добрались
  if ((GameTimer==0) && (GameLevel>7) && (GameLevel<=10) ) {
     VideoNum = 5;
     SetBit(Packet3, W_PLAYER_STATE);
     LightMode = LM_MUTED;
     Old_GameLevel = GameLevel; GameLevel=11;  
  } 
  
  //Время истекло, FAIL1 (до 3 комнаты не дошли)
  if ((GameTimer==0) && (GameLevel<=7)) {
     VideoNum = 5;
     SetBit(Packet3, W_PLAYER_STATE);
     LightMode = LM_MUTED;
     Old_GameLevel = GameLevel; GameLevel=11;  
  }
  
  if ((GameTimer==0) && (BitIsSet(Packet2, O_SMOKE_MACH))) {
     ClearBit(Packet2, O_SMOKE_MACH);
  }
  

  //Установка портов выходов в соответствии с пакетом выходов
  //@inzheneher: 2016-06-27, используем свои реле, которые начинаются с 36 пина, количество то же - 14 шт.
  for (int i=0; i <= 13; i++){
     if (BitIsSet(Packet2, i)) {

       switch (i) {
         case O_EM_DOOR1:
              if (BitIsSet(Packet3, W_BLOK_DOOR1)) {
                digitalWrite(36+i, HIGH);
              } else {
                digitalWrite(36+i, LOW);
              }   
              break;
         case O_EM_DOOR2:
               if (BitIsSet(Packet3, W_BLOK_DOOR2)) {
                digitalWrite(36+i, HIGH);
              } else {
                digitalWrite(36+i, LOW);
              }   
              break;
         case O_EM_DOOR3:
              if (BitIsSet(Packet3, W_BLOK_DOOR3)) {
                digitalWrite(36+i, HIGH);
              } else {
                digitalWrite(36+i, LOW);
              }   
              break;
         default:
              digitalWrite(36+i, HIGH);
              break;
       }

     } else {
       digitalWrite(36+i, LOW);
     }
  }


  //Передаем значения пакетов в на OPC-сервер через MODBUS
 
  holdingRegs[GAME_LEVEL] = GameLevel;
  holdingRegs[LIGHT_MODE] = LightMode;
  holdingRegs[VIDEO_NUM]  = VideoNum;
  
  holdingRegs[PACKET_1] = Packet1;
  holdingRegs[PACKET_2] = Packet2;
  holdingRegs[PACKET_3] = Packet3;
  

  //Обновляем состояние MODBUS
  modbus_update();
  
}

