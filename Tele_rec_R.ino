

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

int Cycles;
byte currentByte;
byte header [4] = {170, 171, 186, 187}; // AA AB BA BB
byte drvlabel [3] = {68, 72, 86,};      // D r V
byte command;
byte numBytes;
byte NumRec;
byte blocksize = 128;
byte PackSize = 22;
byte JournalSize = 29;
byte ServiceSize = 78;
int TelemetryBlock;
int JournalBlock;
byte filesD;
int nextblock;
int ATPos;
bool result;
bool inProgress;
FM24C256 driveD(0x50);

void setup() {
  delay(2000);
  Serial.begin(9600);
  Wire.begin();
  Serial1.begin(9600);
  Serial.println("Starting");



//    result = formatdriveD();
//    if (result)
//    {
//      Serial.println ("Formated drive D");
//    }
//    else Serial.println ("Not formated drive D");


  result = getdriveDinfo();
  if (result)
  {
    Serial.println ("Good drive D");
  }
  else Serial.println ("BAD drive D");

}

void loop()
{

  
    command = headerdetector();
  Serial.print("command ");
 Serial.println(command);

  switch (command)

  {
    case 01:
      Serial.print("01:");
      // delay (5);
      numBytes = 0;

      NumRec = Serial1.read();

      byte CyclesH = Serial1.read();
      byte CyclesL = Serial1.read();
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


      int blocklenght = (Cycles * PackSize) + (NumRec * JournalSize) + ServiceSize;
      int blockend = ATPos + blocklenght;



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

      Serial.println  ("");
      Serial.print    ("FileName:");
      Serial.println  (filesD + 1);
      Serial.print    ("EEPROM Start pos:");
      Serial.println  (ATPos);
      Serial.print    ("EEPROM finish pos:");
      Serial.println  (blockend);

      inProgress = true;
Serial.println  ("Finish 01...");
      break;

    case 02:
    Serial.println("");
      Serial.println("!!!02:");


      if (!inProgress)
      {
        Serial.println  ("Open transaction first...");
        break;
      }
Serial.println  ("Get Cycles...");

      for ( int q = 0; q < Cycles * PackSize; q++)
      {
        byte readbyte = Serial1.read();

        driveD.write(ATPos++, readbyte);
      }

Serial.println  ("Get Journal...");

for ( int q = 0; q < NumRec * JournalSize; q++)
      {
        byte readbyte = Serial1.read();

        driveD.write(ATPos++, readbyte);
      }


Serial.println  ("Get Service...");

for ( int q = 0; q < ServiceSize; q++)
      {
        byte readbyte = Serial1.read();

        driveD.write(ATPos++, readbyte);
      }


      driveD.write (3, ++filesD);
      inProgress = 0;
      Serial.println  ("Finish transaction...");
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
Serial.println (readbyte);

    if (readbyte != drvlabel[q]) return false;
  }



  filesD = driveD.read(3);
  nextblock = filesD * 5 + 5;
  ATPos = 128;

  if (filesD > 0)
  {
    for (int q = 0; q < (filesD * 5); q = q + 5)
    {
      Serial.println(q);

      byte readbyteH = driveD.read(5);
      byte readbyteL = driveD.read(6);
    }

  }
  //  Wire.write((int)(eeaddress >> 8));
  //  Wire.write((int)(eeaddress & 0xFF))

  return true;

}

bool formatdriveD()
{
  ///////////////////////////Clear//////////////////////////////////

  for (long int q = 0; q < 32768; q++)
  {
    driveD.write (q, 00);
    float disp = q % 1000;
    if ( disp == 0) {
      Serial.println (q);
    }
  }

  Serial.println ( "FINISH... ");

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

    return true;
  }

}

