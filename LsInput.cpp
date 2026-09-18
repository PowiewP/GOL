#include "LsInput.h"

#include <SDL3/SDL.h>
#include <array>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// Index-aligned with LsControllerButton's declaration order.
constexpr std::array<SDL_GamepadButton, LS_CONTROLLER_BUTTON_COUNT> kButtonMap = {
    SDL_GAMEPAD_BUTTON_NORTH,          // DIAMOND_TOP
    SDL_GAMEPAD_BUTTON_SOUTH,          // DIAMOND_BOTTOM
    SDL_GAMEPAD_BUTTON_WEST,           // DIAMOND_LEFT
    SDL_GAMEPAD_BUTTON_EAST,           // DIAMOND_RIGHT
    SDL_GAMEPAD_BUTTON_BACK,           // BACK
    SDL_GAMEPAD_BUTTON_GUIDE,          // GUIDE
    SDL_GAMEPAD_BUTTON_START,          // START
    SDL_GAMEPAD_BUTTON_LEFT_STICK,     // LEFT_STICK
    SDL_GAMEPAD_BUTTON_RIGHT_STICK,    // RIGHT_STICK
    SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,  // LEFT_SHOULDER
    SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, // RIGHT_SHOULDER
    SDL_GAMEPAD_BUTTON_DPAD_UP,        // DPAD_UP
    SDL_GAMEPAD_BUTTON_DPAD_DOWN,      // DPAD_DOWN
    SDL_GAMEPAD_BUTTON_DPAD_LEFT,      // DPAD_LEFT
    SDL_GAMEPAD_BUTTON_DPAD_RIGHT,     // DPAD_RIGHT
    SDL_GAMEPAD_BUTTON_MISC1,          // MISC1
    SDL_GAMEPAD_BUTTON_LEFT_PADDLE1,   // LEFT_PADDLE1
    SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1,  // RIGHT_PADDLE1
    SDL_GAMEPAD_BUTTON_LEFT_PADDLE2,   // LEFT_PADDLE2
    SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2,  // RIGHT_PADDLE2
    SDL_GAMEPAD_BUTTON_TOUCHPAD,       // TOUCHPAD
    SDL_GAMEPAD_BUTTON_MISC2,          // MISC2
    SDL_GAMEPAD_BUTTON_MISC3,          // MISC3
    SDL_GAMEPAD_BUTTON_MISC4,          // MISC4
    SDL_GAMEPAD_BUTTON_MISC5,          // MISC5
    SDL_GAMEPAD_BUTTON_MISC6,          // MISC6
};

// Index-aligned with LsControllerAxis's declaration order.
constexpr std::array<SDL_GamepadAxis, LS_CONTROLLER_AXIS_COUNT> kAxisMap = {
    SDL_GAMEPAD_AXIS_LEFTX,        // LEFT_X
    SDL_GAMEPAD_AXIS_LEFTY,        // LEFT_Y
    SDL_GAMEPAD_AXIS_RIGHTX,       // RIGHT_X
    SDL_GAMEPAD_AXIS_RIGHTY,       // RIGHT_Y
    SDL_GAMEPAD_AXIS_LEFT_TRIGGER, // LEFT_PEDAL
    SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,// RIGHT_PEDAL
};

float clamp(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

// Sticks report -32768..32767 -> normalize to -1..1.
// Triggers report 0..32767       -> normalize to 0..1.
float normalizeAxis(LsControllerAxis axis, Sint16 raw)
{
    if (axis == LEFT_PEDAL || axis == RIGHT_PEDAL)
        return clamp(static_cast<float>(raw) / 32767.0f, 0.0f, 1.0f);
    return clamp(static_cast<float>(raw) / 32768.0f, -1.0f, 1.0f);
}

} // namespace

struct LsControllerManager::Impl {
    struct Slot {
        SDL_JoystickID id = 0;
        SDL_Gamepad* gamepad = nullptr;
        bool active = false;
    };

    std::vector<Slot> slots;
    bool plugAndPlay;

    explicit Impl(bool pnp) : plugAndPlay(pnp)
    {
        if (!SDL_Init(SDL_INIT_GAMEPAD))
            throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());

        // With plugAndPlay off, we never touch the gamepad-event path again
        // after this constructor, so hotplugging is genuinely not tracked --
        // matches libgamepad's set_plug_and_play(false) semantics.
        SDL_SetGamepadEventsEnabled(plugAndPlay);

        int count = 0;
        SDL_JoystickID* ids = SDL_GetGamepads(&count);
        if (ids) {
            for (int i = 0; i < count; ++i)
                addDevice(ids[i]);
            SDL_free(ids);
        }
    }

    ~Impl()
    {
        for (auto& slot : slots)
            if (slot.gamepad)
                SDL_CloseGamepad(slot.gamepad);
        SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
    }

    void addDevice(SDL_JoystickID id)
    {
        for (auto& slot : slots)
            if (slot.active && slot.id == id)
                return; // already tracked, e.g. duplicate ADDED event

        SDL_Gamepad* gp = SDL_OpenGamepad(id);
        if (!gp)
            return; // failed to open; silently skip rather than crash the app
                     // over a single bad device -- caller can't act on this
                     // anyway since it never gets a controllerID for it

        for (auto& slot : slots) {
            if (!slot.active) {
                slot.id = id;
                slot.gamepad = gp;
                slot.active = true;
                return;
            }
        }
        slots.push_back(Slot{ id, gp, true });
    }

    void removeDevice(SDL_JoystickID id)
    {
        for (auto& slot : slots) {
            if (slot.active && slot.id == id) {
                SDL_CloseGamepad(slot.gamepad);
                slot.gamepad = nullptr;
                slot.active = false;
                return; // slot index is intentionally left as a gap, so
                        // existing controllerIDs held by the caller for
                        // OTHER controllers never shift
            }
        }
    }

    Slot& slotFor(unsigned controllerID)
    {
        if (controllerID >= slots.size() || !slots[controllerID].active)
            throw std::out_of_range("ControllerManager: invalid or disconnected controllerID");
        return slots[controllerID];
    }
};

LsControllerManager::LsControllerManager(bool plugAndPlay)
    : m_impl(std::make_unique<Impl>(plugAndPlay))
{
}

LsControllerManager::~LsControllerManager() = default;

void LsControllerManager::updateInputs()
{
    if (m_impl->plugAndPlay) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_GAMEPAD_ADDED)
                m_impl->addDevice(event.gdevice.which);
            else if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
                m_impl->removeDevice(event.gdevice.which);
            // Any other event type is silently dropped here. If your
            // application also needs SDL events for something else, this
            // loop is currently the only place pumping SDL's queue --
            // route those events through here too rather than adding a
            // second PollEvent loop elsewhere.
        }
    } else {
        SDL_UpdateGamepads();
    }
}

bool LsControllerManager::isButtonPressed(unsigned controllerID, LsControllerButton button)
{
    auto& slot = m_impl->slotFor(controllerID);
    return SDL_GetGamepadButton(slot.gamepad, kButtonMap[button]);
}

float LsControllerManager::getAxisValue(unsigned controllerID, LsControllerAxis axis)
{
    auto& slot = m_impl->slotFor(controllerID);
    Sint16 raw = SDL_GetGamepadAxis(slot.gamepad, kAxisMap[axis]);
    return normalizeAxis(axis, raw);
}

unsigned LsControllerManager::getControllerCount()
{
    return static_cast<unsigned>(m_impl->slots.size());
}

bool LsControllerManager::controllerIsActive(unsigned controllerID)
{
    return controllerID < m_impl->slots.size() && m_impl->slots[controllerID].active;
}

unsigned LsControllerManager::getButtonCount(unsigned controllerID)
{
    auto& slot = m_impl->slotFor(controllerID);
    unsigned count = 0;
    for (const auto sdlButton : kButtonMap)
        if (SDL_GamepadHasButton(slot.gamepad, sdlButton))
            ++count;
    return count;
}