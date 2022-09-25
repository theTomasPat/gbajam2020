#include "gba.h"

void Vsync() {
	while(*VCOUNT_MEM >= 160);  // wait until VDraw
	while(*VCOUNT_MEM <  160);  // wait until VBlank
}

// The buttons are 0 if pressed, 1 if not pressed
u16 ButtonPressed(InputState *inputs, u16 button) {
	// mask out the button from the prev and curr button states
	u16 prevPressed = (inputs->prev & button);
	u16 currPressed = (inputs->curr & button);

	// since the logic for button presses is inverted, we need to check the
	// "falling edge" to signal a button press. That means the masked input
	// needs to be a lower value than the masked previous input in order to
	// be considered "just pressed"
	if( currPressed < prevPressed ) return 1;
	return 0;
}

// Check if a certain button is not being pressed down at the moment
u16 ButtonUp(InputState *inputs, u16 button) {
	return (inputs->curr & button) > 0 ? 1 : 0;
}

// Check if a certain button is being held down
u16 ButtonDown(InputState *inputs, u16 button) {
	return (inputs->curr & button) < 1 ? 1 : 0;
}

// Update the buffered input states
void UpdateButtonStates(InputState *inputs) {
	inputs->prev = inputs->curr;
	inputs->curr = *KEYINPUT;
}
