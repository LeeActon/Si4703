#include "Arduino.h"
#include "Si4703.h"
#include "Wire.h"

//-----------------------------------------------------------------------------------------------------------------------------------
// Radio Initialization 
//-----------------------------------------------------------------------------------------------------------------------------------
Si4703::Si4703(int resetPin, int sdioPin, int sclkPin, int stcIntPin)
{
  _resetPin   = resetPin;   // Reset pin
  _sdioPin    = sdioPin;    // I2C Data IO Pin
  _sclkPin    = sclkPin;    // I2C Clock Pin
  _stcIntPin  = stcIntPin;  // Seek/Tune Complete Pin
}

//-----------------------------------------------------------------------------------------------------------------------------------
// Read the entire register control set from 0x00 to 0x0F
//-----------------------------------------------------------------------------------------------------------------------------------
void Si4703::readRegisters(){

  // Si4703 begins reading from register upper register of 0x0A and reads to 0x0F, then loops to 0x00.
  Wire.requestFrom(I2C_ADDR, 32); //We want to read the entire register set from 0x0A to 0x09 = 32 bytes.

  // Remember, register 0x0A comes in first so we have to shuffle the array around a bit
  for(int x = 0x0A ; ; x++) 
    {                       //Read in these 32 bytes
      if(x == 0x10) x = 0;  //Loop back to zero
      si4703_registers[x] = Wire.read() << 8;
      si4703_registers[x] |= Wire.read();
      if(x == 0x09) break; 
    }
}

//-----------------------------------------------------------------------------------------------------------------------------------
// Write the current 9 control registers (0x02 to 0x07) to the Si4703
// It's a little weird, you don't write an I2C addres
// The Si4703 assumes you are writing to 0x02 first, then increments
//-----------------------------------------------------------------------------------------------------------------------------------
byte Si4703::updateRegisters() {

  Wire.beginTransmission(I2C_ADDR);
  // A write command automatically begins with register 0x02 so no need to send a write-to address
  // First we send the 0x02 to 0x07 control registers
  // In general, we should not write to registers 0x08 and 0x09
  for(int regSpot = 0x02 ; regSpot < 0x08 ; regSpot++) {
    byte high_byte = si4703_registers[regSpot] >> 8;
    byte low_byte = si4703_registers[regSpot] & 0x00FF;

    Wire.write(high_byte); //Upper 8 bits
    Wire.write(low_byte); //Lower 8 bits
  }

  // End this transmission
  byte ack = Wire.endTransmission();
  if(ack != 0) { //We have a problem! 
    return(FAIL);
  }

  return(SUCCESS);
}

//-----------------------------------------------------------------------------------------------------------------------------------
// To get the Si4703 in to 2-wire mode, SEN needs to be high and SDIO needs to be low after a reset
// The breakout board has SEN pulled high, but also has SDIO pulled high. Therefore, after a normal power up
// The Si4703 will be in an unknown state. RST must be controlled
//-----------------------------------------------------------------------------------------------------------------------------------
void Si4703::si4703_init() 
{
  pinMode(_resetPin , OUTPUT);      // Reset pin
  pinMode(_sdioPin  , OUTPUT);      // I2C data io pin
  pinMode(_stcIntPin, OUTPUT);	    // STC (search/tune complete) interrupt pin

  digitalWrite(_sdioPin   , LOW);   // A low SDIO indicates a 2-wire interface
  digitalWrite(_resetPin  , LOW);   // Put Si4703 into reset
  digitalWrite(_stcIntPin , HIGH);  // STC goes low on interrupt
  delay(1);                         // Some delays while we allow pins to settle

  digitalWrite(_resetPin  , HIGH);  // Bring Si4703 out of reset with SDIO set to low and SEN pulled high with on-board resistor
  delay(1);                         // Allow Si4703 to come out of reset

  Wire.begin();                     // Now that the unit is reset and I2C inteface mode, we need to begin I2C

  readRegisters();                  // Read the current register set
  si4703_registers[0x07] = 0x8100;  // Enable the oscillator, from AN230 page 9, rev 0.61 (works)
  si4703_registers[0x04] |= 0x2000; // Set bit 14 to high to enable STC Interrupt on GPIO2
  updateRegisters();                // Update

  delay(500);                       //Wait for clock to settle - from AN230 page 9

  readRegisters();                  //Read the current register set
  
  si4703_registers[POWERCFG] = 0x4001; //Enable the IC
  //  si4703_registers[POWERCFG] |= (1<<SMUTE) | (1<<DMUTE); //Disable Mute, disable softmute
  si4703_registers[SYSCONFIG1] |= (1<<RDS);     //Enable RDS
  si4703_registers[SYSCONFIG1] |= (1<<DE);      //50kHz Europe setup
  si4703_registers[SYSCONFIG2] |= (1<<SPACE0);  //100kHz channel spacing for Europe
  si4703_registers[SYSCONFIG2] &= 0xFFF0;       //Clear volume bits
  si4703_registers[SYSCONFIG2] |= 0x0001;       //Set volume to lowest

  updateRegisters();                            //Update
  delay(110);                                   //Max power up time, from datasheet page 13
}

//-----------------------------------------------------------------------------------------------------------------------------------
// Power On 
//-----------------------------------------------------------------------------------------------------------------------------------
void Si4703::powerOn()
{
    si4703_init();
}

//-----------------------------------------------------------------------------------------------------------------------------------
// Set Channel
//-----------------------------------------------------------------------------------------------------------------------------------
void Si4703::setChannel(int channel)
{
  // Europe Freq MHz = 0.100 MHz * Channel + 87.5 MHz
  // US     Freq MHz = 0.200 MHz * Channel + 87.5 MHz
  
  int newChannel = channel - 875;
      
  //These steps come from AN230 page 20 rev 0.5
  readRegisters();
  si4703_registers[CHANNEL] &= 0xFE00;      //Clear out the channel bits
  si4703_registers[CHANNEL] |= newChannel;  //Mask in the new channel
  si4703_registers[CHANNEL] |= (1<<TUNE);   //Set the TUNE bit to start
  updateRegisters();

  //delay(60); //Wait 60ms - you can use or skip this delay

  while(_stcIntPin == 1) {}	//Wait for interrupt indicating STC (Seek/Tune Complete)

  readRegisters();
  si4703_registers[CHANNEL] &= ~(1<<TUNE); //Clear the tune after a tune has completed
  updateRegisters();

  //Wait for the si4703 to clear the STC as well
  while(1) {
    readRegisters();
    if( (si4703_registers[STATUSRSSI] & (1<<STC)) == 0) break; //Tuning complete!
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------
// Reads the current channel from READCHAN
// Returns a number like 973 for 97.3MHz
//-----------------------------------------------------------------------------------------------------------------------------------
int Si4703::getChannel() {
  readRegisters();

  // Europe Freq MHz = 0.100 MHz * Channel + 87.5 MHz
  // US     Freq MHz = 0.200 MHz * Channel + 87.5 MHz
  
  int channel = si4703_registers[READCHAN] & 0x03FF;  // Mask out everything but the lower 10 bits
      channel += 875;                                 //98 + 875 = 973

  return(channel);
}

//-----------------------------------------------------------------------------------------------------------------------------------
// Seeks out the next available station
// Returns the freq if it made it
// Returns zero if failed
//-----------------------------------------------------------------------------------------------------------------------------------
int Si4703::seek(byte seekDirection){
  readRegisters();
  // Set seek mode wrap bit
  si4703_registers[POWERCFG] |= (1<<SKMODE); //Allow wrap

  // si4703_registers[POWERCFG] &= ~(1<<SKMODE); //Disallow wrap - if you disallow wrap, you may want to tune to 87.5 first
  if(seekDirection == SEEK_DOWN) si4703_registers[POWERCFG] &= ~(1<<SEEKUP); //Seek down is the default upon reset
  else si4703_registers[POWERCFG] |= 1<<SEEKUP; //Set the bit to seek up

  si4703_registers[POWERCFG] |= (1<<SEEK); //Start seek
  updateRegisters(); // Seeking will now start

  while(_stcIntPin == 1) {} // Wait for interrupt indicating STC (Seek/Tune complete)

  readRegisters();
  int valueSFBL = si4703_registers[STATUSRSSI] & (1<<SFBL); //Store the value of SFBL
  si4703_registers[POWERCFG] &= ~(1<<SEEK); // Clear the seek bit after seek has completed
  updateRegisters();

  //Wait for the si4703 to clear the STC as well
  while(1) {
    readRegisters();
    if( (si4703_registers[STATUSRSSI] & (1<<STC)) == 0) break; // Tuning complete!
  }

  if(valueSFBL) { // The bit was set indicating we hit a band limit or failed to find a station
    return(0);
  }
return getChannel();
}

//----------------------------------------------------------------------------------------------------------------------------------
// Seek Up
//-----------------------------------------------------------------------------------------------------------------------------------
int Si4703::seekUp()
{
	return seek(SEEK_UP);
}

//-----------------------------------------------------------------------------------------------------------------------------------
// Seek Down
//-----------------------------------------------------------------------------------------------------------------------------------
int Si4703::seekDown()
{
	return seek(SEEK_DOWN);
}

//-----------------------------------------------------------------------------------------------------------------------------------
// Set Volume
//-----------------------------------------------------------------------------------------------------------------------------------
void Si4703::setVolume(int volume)
{
  readRegisters();                        // Read the current register set
  if (volume < 0 ) volume = 0;            // Accepted Volume value 0-15
  if (volume > 15) volume = 15;           // Accepted Volume value 0-15
  si4703_registers[SYSCONFIG2] &= 0xFFF0; // Clear volume bits
  si4703_registers[SYSCONFIG2] |= volume; // Set new volume
  updateRegisters();                      // Update
}

//-----------------------------------------------------------------------------------------------------------------------------------
// Read RDS
//-----------------------------------------------------------------------------------------------------------------------------------
void Si4703::readRDS(char* buffer, long timeout)
{ 
	long    endTime         = millis() + timeout;
  boolean completed[]     = {false, false, false, false};
  int     completedCount  = 0;

  while(completedCount < 4 && millis() < endTime) {
    
    readRegisters();
    
    if(si4703_registers[STATUSRSSI] & (1<<RDSR)){
      // ls 2 bits of B determine the 4 letter pairs
      // once we have a full set return
      // if you get nothing after 20 readings return with empty string
      uint16_t b = si4703_registers[RDSB];
      int index = b & 0x03;
      if (! completed[index] && b < 500)
      {
        completed[index] = true;
        completedCount   ++;
        char Dh = (si4703_registers[RDSD] & 0xFF00) >> 8;
        char Dl = (si4703_registers[RDSD] & 0x00FF);
        buffer[index * 2]     = Dh;
        buffer[index * 2 +1]  = Dl;

        // Serial.print(si4703_registers[RDSD]); Serial.print(" ");
        // Serial.print(index);Serial.print(" ");
        // Serial.write(Dh);
        // Serial.write(Dl);
        // Serial.println();
      }

      delay(40); //Wait for the RDS bit to clear
    }
    
    else {
      delay(30); //From AN230, using the polling method 40ms should be sufficient amount of time between checks
    }

  }

	if (millis() >= endTime) {
		buffer[0] ='\0';
		return;
	}

  buffer[8] = '\0';
}

//-----------------------------------------------------------------------------------------------------------------------------------
// Writes GPIO1-3
//-----------------------------------------------------------------------------------------------------------------------------------
	void	Si4703::writeGPIO(int GPIO, int val)
{
  readRegisters();                          // Read the current register set

  if (val==GPIO_Z) {

    si4703_registers[SYSCONFIG1] &= ~GPIO;  // Clear GPIO bits (00)
  }

  else if (val==GPIO_I) {

    si4703_registers[SYSCONFIG1] &= ~GPIO;  // Clear GPIO bits
    GPIO &= 0b00010101;
    si4703_registers[SYSCONFIG1] |= GPIO;   // Set GPIO bits to (01)
  }

  else if (val==GPIO_Low) {

    si4703_registers[SYSCONFIG1] &= ~GPIO;  // Clear GPIO bits
    GPIO &= 0b00101010;
    si4703_registers[SYSCONFIG1] |= GPIO;   // Set GPIO bits to (10)
  }

  else if (val==GPIO_High) {

    si4703_registers[SYSCONFIG1] |= GPIO;   // Set GPIO bits (11)
  }

  else {
    
    Serial.println("Error undefined GPIO value!");
  }
  
  updateRegisters();
}

//-----------------------------------------------------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------------------------------------------------