// AA - 170
// AB - 171
// BA - 186
// BB - 187


#include <AT24C256.h>

int Cycles;
byte currentByte;
byte header [4] = {170, 171, 186, 187};
byte command;



AT24C256 drive(0x50);

void setup() {
Serial.begin(9600);


  

}










void loop()
{




for (byte q = 0; q < 4; q++)

  {

Serial.write (header[q]);
//Serial.println (header[q], HEX);

  }
 
  
  randomSeed(millis());
  int q = random(1, 3);
  Serial.write (q);
//Serial.print (q, HEX );




}
