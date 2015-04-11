/**
 * \file buttons.h
 *
 * \date 9/04/2014
 * \authors Dominic Shelton
 * \authors Colleen Kerr
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>

/**
 * The mode a button is set to affects how it functions and how the state read
 * relates to the physical state of the button.
 */
typedef enum
{
    MODE_ONESHOT, ///< State is reset to 0 after reading.
    MODE_MOMENTARY, ///< State is the same as the physical button state
    MODE_TOGGLE, ///< State toggles when the button goes on.
    MODE_DISABLED ///< The state is permanently 0.
} Mode;

/**
 * Stores the working state and hardware details of the button.
 */
typedef struct
{
    uint32_t readTime; ///< The time since the button was last read.
    uint32_t port; ///< The hardware port the button is on.
    uint8_t pin; ///< The hardware pin the button is on.
    uint8_t state; ///< The virtual state of the button.
    uint8_t stateRaw; ///< The state of the physical button.
    Mode mode; ///< The mode the button is in.
} Button;

/**
 * Sets the delay for debouncing for on and off, and initialises the GPIO for
 * the buttons. The units for delays are unimportant, as long as they match
 * the timeElapsed in buttons_poll. Extremely large delay values will cause
 * overflow and undefined behaviour. All buttons must be on the same GPIO port.
 * \param   onDelay     The delay after registering a change to 'on' before
 *                      checking again for changes in button state.
 * \param   offDelay    The delay after registering a change to 'off'
 *                      before checking again for changes in button state.
 * \param   b           The array of buttons to poll.
 * \param   n           The number of buttons in the array.
 */
void buttons_init (uint32_t onDelay, uint32_t offDelay, Button b[], uint8_t n);

/**
 * Polls the buttons for changes in state. Extremely large polling intervals
 * will cause overflow and undefined behaviour.
 * \param   timeElapsed The time elapsed since the last poll, units are
 *                          unimportant as long as they match the delay
 *                          values used when initialising.
 */
void buttons_poll (uint32_t timeElapsed);

/**
 * Get the current state of button specified.
 * \param   b   The button to get the state of.
 * \return      The current state of the button, i.e. 1 or 0.
 */
tBoolean buttons_get (Button* b);

/**
 * Populates the fields of the button with initial values
 * \param   b       The button to assign the values to.
 * \param   port    The port the button is on.
 * \param   pin     The pin the button is on.
 * \param   mode    What the mode the button should be.
 */
void buttons_new (Button* b, uint32_t port, uint8_t pin, Mode mode);

/**
 * Sets the mode of the specified button.
 * \param   b   The button to change the mode of.
 * \param   m   The mode to set the button to.
 */
void buttons_setmode (Button* b, Mode m);

#endif /* BUTTONS_H_ */
