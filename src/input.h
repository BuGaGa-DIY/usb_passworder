#ifndef _INPUT_H
#define _INPUT_H

class Input
{
public:
    enum class ClickType{
        none = 0,
        click,
        longClick,
    };
    Input(){};
    void init();
    ClickType get_click();
private:

};

#endif