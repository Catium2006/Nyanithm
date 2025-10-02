#include <led.h>

bool _ledState = false;

bool getOnBoardLedState(){
    return _ledState;
}

void onboardLED(bool state) {
    _ledState = state;
    gpio_put(25, state);
}

void initOnboardLED() {
    gpio_init(25);
    gpio_set_dir(25, true);
    gpio_pull_up(25);

    onboardLED(!getOnBoardLedState());
}
