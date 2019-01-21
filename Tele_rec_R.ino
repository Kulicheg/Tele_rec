

// AA - 170
// AB - 171
// BA - 186
// BB - 187

/////////////////////////////////////////////////////////////////////////////
//Оглавление хранит в  четвертом байте сколько всего записей сразу после метки DrV. Пятый байт в резерве.  Далее с пятого байта  по 5 байт, 1 байт номер записи, дале 2 по 2  вычисленные старт-финиш позиции в хранилище, например 00128 - 1445, 1446 - 2487
//Сама запись начинается с четырех байт оглавления Номер записи, Cycles записи и RecNum записи. Всего разрешим хранить 31 запись + первая служебная  и  делим соответственно 160 байт оглавления.
//Далее идут 3 массива данных  из AT берем все байты с 0000  по Cycles * PackSize, из EEPROM передаем NumRec * 29(LogSize), а далее берем массив служебных данных на всякий из EEPROM 0945 - 1024
//
//
//
//00000 - 00128   Оглавление
//00129 - 32000   Данные для хранения
//32001 - 32767   Зарезервировано
///////////////////////////////////////////////////////////////////////////

#include <FM24C256.h>
#include <Wire.h>






int EEXPos;
int Altitude;
float Alt1, Alt2;
long int Pressure;
boolean writeit;
boolean powerLost;

int Apogee;
int bx, by, bz;
int bax, bay, baz;
float cx1, cy1, cz1;
double xyz[3];
double ax, ay, az;
int x, y, z;
int Temperature;
int Speed;
int bufwrite;
int msgCount;

boolean Fallen;
//byte NumRec;
int xCal, yCal, zCal, axCal, ayCal, azCal;
long int Start, Start2, Finish, Finish2, routineTime;
long int FirstTime, SecondTime, oldAltitude, newAltitude, SecondTimeM, FirstTimeM;
byte MOSFET_1, MOSFET_2, MOSFET_3;
boolean MOSFET_1_IS_FIRED, MOSFET_2_IS_FIRED, MOSFET_3_IS_FIRED;
int Maxspeed;















struct telemetrystruct
{
  int bax, bay, baz;
  int bx, by, bz;
  long int Pressure;
  int Temperature;
  int Altitude;
  int  Speed;
};

struct SystemLog
{
  unsigned long timestamp;
  char message[25];
};

struct telemetrystruct telemetry;
struct SystemLog capitansLog;

int Cycles;
byte currentByte;
byte header [4] = {170, 171, 186, 187}; // AA AB BA BB
byte drvlabel [3] = {68, 72, 86,};      // D r V
byte command;
byte numBytes;
byte NumRec;
byte blocksize = 128;
byte PackSize = sizeof (telemetry);
byte JournalSize = sizeof (capitansLog);
byte ServiceSize = 78;
int TelemetryBlock;
int JournalBlock;
byte filesD;
int nextblock;
unsigned int ATPos;
bool result;
bool inProgress;
unsigned int blockend;
int blocklenght;
int CyclesL;
int CyclesH;
unsigned int w;

FM24C256 driveD(0x50);

void setup() {
  delay(2000);
  Serial.begin(9600);
  Wire.begin();
  Serial1.begin(9600);
  Serial.println("Starting");

  Serial.println(sizeof (ATPos));


  result = formatdriveD();
  if (result)
  {
    Serial.println ("Formated drive D");
  }
  else Serial.println ("Not formated drive D");


  result = getdriveDinfo();
  if (result)
  {
    Serial.println ("Good drive D");
  }
  else Serial.println ("BAD drive D");

  driveD.write(3, 3);
  //fromLog ();
  getInfo2();

  Serial.println ("Break");
  while (1);

}

void loop()
{


  command = headerdetector();

  switch (command)

  {
    case 01:
      Serial.print("01:");
      // delay (5);
      numBytes = 0;

      NumRec = Serial1.read();

      CyclesH = Serial1.read();
      CyclesL = Serial1.read();
      Cycles = word(CyclesH, CyclesL);

      TelemetryBlock = Cycles * PackSize;
      JournalBlock = NumRec * JournalSize;



      Serial.print(Cycles);
      Serial.print("/");
      Serial.println(NumRec);

      result = getdriveDinfo();
      if (result)
      {
        Serial.println ("Good drive D");
      }
      else Serial.println ("BAD drive D");

      ////////////////////////////////// Records Counter //////////////////////////////////
      //driveD.write (3, ++filesD);
      ////////////////////////////////////////////////////////////////////////////////////


      blocklenght = (Cycles * PackSize) + (NumRec * JournalSize) + ServiceSize + 4;
      blockend = ATPos + blocklenght;


      Serial.println  ("");
      Serial.print    ("FileName:");
      Serial.println  (filesD + 1);
      Serial.print    ("EEPROM Start pos:");
      Serial.println  (ATPos);
      Serial.print    ("EEPROM finish pos:");
      Serial.println  (blockend);



      if (blockend > 32767)
      {
        Serial.println    ("NOT ENOUGHT SPACE FOR NEXT PACKET");
        while (1);
      }

      ////////////////////////////////// Directory //////////////////////////////////
      driveD.write (nextblock++, filesD + 1);

      driveD.write(nextblock++, (int)(ATPos >> 8));
      driveD.write(nextblock++, (int)(ATPos & 0xFF));

      driveD.write(nextblock++, (int)(blockend >> 8));
      driveD.write(nextblock++, (int)(blockend & 0xFF));
      //////////////////////////////////////////////////////////////////////////////



      driveD.write (ATPos++, filesD + 1);

      driveD.write(ATPos++, (int)(Cycles >> 8));
      driveD.write(ATPos++, (int)(Cycles & 0xFF));

      driveD.write(ATPos++, NumRec);



      inProgress = true;
      break;

    case 02:
      Serial.println("");
      Serial.println("02:");


      if (!inProgress)
      {
        Serial.println  ("Open transaction first...");
        break;
      }
      Serial.print  ("Get Cycles...");
      Serial.println  (ATPos);

      w = 0;
      while (w < Cycles * PackSize)
      {
        if (Serial1.available())
        {
          byte readbyte = Serial1.read();
          driveD.write(ATPos++, readbyte);
          w++;
        }
      }

      Serial.print  ("Get Journal... ");
      Serial.println  (ATPos);

      w = 0;
      while (w < JournalBlock)
      {
        if (Serial1.available())
        {
          byte readbyte = Serial1.read();
          driveD.write(ATPos++, readbyte);
          w++;
        }
      }

      Serial.print  ("Get Service...");
      Serial.println  (ATPos);

      w = 0;
      while (w < ServiceSize)
      {
        if (Serial1.available())
        {
          byte readbyte = Serial1.read();
          driveD.write(ATPos++, readbyte);
          w++;
        }
      }

      driveD.write (3, filesD + 1);
      inProgress = 0;
      Serial.print  ("Finish transaction...");
      Serial.println  (ATPos);
      break;



    case 03:
      Serial.println("03:");



      while (numBytes < 128)
      {
        if (Serial1.available())
        {

          byte recbyte =  Serial1.read();
          numBytes++;
          Serial.print (recbyte, HEX);
          Serial.print (" ");

        }
      }

      Serial.println ("");
      break;




  }



  //delay (1000);




}

byte headerdetector()
{
  byte command;
  byte flag = 0;

  while (flag < 4)
  {
    if (Serial1.available())




    { currentByte = Serial1.read();

      if (currentByte == header [flag]) flag++; else flag = 0;
    }
  }

  delay (5);
  if (Serial1.available()) command = Serial1.read();

  if (command > 8)
  {
    Serial.print ("Invalid command: ");
    Serial.println (command);
    command = 255;
  }

  return command;
}


bool getdriveDinfo()
{
  byte readbyte;
  for (int q = 0; q < 3; q++)
  {
    readbyte = driveD.read(q);


    if (readbyte != drvlabel[q]) return false;
  }



  filesD = driveD.read(3);

  nextblock = filesD * 5 + 5;
  ATPos = 160;

  if (filesD > 0)
  {

    byte readbyteH = driveD.read(nextblock - 2);
    byte readbyteL = driveD.read(nextblock - 1);


    ATPos = word (readbyteH, readbyteL) + 1;
  }
  return true;
}

bool formatdriveD()
{
  ///////////////////////////Clear//////////////////////////////////

  //  for (long int q = 0; q < 32768; q++)
  //  {
  //    driveD.write (q, 00);
  //    float disp = q % 1000;
  //    if ( disp == 0) {
  //      Serial.println (q);
  //    }
  //  }
  //
  //  Serial.println ( "FINISH... ");
  driveD.write(3, 0);

  ///////////////////////////Labeling////////////////////////////////

  for (int q = 0; q < 3; q++)
  {
    driveD.write(q, drvlabel[q]);
  }

  ///////////////////////////Verify//////////////////////////////////

  for (int q = 0; q < 3; q++)
  {
    byte readbyte = driveD.read(q);
    if (readbyte != drvlabel[q]) return false;
  }
  return true;
}


void fromLog ()
{
  byte Packet[JournalSize];

  byte NumRec2 = driveD.read(3);

  unsigned int startblock;
  unsigned int endblock;
  for (int q = 0; q < NumRec2; q++)
  {


    nextblock = q * 5 + 10;

    byte readbyteH = driveD.read(nextblock - 2);
    byte readbyteL = driveD.read(nextblock - 1);

    endblock  = word (readbyteH, readbyteL);

    readbyteH = driveD.read(nextblock - 4);
    readbyteL = driveD.read(nextblock - 3);


    startblock  = word (readbyteH, readbyteL);

    readbyteH = driveD.read(startblock + 1);
    readbyteL = driveD.read(startblock + 2);

    Cycles  = word (readbyteH, readbyteL);

    byte  NR = driveD.read(startblock + 3);


    ATPos = startblock + (Cycles * PackSize) + 4;

    Serial.println ("");
    Serial.print ("FileName=");
    Serial.println (NumRec2);
    Serial.print ("Cycles=");
    Serial.println (Cycles);
    Serial.print ("NR=");
    Serial.println (NR);
    Serial.print ("ATPos=");
    Serial.println (ATPos);
    Serial.print ("startblock=");
    Serial.println (startblock);
    Serial.print ("endblock=");
    Serial.println (endblock);

    Serial.println("");
    Serial.println("-----------------------------------------------");

    for (int msgCount = 0; msgCount < (JournalSize * NR);  msgCount = msgCount + JournalSize)
    {


      for (int intern = 0; intern < JournalSize; intern++)
      {
        Packet[intern] = driveD.read(ATPos + intern + msgCount);
      }

      memcpy(&capitansLog, Packet, sizeof(capitansLog));
      String str(capitansLog.message);
      Serial.print(capitansLog.timestamp);
      Serial.print(":");
      Serial.println(str);
      Serial.println("-----------------------------------------------");



    }

  }
}





void getInfo2()
{
  int  EEXPos = 164;
  //  EEPROM.get (930, Apogee);
  //  Serial.print ("Apogee = ");
  //  Serial.println (Apogee);
  //
  //  EEPROM.get (950, Maxspeed);
  //  Serial.print ("Max Speed = ");
  //  Serial.println (Maxspeed);

  PackSize = sizeof (telemetry);
  byte Packet[PackSize];





  byte NumRec2 = driveD.read(3);

  unsigned int startblock;
  unsigned int endblock;

  for (int q = 0; q < NumRec2; q++)
  {


    nextblock = q * 5 + 10;

    byte readbyteH = driveD.read(nextblock - 2);
    byte readbyteL = driveD.read(nextblock - 1);

    endblock  = word (readbyteH, readbyteL);

    readbyteH = driveD.read(nextblock - 4);
    readbyteL = driveD.read(nextblock - 3);


    startblock  = word (readbyteH, readbyteL);

    readbyteH = driveD.read(startblock + 1);
    readbyteL = driveD.read(startblock + 2);

    Cycles  = word (readbyteH, readbyteL);

    byte  NR = driveD.read(startblock + 3);



    ATPos = startblock + 4;

    Serial.println ("");
    Serial.print ("FileName=");
    Serial.println (q);
    Serial.print ("Cycles=");
    Serial.println (Cycles);
    Serial.print ("NR=");
    Serial.println (NR);
    Serial.print ("ATPos=");
    Serial.println (ATPos);
    Serial.print ("startblock=");
    Serial.println (startblock);
    Serial.print ("endblock=");
    Serial.println (endblock);

    Serial.println("");
    Serial.println ("----------------------------------------------------------------------------");

    Serial.println ("Alt\t Spd\t Prs\t Tmp\t bx\t by\t bz\t gX\t gY\t gZ");
    Serial.println (" ");



    for (int Rec = 0; Rec < Cycles; Rec++)
    {
      for (int intern = 0; intern < PackSize; intern++)
      {
        Packet[intern] = driveD.read(ATPos + intern);
      }

      memcpy(&telemetry, Packet, sizeof(telemetry));




      bx            =  telemetry.bx;
      by            =  telemetry.by;
      bz            =  telemetry.bz;

      float DT_bx = bx;
      float DT_by = by;
      float DT_bz = bz;

      DT_bx = DT_bx / 10;
      DT_by = DT_by / 10;
      DT_bz = DT_bz / 10;


      bax           = telemetry.bax;
      bay           = telemetry.bay;
      baz           = telemetry.baz;


      Pressure      = telemetry.Pressure;
      Temperature   = telemetry.Temperature;
      Altitude      = telemetry.Altitude;
      Speed         = telemetry.Speed;
      float Speed2  =  Speed;



      Serial.print (Altitude);
      Serial.print("\t");
      Serial.print (Speed2);
      Serial.print("\t");
      Serial.print (Pressure);
      Serial.print("\t");
      Serial.print (Temperature);
      Serial.print("\t");
      Serial.print (DT_bx, 2);
      Serial.print("\t");
      Serial.print (DT_by, 2);
      Serial.print("\t");
      Serial.print (DT_bz, 2);
      Serial.print("\t");
      Serial.print (round(bax));
      Serial.print("\t");
      Serial.print (round (bay));
      Serial.print("\t");
      Serial.println (round (baz));

     ATPos = ATPos + PackSize; 
    }
    Serial.println ("----------------------------------------------------------------------------");

  }

}


void getInfo2()
{
  int  EEXPos = 164;
  

  PackSize = sizeof (telemetry);
  byte Packet[PackSize];





  byte NumRec2 = driveD.read(3);

  unsigned int startblock;
  unsigned int endblock;

  for (int q = 0; q < NumRec2; q++)
  {


    nextblock = q * 5 + 10;

    byte readbyteH = driveD.read(nextblock - 2);
    byte readbyteL = driveD.read(nextblock - 1);

    endblock  = word (readbyteH, readbyteL);

    readbyteH = driveD.read(nextblock - 4);
    readbyteL = driveD.read(nextblock - 3);


    startblock  = word (readbyteH, readbyteL);

    readbyteH = driveD.read(startblock + 1);
    readbyteL = driveD.read(startblock + 2);

    Cycles  = word (readbyteH, readbyteL);

    byte  NR = driveD.read(startblock + 3);



    ATPos = startblock + 4;

    Serial.println ("");
    Serial.print ("FileName=");
    Serial.println (q);
    Serial.print ("Cycles=");
    Serial.println (Cycles);
    Serial.print ("NR=");
    Serial.println (NR);
    Serial.print ("ATPos=");
    Serial.println (ATPos);
    Serial.print ("startblock=");
    Serial.println (startblock);
    Serial.print ("endblock=");
    Serial.println (endblock);

    Serial.println("");
    Serial.println ("----------------------------------------------------------------------------");

 int Offset = 945;
Apogee = 99999

//  EEPROM.get (930, Apogee);
  //  Serial.print ("Apogee = ");
  //  Serial.println (Apogee);
  //
  //  EEPROM.get (950, Maxspeed);
  //  Serial.print ("Max Speed = ");
  //  Serial.println (Maxspeed);



    

     ATPos = ATPos + PackSize; 
    }
    Serial.println ("----------------------------------------------------------------------------");

  }

}

