#ifndef _USB_H_
#define _USB_H_
#include <cstdint>

#define CDC_TUSK_INTERVAL 1000
#define HID_NOT_READY_MAX_INTERVAL 1000
#define MAX_PASS_LEN 64

class UsbDevice
{
private:
	static uint8_t char_to_hid_keycode( char c, uint8_t* modifier );
	static bool _start_pass;
	static uint8_t _current_pos;
	static char password[MAX_PASS_LEN];
	static bool is_hid_ready();
public:
	static bool init();
	static void pool();
	static bool send_password();
	static bool send_empty_report();
	static int read_line( char *buffer, uint32_t max_len );
	static void write_line( const char *buffer );
};



#endif 