#pragma once

#include <memory>

enum LsControllerButton {
    DIAMOND_TOP,
    DIAMOND_BOTTOM,
    DIAMOND_LEFT,
    DIAMOND_RIGHT,
    BACK,
    GUIDE,
    START,
    LEFT_STICK,
    RIGHT_STICK,
    LEFT_SHOULDER,
    RIGHT_SHOULDER,
    DPAD_UP,
    DPAD_DOWN,
    DPAD_LEFT,
    DPAD_RIGHT,
    MISC1,
    LEFT_PADDLE1,
    RIGHT_PADDLE1,
    LEFT_PADDLE2,
    RIGHT_PADDLE2,
    TOUCHPAD,
    MISC2,
    MISC3,
    MISC4,
    MISC5,
    MISC6,
    LS_CONTROLLER_BUTTON_COUNT
};

enum LsControllerAxis {
    LEFT_X,
    LEFT_Y,
    RIGHT_X,
    RIGHT_Y,
    LEFT_PEDAL,
    RIGHT_PEDAL,
    LS_CONTROLLER_AXIS_COUNT
};

class LsControllerManager {
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

public:
    void updateInputs();
    bool isButtonPressed(unsigned controllerID, LsControllerButton button);
    float getAxisValue(unsigned controllerID, LsControllerAxis axis);
    bool controllerIsActive(unsigned controllerID);
    unsigned getControllerCount();
    unsigned getButtonCount(unsigned controllerID);

    explicit LsControllerManager(bool plugAndPlay = true);
    ~LsControllerManager();
    LsControllerManager(const LsControllerManager&) = delete;
    LsControllerManager& operator=(const LsControllerManager&) = delete;

};
