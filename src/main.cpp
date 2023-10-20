#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cstdint>
#include <ctype.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "hardware/i2c.h"

#include "log.h"
#include "usb_device.h"
#include "mfrc522.h"
#include "ssd1306.h"
#include "input.h"


#define CARD_READ_INTERVAL 5000

MFRC522::Uid myCard;


void setup_gpios(void) {
    i2c_init(i2c1, 400000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14);
    gpio_pull_up(15);
}

int main()
{
    stdio_init_all();
    setup_gpios();
    board_init();

    Input action_button;
    action_button.init();

    UsbDevice::init();
    MFRC522 mfrc;
    ssd1306_t disp;
    disp.external_vcc=false;
    ssd1306_init(&disp, 128, 32, 0x3C, i2c1);
    ssd1306_clear(&disp);
    char buf[5][15] = { "Sending", "Waiting", "Service mode", "password 1", "password 2" };
    
    myCard.size = 7;
    myCard.uidByte[0] = 0x53;
    myCard.uidByte[1] = 0x03;
    myCard.uidByte[2] = 0xAB;
    myCard.uidByte[3] = 0xB2;
    myCard.uidByte[4] = 0x50;
    myCard.uidByte[5] = 0x00;
    myCard.uidByte[6] = 0x01;
    uint32_t last_sent_time = 0;
    LOGS_INFO( "Initialization done" );
    for (;;)
    {
        UsbDevice::pool();
        auto action = action_button.get_click();
        switch (action)
        {
        case Input::ClickType::click:
        {
            if( UsbDevice::get_selected_password_id() ){
                UsbDevice::select_password(0);
                ssd1306_clear(&disp);
                ssd1306_draw_string(&disp, 8, 12, 2, buf[3]);
                ssd1306_show(&disp);
                sleep_ms(1000);
            } else {
                UsbDevice::select_password(1);
                ssd1306_clear(&disp);
                ssd1306_draw_string(&disp, 8, 12, 2, buf[4]);
                ssd1306_show(&disp);
                sleep_ms(1000);
            }
            break;
        }
        case Input::ClickType::longClick:
            ssd1306_clear(&disp);
            ssd1306_draw_string(&disp, 8, 12, 2, buf[2]);
            ssd1306_show(&disp);
            sleep_ms(1000);
            break;
        default:
            break;
        }
        if( board_millis() - last_sent_time > CARD_READ_INTERVAL && mfrc.isCardPresent( myCard ) )
        {
            LOGS_INFO( "Card found!" );
            UsbDevice::write_line( "Card found!\n\r");
            ssd1306_clear(&disp);
            ssd1306_draw_string(&disp, 8, 12, 2, buf[0]);
            ssd1306_show(&disp);
            sleep_ms(100);
            bool r = UsbDevice::send_password();
            sleep_ms(100);
            last_sent_time = board_millis();
        }
        ssd1306_clear(&disp);
        ssd1306_draw_string(&disp, 8, 12, 2, buf[1]);
        ssd1306_show(&disp);
        UsbDevice::send_empty_report();
    }

    return 0;
}
