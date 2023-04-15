#define DEBUG
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cstdint>
#include <ctype.h>

#include "bsp/board.h"
#include "log.h"
#include "usb_device.h"




/*------------- MAIN -------------*/
int main()
{
	UsbDevice::init();
	while (1)
	{
		//UsbDevice::hid_task();
		//UsbDevice::cdc_task();
		UsbDevice::pool();
		char buff[MAX_PASS_LEN] = {0};
		UsbDevice::write_line( "Enter your echo line\n\r" );
		uint32_t timeout = board_millis() + 10000;
		do
		{
			UsbDevice::read_line( buff, MAX_PASS_LEN );
			UsbDevice::pool();
		} while ( !buff[0] && ( board_millis() < timeout ) );
		if( buff[0] )
		{
			UsbDevice::write_line( buff );
		}
	}

	return 0;
}

