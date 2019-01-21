// AA - 170
// AB - 171
// BA - 186
// BB - 187

#include <EEPROM.h>
#include <AT24C256.h>
#include <Wire.h>

int Cycles = 400;
int PackSize = 22;
int NumRec = 12;
byte JournalSize = 29;

byte currentByte;
byte header [4] = {170, 171, 186, 187};
byte command;
int C_drive_c = EEPROM.length();
int C_drive_d = 32768;


AT24C256 driveD(0x50);

void setup()
{

  Serial.begin(9600);
  Wire.begin();

}

void loop()
{

  sendheader(01);


  Serial.write (NumRec);

  Serial.write (highByte(Cycles));
  Serial.write (lowByte(Cycles));

  Serial.flush();

  delay (500);

  sendheader(02);

  //tone (3, 1000, 50);
 
  for ( int q = 0; q < Cycles * PackSize; q++)
  {
    byte Sendbyte = driveD.read (q);
    Serial.write (Sendbyte);


  }


  Serial.flush();
  //tone (3, 1000, 50);


  for ( int q = 0; q < NumRec * JournalSize; q++)
  {
    byte Sendbyte = EEPROM.read (q);
    Serial.write (Sendbyte);

  }
  Serial.flush();
  //tone (3, 1000, 50);



  for ( int q = 945; q < 1024 ; q++)
  {
    byte Sendbyte = EEPROM.read (q);
    Serial.write (Sendbyte);
  }


  Serial.flush();

  //tone (3, 1000, 50);
  delay (10000);


  //Serial.println(eeprom_crc(), HEX);

}




unsigned long eeprom_crc(void) {

  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  unsigned long crc = ~0L;

  for (int index = 0 ; index < EEPROM.length()  ; ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  return crc;
}


void sendheader(byte command)

{

  for (byte q = 0; q < 4; q++)
  {
    Serial.write (header[q]);
  }
  Serial.write (command);
}

