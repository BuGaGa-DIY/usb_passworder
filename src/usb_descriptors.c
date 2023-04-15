/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb.h"
#include "log.h"

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
							 _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

#define USB_VID   0xCafe
#define USB_BCD   0x0200

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
	.bLength            = sizeof(tusb_desc_device_t),
	.bDescriptorType    = TUSB_DESC_DEVICE,
	.bcdUSB             = USB_BCD,
	.bDeviceClass       = 0x00,
	.bDeviceSubClass    = 0x00,
	.bDeviceProtocol    = 0x00,
	.bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

	.idVendor           = USB_VID,
	.idProduct          = USB_PID,
	.bcdDevice          = 0x0100,

	.iManufacturer      = 0x01,
	.iProduct           = 0x02,
	.iSerialNumber      = 0x03,

	.bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
	LOGS_INFO("tud_descriptor_device_cb\n\r");
	return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

uint8_t const desc_hid_report[] =
{
	TUD_HID_REPORT_DESC_KEYBOARD( HID_REPORT_ID(REPORT_ID_KEYBOARD         ))
};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance)
{
	LOGS_INFO("tud_hid_descriptor_report_cb instance: %d\n\r", instance);
	(void) instance;
	return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum
{
	ITF_NUM_HID,
	ITF_NUM_CDC_0,
	ITF_NUM_CDC_0_DATA,
	ITF_NUM_TOTAL
};


#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_CDC_DESC_LEN)

#define EPNUM_HID   0x81

#define EPNUM_CDC_0_NOTIF   0x82
#define EPNUM_CDC_0_OUT     0x03
#define EPNUM_CDC_0_IN      0x83

uint8_t const desc_configuration[] =
{
	// Config number, interface count, string index, total length, attribute, power in mA
	TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

	// Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
	TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5),

	// 1st CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
	TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, 4, EPNUM_CDC_0_NOTIF, 8, EPNUM_CDC_0_OUT, EPNUM_CDC_0_IN, 64)
};

#if TUD_OPT_HIGH_SPEED
// Per USB specs: high speed capable device must report device_qualifier and other_speed_configuration

// other speed configuration
uint8_t desc_other_speed_config[CONFIG_TOTAL_LEN];

// device qualifier is mostly similar to device descriptor since we don't change configuration based on speed
tusb_desc_device_qualifier_t const desc_device_qualifier =
{
	.bLength            = sizeof(tusb_desc_device_qualifier_t),
	.bDescriptorType    = TUSB_DESC_DEVICE_QUALIFIER,
	.bcdUSB             = USB_BCD,

	.bDeviceClass       = 0x00,
	.bDeviceSubClass    = 0x00,
	.bDeviceProtocol    = 0x00,

	.bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
	.bNumConfigurations = 0x01,
	.bReserved          = 0x00
};

// Invoked when received GET DEVICE QUALIFIER DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete.
// device_qualifier descriptor describes information about a high-speed capable device that would
// change if the device were operating at the other speed. If not highspeed capable stall this request.
uint8_t const* tud_descriptor_device_qualifier_cb(void)
{
	return (uint8_t const*) &desc_device_qualifier;
}

// Invoked when received GET OTHER SEED CONFIGURATION DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
// Configuration descriptor in the other speed e.g if high speed then this is for full speed and vice versa
uint8_t const* tud_descriptor_other_speed_configuration_cb(uint8_t index)
{
	(void) index; // for multiple configurations

	// other speed config is basically configuration with type = OHER_SPEED_CONFIG
	memcpy(desc_other_speed_config, desc_configuration, CONFIG_TOTAL_LEN);
	desc_other_speed_config[1] = TUSB_DESC_OTHER_SPEED_CONFIG;

	// this example use the same configuration for both high and full speed mode
	return desc_other_speed_config;
}

#endif // highspeed

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
	LOGS_INFO("descriptor_configuration index: %d\n\r", index);
	(void) index; // for multiple configurations
	// This example use the same configuration for both high and full speed mode
	return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const* string_desc_arr [] =
{
	(const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
	"TinyUSB",                     // 1: Manufacturer
	"TinyUSB Device",              // 2: Product
	"123456",                      // 3: Serials, should use chip ID
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	LOGS_INFO("tud_descriptor_string_cb index: %d; langid: %d\n\r", index, langid);
	(void) langid;

	uint8_t chr_count;

	if ( index == 0)
	{
	memcpy(&_desc_str[1], string_desc_arr[0], 2);
	chr_count = 1;
	}else
	{

	if ( !(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) ) return NULL;

	const char* str = string_desc_arr[index];

	// Cap at max char
	chr_count = strlen(str);
	if ( chr_count > 31 ) chr_count = 31;

	// Convert ASCII string into UTF-16
	for(uint8_t i=0; i<chr_count; i++)
	{
		_desc_str[1+i] = str[i];
	}
	}

	// first byte is length (including header), second byte is string type
	_desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2*chr_count + 2);

	return _desc_str;
}

// Callbaks
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
