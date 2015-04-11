/**
 * \file buttons.c
 *
 * \date 9/04/2014
 * \authors Dominic Shelton
 * \authors Colleen Kerr
 */

#include "inc/lm3s1968.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#include "gpio_helper.h"
#include "buttons.h"

static Button *g_Buttons;

static uint8_t g_ucNumButtons;

static uint8_t g_ucPins;

static uint32_t g_ulDelay[2];

void buttons_init (uint32_t onDelay, uint32_t offDelay, Button b[], uint8_t n)
{
    uint8_t i = 0;
    g_Buttons = b;
    g_ucNumButtons = n;
    g_ulDelay[0] = offDelay;
    g_ulDelay[1] = onDelay;

    // Enable port G
    SysCtlPeripheralEnable (gpio_getPeriph(b[0].port));

    // Iterate through the buttons anding their pins together.
    for (i = 0; i < n; i++)
    {
        g_ucPins |= b[i].pin;
    }

    GPIOPinTypeGPIOInput (b[0].port, g_ucPins);
    GPIOPadConfigSet (b[0].port, g_ucPins, GPIO_STRENGTH_2MA,
            GPIO_PIN_TYPE_STD_WPU);
}

void buttons_poll (uint32_t timeElapsed)
{
    uint8_t i = 0;

    // Read the value all the button pins.
    uint32_t pins = GPIOPinRead (g_Buttons[0].port, g_ucPins);

    for (i = 0; i < g_ucNumButtons; i++)
    {
        g_Buttons[i].readTime += timeElapsed;

        // If enough time has elapsed, get the button state.
        if (g_Buttons[i].readTime >= g_ulDelay[g_Buttons[i].stateRaw])
        {
            // Determine the state of the pin, low is on, high is off.
            uint8_t pinState = (pins & g_Buttons[i].pin) != g_Buttons[i].pin;

            // If the value has changed, reset the time counter and change
            // the button state.
            if (pinState != g_Buttons[i].stateRaw)
            {
                if (g_Buttons[i].mode == MODE_TOGGLE)
                {
                    // If the button is a toggle, the actual state is toggled
                    // when the raw state becomes on.
                    g_Buttons[i].stateRaw = pinState;
                    if (pinState)
                    {
                        g_Buttons[i].state = !g_Buttons[i].state;
                    }
                }
                else
                {
                    g_Buttons[i].stateRaw = pinState;
                    g_Buttons[i].state = pinState;
                }
                g_Buttons[i].readTime = 0;
            }
            else
            {
                // Otherwise set the time to the delay so that the value will
                // never overflow and the state will be checked again next poll.
                g_Buttons[i].readTime = g_ulDelay[g_Buttons[i].state];
            }
        }
    }
}

tBoolean buttons_get (Button* b)
{
    if (b->mode == MODE_DISABLED)
        return 0;
    uint8_t state = b->state;
    // If the button is a one-shot, the state should only be read as on once.
    if (b->mode == MODE_ONESHOT)
        b->state = 0;
    return state;
}

void buttons_new (Button* b, uint32_t port, uint8_t pin, Mode mode)
{
    b->port = port;
    b->pin = pin;
    b->mode = mode;
    b->readTime = 0;
    b->stateRaw = 0;
    b->state = 0;
}

void buttons_setmode (Button* b, Mode m)
{
    b->mode = m;
}
