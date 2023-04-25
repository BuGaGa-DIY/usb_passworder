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
	board_init();
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
	uint32_t state = board_button_read();
	LOGS_INFO( "Initialization done" );
	for (;;)
	{
		UsbDevice::pool();
		//if( mfrc.isCardPresent( myCard ) )
		if( board_button_read() && !state )
		{
			state = 1;
			LOGS_INFO( "Card found!" );
			UsbDevice::write_line( "Card found!\n\r");
			bool r = UsbDevice::send_password();
			LOGS_DEBUG( "Posword send result: %s\n\r", r ? "true" : "false" );
		} 
		else if( !board_button_read() && state )
		{
			state = 0;
			UsbDevice::send_empty_report();
		}
	}

	return 0;
}

