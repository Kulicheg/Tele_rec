// AA - 170
// AB - 171
// BA - 186
// BB - 187


#include <AT24C256.h>

int Cycles;
byte currentByte;
byte header [4] = {170, 171, 186, 187};
byte command;
byte blocksize = 128;
byte numBytes;

AT24C256 drive(0x50);

void setup() {
  delay(2000);
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("Starting");

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
      Serial.print("01:");




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
