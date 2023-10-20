#include <stdlib.h>
#include "pico/stdlib.h"
#include "bsp/board.h"
#include <ctype.h>
#include "input.h"

void Input::init()
{
    gpio_init(16);// Action button
    gpio_set_dir(16, GPIO_IN);
    gpio_pull_up(16);
}



Input::ClickType Input::get_click()
{
    ClickType click = ClickType::none;
    if(!gpio_get(16)){
        uint32_t ct = board_millis();
        while(!gpio_get(16) && board_millis() - ct < 1500){
            sleep_ms(10);
        }
        if(!gpio_get(16))
        {
            click = ClickType::longClick;
        } else 
        {
            click = ClickType::click;
        }
    }
    return click;
}
