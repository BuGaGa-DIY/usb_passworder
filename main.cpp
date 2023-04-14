#define DEBUG
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include <ctype.h>

#include "log.h"
#define CDC_TUSK_INTERVAL 1000
#define MAX_PASS_LEN 64
static char password[MAX_PASS_LEN] = "My!paSs\0";
static uint8_t current_pos = 0;
static bool start_pass = false;

void hid_task(void);
static void cdc_task(void);

uint8_t char_to_hid_keycode( char c, uint8_t* modifier )
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

/*------------- MAIN -------------*/
int main()
{
	board_init();
	tusb_init();

	while (1)
	{
		tud_task(); // tinyusb device task
		hid_task();
		cdc_task();
	}

	return 0;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
	LOGS_INFO("Device is mounted\n\r");
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
	LOGS_INFO("Device is unmounted\n\r");
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
	(void) remote_wakeup_en;
	LOGS_INFO("Device is suspended\n\r");

}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
	LOGS_INFO("Device is resumed\n\r");
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_password()
{
	// skip if hid is not ready yet
	if ( !tud_hid_ready() ) return;
	
	static bool toggle = false;
	uint8_t keycode[6] = { 0 };
	uint8_t modifier = 0;
	for( int i = 0; password[i]; i++ )
	{
		keycode[0] = char_to_hid_keycode( password[i], &modifier );
		while (!tud_hid_ready()) {
			tud_task();
		}
		tud_hid_keyboard_report( REPORT_ID_KEYBOARD, modifier, keycode) ;
		sleep_ms(10);
		while (!tud_hid_ready()) {
			tud_task();
		}
		tud_hid_keyboard_report( REPORT_ID_KEYBOARD, 0, NULL );
		sleep_ms(10);
		LOGS_INFO( "Sent password key %c\n\r", (char)keycode[0] );
	}

}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task()
{
	// Poll every 10ms
	const uint32_t interval_ms = 10;
	static uint32_t start_ms = 0;
	
	if ( board_millis() - start_ms < interval_ms ) return; // not enough time
	start_ms += interval_ms;
	
	const uint32_t btn = board_button_read();
	if( btn )
	{
		if ( tud_suspended() )
		{
			tud_remote_wakeup();
			LOGS_INFO( "Remote Wakeup\n\r" );
		}
		if( !start_pass )
		{
			// Start password sending siquence
			start_pass = true;
			send_password();
		} else
		{
			send_password();
		}

	}
}
static void cdc_task(void)
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
// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
	LOGS_DEBUG( "HID report complete, instance: %d; len: %d\n\r", instance, len );
	(void) instance;
	(void) len;

}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
	// TODO not Implemented
	LOGS_DEBUG( "HID report complete, instance: %d; report_id: %d\n\r", instance, report_id );
	(void) instance;
	(void) report_id;
	(void) report_type;
	(void) buffer;
	(void) reqlen;

	return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
	(void) instance;

	if (report_type == HID_REPORT_TYPE_OUTPUT)
	{
	// Set keyboard LED e.g Capslock, Numlock etc...
		if (report_id == REPORT_ID_KEYBOARD)
		{
			// bufsize should be (at least) 1
			if ( bufsize < 1 ) return;

			(void) buffer[0];
		}
		LOGS_DEBUG( "HID set report done\n\r" );
	}
}
