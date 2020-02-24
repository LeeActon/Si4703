/* 

Library Modified by Muthanna Alwahash 2020

This library based on the code produced by Nathan Seidle and Simon Monk.

Remarks:

	* The Si4703 ACKs the first byte, and NACKs the 2nd byte of a read.
	* Don't updateRegisters without first readRegisters.

*/

#ifndef Si4703_h
#define Si4703_h

#include "Arduino.h"


// BAND (0x05.7:6)—FM Band Select
static const uint16_t  	BAND_USEU		= 0;	// 87.5–108 MHz (US / Europe, Default)
static const uint16_t  	BAND_JPW		= 1;	// 76–108 MHz (Japan wide band)
static const uint16_t  	BAND_JP			= 2;	// 76–90 MHz (Japan)

// SPACE (0x05.5:4)—FM Channel Spacing
static const uint16_t  	SPACE_200KHz	= 0;	// 200 kHz (US / Australia, Default)
static const uint16_t  	SPACE_100KHz 	= 1;	// 100 kHz (Europe / Japan)
static const uint16_t  	SPACE_50KHz  	= 2;	//  50 kHz (Other)

// GPIO1-3 Pins
static const uint16_t  	GPIO1			= 0x0003;	// SYSCONFIG1.D0:D1
static const uint16_t  	GPIO2			= 0x00C0;	// SYSCONFIG1.D3:D4
static const uint16_t  	GPIO3			= 0x0300;	// SYSCONFIG1.D5:D6

// GPIO1-3 Possible Values
static const uint16_t 	GPIO_Z			= 0;	// High impedance (default)
static const uint16_t 	GPIO_I			= 1;	// GPIO1-Reserved, GPIO2-STC/RDS int, or GPIO3-Mono/Sterio Indicator
static const uint16_t 	GPIO_Low		= 2;	// Low output (GND level)
static const uint16_t 	GPIO_High		= 3;	// High output (VIO level)

//------------------------------------------------------------------------------------------------------------
union DEVICEID_t	// Register 0x00
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	MFGID	:12;
		uint16_t 	PN		:4;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union CHIPID_t		// Register 0x01
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	FIRMWARE:6;
		uint16_t 	DEV		:4;
		uint16_t 	REV		:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union POWERCFG_t	// Register 0x02
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	X:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union CHANNEL_t		// Register 0x03 
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	X:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union SYSCONFIG1_t	// Register 0x04
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	X:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union SYSCONFIG2_t	// Register 0x05
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	X:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union SYSCONFIG3_t	// Register 0x06
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	X:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union TEST1_t		// Register 0x07
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	X:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union TEST2_t		// Register 0x08
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	X:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union BOOTCONFIG_t	// Register 0x09
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	X:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union STATUSRSSI_t	// Register 0x0A
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	X:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union READCHAN_t	// Register 0x0B
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	X:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union RDSA_t		// Register 0x0C
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	X:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union RDSB_t		// Register 0x0D
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	X:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union RDSC_t		// Register 0x0E
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	X:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union RDSD_t		// Register 0x0F
{
	uint16_t 	bytes;

	struct bits
	{
		uint16_t	X:6;
	} 			bits;
};
//------------------------------------------------------------------------------------------------------------
union shadow_t
{
	uint16_t	si4703_registers[16];

	struct bytes
	{

		STATUSRSSI_t	STATUSRSSI;	// Register 0x0A
		READCHAN_t		READCHAN;	// Register 0x0B
		RDSA_t			RDSA;		// Register 0x0C
		RDSB_t			RDSB;		// Register 0x0D
		RDSC_t			RDSC;		// Register 0x0E
		RDSD_t			RDSD;		// Register 0x0F

		DEVICEID_t 		DEVICEID;	// Register 0x00
		CHIPID_t		CHIPID;		// Register 0x01
		POWERCFG_t 		POWERCFG;	// Register 0x02
		CHANNEL_t		CHANNEL;	// Register 0x03
		SYSCONFIG1_t	SYSCONFIG1;	// Register 0x04
		SYSCONFIG2_t	SYSCONFIG2;	// Register 0x05
		SYSCONFIG3_t	SYSCONFIG3;	// Register 0x06
		TEST1_t			TEST1;		// Register 0x07
		TEST2_t			TEST2;		// Register 0x08
		BOOTCONFIG_t	BOOTCONFIG;	// Register 0x09


	} bytes;

};
//------------------------------------------------------------------------------------------------------------

class Si4703
{
//------------------------------------------------------------------------------------------------------------
  public:
    Si4703(	int resetPin, 				// Reset pin
			int sdioPin,				// I2C Data IO Pin
			int sclkPin,				// I2C Clock Pin
			int stcIntPin);				// Seek/Tune Complete Pin

	deviceID_t 	getDeviceID();

	int		getChipID();
	
    void	powerOn();					// call in setup

	void	setChannel(int channel);  	// Set 3 digit channel number
	int 	getChannel();				// Get 3 digit channel number

	int 	seekUp(); 					// Seeks up and returns the tuned channel or 0
	int 	seekDown(); 				// Seeks down and returns the tuned channel or 0

	void	setVolume(int volume); 		// Sets volume value 0 to 15

	void	readRDS(char* message,		// Reads RDS, message should be at least 9 chars, result will be null terminated.
					long timeout);		// timeout in milliseconds

	void	writeGPIO(int GPIO, 		// Write to GPIO1,GPIO2, and GPIO3
					  int val); 		// values can be GPIO_Z, GPIO_I, GPIO_Low, and GPIO_High

//------------------------------------------------------------------------------------------------------------
  private:
    int  	_resetPin;					// Reset pin
	int  	_sdioPin;					// I2C Data IO Pin
	int  	_sclkPin;					// I2C Clock Pin
	int  	_stcIntPin;					// Seek/Tune Complete Pin

	void 	si4703_init();				// init class
	void 	readRegisters();			// Read registers to shadow 
	byte 	updateRegisters();			// Write registers from shadow
	int 	seek(byte seekDirection);
	
	// Registers shadow
	uint16_t 				si4703_registers[16]; 	// There are 16 registers, each 16 bits large

	// I2C interface
	static const int  		I2C_ADDR		= 0x10; // I2C address of Si4703 - note that the Wire function assumes non-left-shifted I2C address, not 0b.0010.000W
	static const uint16_t  	I2C_FAIL_MAX 	= 10; 	// This is the number of attempts we will try to contact the device before erroring out

	static const uint16_t  	FAIL 			= 0;
	static const uint16_t  	SUCCESS 		= 1;

	static const uint16_t  	SEEK_DOWN 		= 0; 	// Direction used for seeking. Default is down
	static const uint16_t  	SEEK_UP 		= 1;


	

	// Register names
	static const uint16_t  DEVICEID 	= 0x00;
	static const uint16_t  CHIPID 		= 0x01;
	static const uint16_t  POWERCFG 	= 0x02;
	static const uint16_t  CHANNEL 		= 0x03;
	static const uint16_t  SYSCONFIG1 	= 0x04;
	static const uint16_t  SYSCONFIG2 	= 0x05;
	static const uint16_t  SYSCONFIG3 	= 0x06;
	static const uint16_t  TEST1 		= 0x07;
	static const uint16_t  TEST2 		= 0x08;
	static const uint16_t  BOOTCONFIG	= 0x09;

	static const uint16_t  STATUSRSSI 	= 0x0A;
	static const uint16_t  READCHAN 	= 0x0B;
	static const uint16_t  RDSA 		= 0x0C;
	static const uint16_t  RDSB 		= 0x0D;
	static const uint16_t  RDSC 		= 0x0E;
	static const uint16_t  RDSD 		= 0x0F;

	// Register 0x02 - POWERCFG
	static const uint16_t  SMUTE 		= 15;
	static const uint16_t  DMUTE 		= 14;
	static const uint16_t  MONO			= 13;
	static const uint16_t  RDSM			= 11;
	static const uint16_t  SKMODE 		= 10;
	static const uint16_t  SEEKUP 		= 9;
	static const uint16_t  SEEK 		= 8;
	static const uint16_t  DISABLE		= 6;
	static const uint16_t  ENABLE 		= 0;

	// Register 0x03 - CHANNEL
	static const uint16_t  TUNE 		= 15;

	// Register 0x04 - SYSCONFIG1
	static const uint16_t  RDSIEN		= 15;
	static const uint16_t  STCIEN		= 14;
	static const uint16_t  RDS 			= 12;
	static const uint16_t  DE 			= 11;

	// Register 0x05 - SYSCONFIG2
	static const uint16_t  SPACE1 		= 5;
	static const uint16_t  SPACE0 		= 4;

	// Register 0x07 - TEST1
	static const uint16_t  XOSCEN		= 15;
	static const uint16_t  AHIZEN		= 14;

	// Register 0x0A - STATUSRSSI
	static const uint16_t  RDSR 		= 15;
	static const uint16_t  STC 			= 14;
	static const uint16_t  SFBL 		= 13;
	static const uint16_t  AFCRL 		= 12;
	static const uint16_t  RDSS 		= 11;
	static const uint16_t  STEREO 		= 8;


};

#endif