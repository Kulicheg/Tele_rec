

// AA - 170
// AB - 171
// BA - 186
// BB - 187

/////////////////////////////////////////////////////////////////////////////
//Оглавление хранит в первом байте сколько всего записей сразу после метки DrV. Далее с четвертого байта  по 4 байта вычисленные старт-финиш позиции в хранилище, например 00128 - 1445, 1446 - 2487
//Сама запись начинается с четырех байт оглавления Номер записи, Cycles записи и RecNum записи. Всего разрешим хранить 31 запись + первая служебная  и вы делим соответственно 128 байт.
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
int TelemetryBlock;
int JournalBlock;
byte filesD;
int nextblock;



FM24C256 driveD(0x50);

void setup() {
  delay(2000);
  Serial.begin(9600);
  Wire.begin();
  Serial1.begin(9600);
  Serial.println("Starting");

  bool result;


  //  result = formatdriveD();
  //  if (result)
  //  {
  //    Serial.println ("Formated drive D");
  //  }
  //  else Serial.println ("Not formated drive D");


  result = getdriveDinfo();
  if (result)
  {
    Serial.println ("Good drive D");
  }
  else Serial.println ("BAD drive D");
  delay(50000);
}

void loop()
{

  do
  {
    command = headerdetector();
  }
  while (command == 255);

  switch (command)

  {
    case 01:
      // delay (5);
      numBytes = 0;

      NumRec = Serial1.read();

      byte CyclesH = Serial1.read();
      byte CyclesL = Serial1.read();
      Cycles = word(CyclesH, CyclesL);

      TelemetryBlock = Cycles * PackSize;
      JournalBlock = NumRec * JournalSize;


      Serial.print("01:");
      Serial.print(Cycles);
      Serial.print("/");
      Serial.println(NumRec);



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

    case 02:
      Serial.println("02:");
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
  nextblock = filesD * 4 + 4;

  for (int q = 0; q < (filesD*4); q=q+4)
  

  {
    byte readbyteH = driveD.read(5);
    byte readbyteL = driveD.read(6);
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



