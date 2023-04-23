#define DEBUG
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cstdint>
#include <ctype.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "log.h"
#include "usb_device.h"
#include "MFRC522.h"


MFRC522::Uid myCard;

/*------------- MAIN -------------*/
int main()
{
	stdio_init_all();
	UsbDevice::init();
	MFRC522 mfrc;
	myCard.size = 7;
	myCard.uidByte[0] = 0x53;
	myCard.uidByte[1] = 0x03;
	myCard.uidByte[2] = 0xAB;
	myCard.uidByte[3] = 0xB2;
	myCard.uidByte[4] = 0x50;
	myCard.uidByte[5] = 0x00;
	myCard.uidByte[6] = 0x01;
	LOGS_INFO( "Init!" );

	for (;;)
	{
		UsbDevice::pool();
		if( mfrc.isCardPresent( myCard ) )
		{
			LOGS_INFO( "Card found!" );
			UsbDevice::send_password();
		}
	}

	return 0;
}

