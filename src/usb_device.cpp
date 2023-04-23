#include "usb_device.h"
#include <stdlib.h>
#include <ctype.h>
#include "tusb.h"
#include "bsp/board.h"
#include "log.h"



bool UsbDevice::_start_pass = false;
uint8_t UsbDevice::_current_pos = 0;
char UsbDevice::password[MAX_PASS_LEN] = "MyT4st_pAs7";

bool UsbDevice::init()
{
	board_init();
	return tusb_init();
}

void UsbDevice::pool()
{
	tud_task();
}

int UsbDevice::read_line(char *buffer, uint32_t max_len)
{
	if ( !tud_cdc_n_connected(0) ) return 0;
	uint32_t count = 0;
	uint32_t timeout = board_millis() + HID_NOT_READY_MAX_INTERVAL;
	bool is_cr = false;
	while ( count < max_len - 1 )
	{
		if (tud_cdc_n_available(0))
		{
			uint8_t ch;
			uint32_t read_len = tud_cdc_n_read(0, &ch, 1);
			LOGS_INFO( "CDC read ch: %c\n\r", ch );
			if (read_len)
			{
				// Stop reading when a newline character is encountered
				if (ch == '\r' || ch == '\n')
				{
					buffer[count++] = '\n';
					buffer[count++] = '\r';
					break;
				} else
					buffer[count++] = ch;
			}
		}
		else
			tud_task();

		if( !count && ( board_millis() < timeout ) ) break;
	}
	buffer[count] = '\0';
	if( count ) LOGS_INFO( "CDC read line: %s\n\r", buffer );
	return (int)count;
}

void UsbDevice::write_line( const char *buffer )
{
	if ( !tud_cdc_n_connected( 0 ) ) return;
	uint32_t timeout = board_millis() + HID_NOT_READY_MAX_INTERVAL;
	while ( board_millis() < timeout )
	{
		if (tud_cdc_n_write_available(0))
		{
			for(uint32_t i=0; i<strlen(buffer); i++)
			{
				tud_cdc_n_write_char( 0, buffer[i] );
			}
			tud_cdc_n_write_flush( 0 );
			tud_task();
			LOGS_INFO( "CDC writed line: %s\n\r", buffer );
			break;
		}
		else 
			tud_task();
	}

}
void UsbDevice::cdc_task()
{
	uint8_t itf = 0;
	static uint32_t start_ms = 0;
	static uint8_t first_time = true;
	if ( board_millis() - start_ms < CDC_TUSK_INTERVAL) return; // not enough time
	start_ms += CDC_TUSK_INTERVAL;

	if ( !tud_cdc_n_connected(itf) )
	{ 
		LOGS_ERROR( "CDC %d not connected\n\r", itf );
		first_time = true;
		return;
	}
	if ( tud_cdc_n_write_available(itf) && first_time )
	{
		const char *buf = "Please enter new password: ";
		for(uint32_t i=0; i<strlen(buf); i++)
		{
			tud_cdc_n_write_char(itf, buf[i]);
		}
		tud_cdc_n_write_flush(itf);
		first_time = false;
	}
	if( tud_cdc_n_available(itf) )
	{
		uint8_t buf[64];
		uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));
		strncpy(password, (const char*)buf, sizeof(password) - 1);
		password[sizeof(password) - 1] = '\0';
	}
	else
	{
		LOGS_ERROR( "CDC %d not available\n\r", itf );
	}

}


bool UsbDevice::send_password()
{
	uint32_t timeout = board_millis() + HID_NOT_READY_MAX_INTERVAL;
	do{
		tud_task();
		sleep_ms(10);
	} while( !tud_hid_ready() && (board_millis() < timeout) );
	// skip if hid is not ready yet
	if( !tud_hid_ready ){
		LOGS_ERROR( "Abort sending pass, HID not ready\n" );
		return false;
	} 

	if ( tud_suspended() )
	{
		if( tud_remote_wakeup() ){
			LOGS_INFO( "Remote Wakeup\n\r" );
		} else {
			LOGS_INFO( "Remote Wakeup failde\n\r" );
		}
	}

	uint8_t keycode[6] = { 0 };
	uint8_t modifier = 0;
	for( int i = 0; password[i]; i++ )
	{
		keycode[0] = char_to_hid_keycode( password[i], &modifier );
		while (!tud_hid_ready()) {
			tud_task();
		}
		tud_hid_keyboard_report( REPORT_ID_KEYBOARD, modifier, keycode) ;
		sleep_ms(50);
		while (!tud_hid_ready()) {
			tud_task();
		}
		tud_hid_keyboard_report( REPORT_ID_KEYBOARD, 0, NULL );
		sleep_ms(50);
		LOGS_DEBUG( "Letter:(%c) sent\n\r", (char)password[i] );
	}
	LOGS_INFO( "Password sent\n\r", (char)keycode[0] );
	return true;
}

uint8_t UsbDevice::char_to_hid_keycode( char c, uint8_t* modifier )
{
	if( !c ) return 0;
	*modifier = 0;

	// Handle letters (A-Z, a-z)
	if ( isalpha( c ) )
	{
		if ( isupper( c ) ) *modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
		return HID_KEY_A + ( toupper( c ) - 'A' );
	}

	// Handle numbers (0-9)
	if ( isdigit( c ) )
	{
		return HID_KEY_0 + ( c - '0' );
	}

	// Handle some punctuation
	switch ( c )
	{
		case '!': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_1;
		case '@': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_2;
		case '#': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_3;
		case '$': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_4;
		case '%': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_5;
		case '^': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_6;
		case '&': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_7;
		case '*': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_8;
		case '(': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_9;
		case ')': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_0;
		case '-': return HID_KEY_MINUS;
		case '_': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_MINUS;
		case '=': return HID_KEY_EQUAL;
		case '+': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_EQUAL;
		case '[': return HID_KEY_BRACKET_LEFT;
		case '{': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_BRACKET_LEFT;
		case ']': return HID_KEY_BRACKET_RIGHT;
		case '}': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_BRACKET_RIGHT;
		case '\\': return HID_KEY_BACKSLASH;
		case '|': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_BACKSLASH;
		case ';': return HID_KEY_SEMICOLON;
		case ':': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_SEMICOLON;
		case '\'': return HID_KEY_APOSTROPHE;
		case '\"': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_APOSTROPHE;
		case ',': return HID_KEY_COMMA;
		case '<': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_COMMA;
		case '.': return HID_KEY_PERIOD;
		case '>': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_PERIOD;
		case '/': return HID_KEY_SLASH;
		case '?': *modifier = KEYBOARD_MODIFIER_LEFTSHIFT; return HID_KEY_SLASH;
		default: return 0;
	}
}