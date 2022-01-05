/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "pico/stdlib.h"
#include "gamepad.h"
#include "display.h"
#include "storage.h"
#include "display.h"
#include "OneBitDisplay.h"

void Gamepad::setup()
{
	load();

	// Configure pin mapping
	f2Mask = (GAMEPAD_MASK_A1 | GAMEPAD_MASK_S2);
	BoardOptions boardOptions = getBoardOptions();
	// configure left analog stick for personal usage
	hasLeftAnalogStick = true;
	if (!boardOptions.hasBoardOptions)
	{
		boardOptions.pinDpadUp    = PIN_DPAD_UP;
		boardOptions.pinDpadDown  = PIN_DPAD_DOWN;
		boardOptions.pinDpadLeft  = PIN_DPAD_LEFT;
		boardOptions.pinDpadRight = PIN_DPAD_RIGHT;
		boardOptions.pinLeftStickUp    = PIN_LEFT_STICK_UP;
		boardOptions.pinLeftStickDown  = PIN_LEFT_STICK_DOWN;
		boardOptions.pinLeftStickLeft  = PIN_LEFT_STICK_LEFT;
		boardOptions.pinLeftStickRight = PIN_LEFT_STICK_RIGHT;
		boardOptions.pinButtonB1  = PIN_BUTTON_B1;
		boardOptions.pinButtonB2  = PIN_BUTTON_B2;
		boardOptions.pinButtonB3  = PIN_BUTTON_B3;
		boardOptions.pinButtonB4  = PIN_BUTTON_B4;
		boardOptions.pinButtonL1  = PIN_BUTTON_L1;
		boardOptions.pinButtonR1  = PIN_BUTTON_R1;
		boardOptions.pinButtonL2  = PIN_BUTTON_L2;
		boardOptions.pinButtonR2  = PIN_BUTTON_R2;
		boardOptions.pinButtonS1  = PIN_BUTTON_S1;
		boardOptions.pinButtonS2  = PIN_BUTTON_S2;
		boardOptions.pinButtonL3  = PIN_BUTTON_L3;
		boardOptions.pinButtonR3  = PIN_BUTTON_R3;
		boardOptions.pinButtonA1  = PIN_BUTTON_A1;
		boardOptions.pinButtonA2  = PIN_BUTTON_A2;

		boardOptions.i2cSDAPin = I2C_SDA_PIN;
		boardOptions.i2cSCLPin = I2C_SCL_PIN;
		boardOptions.i2cBlock  = (I2C_BLOCK == i2c0) ? 0 : 1;
		boardOptions.i2cSpeed  = I2C_SPEED;

		boardOptions.hasI2CDisplay     = HAS_I2C_DISPLAY;
		boardOptions.displayI2CAddress = DISPLAY_I2C_ADDR;
		boardOptions.displaySize       = DISPLAY_SIZE;
		boardOptions.displayFlip       = DISPLAY_FLIP;
		boardOptions.displayInvert     = DISPLAY_INVERT;

		boardOptions.hasBoardOptions = true;
		setBoardOptions(boardOptions);
		GamepadStore.save();
	}

	mapDpadUp    = new GamepadButtonMapping(boardOptions.pinDpadUp,    GAMEPAD_MASK_UP);
	mapDpadDown  = new GamepadButtonMapping(boardOptions.pinDpadDown,  GAMEPAD_MASK_DOWN);
	mapDpadLeft  = new GamepadButtonMapping(boardOptions.pinDpadLeft,  GAMEPAD_MASK_LEFT);
	mapDpadRight = new GamepadButtonMapping(boardOptions.pinDpadRight, GAMEPAD_MASK_RIGHT);
	mapLeftStickUp    = new GamepadButtonMapping(boardOptions.pinLeftStickUp,    GAMEPAD_MASK_DU);
	mapLeftStickDown  = new GamepadButtonMapping(boardOptions.pinLeftStickDown,  GAMEPAD_MASK_DD);
	mapLeftStickLeft  = new GamepadButtonMapping(boardOptions.pinLeftStickLeft,  GAMEPAD_MASK_DL);
	mapLeftStickRight = new GamepadButtonMapping(boardOptions.pinLeftStickRight, GAMEPAD_MASK_DR);
	mapButtonB1  = new GamepadButtonMapping(boardOptions.pinButtonB1,  GAMEPAD_MASK_B1);
	mapButtonB2  = new GamepadButtonMapping(boardOptions.pinButtonB2,  GAMEPAD_MASK_B2);
	mapButtonB3  = new GamepadButtonMapping(boardOptions.pinButtonB3,  GAMEPAD_MASK_B3);
	mapButtonB4  = new GamepadButtonMapping(boardOptions.pinButtonB4,  GAMEPAD_MASK_B4);
	mapButtonL1  = new GamepadButtonMapping(boardOptions.pinButtonL1,  GAMEPAD_MASK_L1);
	mapButtonR1  = new GamepadButtonMapping(boardOptions.pinButtonR1,  GAMEPAD_MASK_R1);
	mapButtonL2  = new GamepadButtonMapping(boardOptions.pinButtonL2,  GAMEPAD_MASK_L2);
	mapButtonR2  = new GamepadButtonMapping(boardOptions.pinButtonR2,  GAMEPAD_MASK_R2);
	mapButtonS1  = new GamepadButtonMapping(boardOptions.pinButtonS1,  GAMEPAD_MASK_S1);
	mapButtonS2  = new GamepadButtonMapping(boardOptions.pinButtonS2,  GAMEPAD_MASK_S2);
	mapButtonL3  = new GamepadButtonMapping(boardOptions.pinButtonL3,  GAMEPAD_MASK_L3);
	mapButtonR3  = new GamepadButtonMapping(boardOptions.pinButtonR3,  GAMEPAD_MASK_R3);
	mapButtonA1  = new GamepadButtonMapping(boardOptions.pinButtonA1,  GAMEPAD_MASK_A1);
	mapButtonA2  = new GamepadButtonMapping(boardOptions.pinButtonA2,  GAMEPAD_MASK_A2);

	gamepadMappings = new GamepadButtonMapping *[GAMEPAD_DIGITAL_INPUT_COUNT+4]
	{
		mapDpadUp,   mapDpadDown, mapDpadLeft, mapDpadRight,
		mapButtonB1, mapButtonB2, mapButtonB3, mapButtonB4,
		mapButtonL1, mapButtonR1, mapButtonL2, mapButtonR2,
		mapButtonS1, mapButtonS2, mapButtonL3, mapButtonR3,
		mapButtonA1, mapButtonA2, mapLeftStickUp, mapLeftStickDown,
		mapLeftStickLeft, mapLeftStickRight
	};

	for (int i = 0; i < GAMEPAD_DIGITAL_INPUT_COUNT; i++)
	{
		gpio_init(gamepadMappings[i]->pin);             // Initialize pin
		gpio_set_dir(gamepadMappings[i]->pin, GPIO_IN); // Set as INPUT
		gpio_pull_up(gamepadMappings[i]->pin);          // Set as PULLUP
	}

	#ifdef PIN_SETTINGS
		gpio_init(PIN_SETTINGS);             // Initialize pin
		gpio_set_dir(PIN_SETTINGS, GPIO_IN); // Set as INPUT
		gpio_pull_up(PIN_SETTINGS);          // Set as PULLUP
	#endif
}

void Gamepad::read()
{
	// Need to invert since we're using pullups
	uint32_t values = ~gpio_get_all();

	#ifdef PIN_SETTINGS
	state.aux = 0
		| ((values & (1 << PIN_SETTINGS)) ? (1 << 0) : 0)
	;
	#endif

	state.dpad = 0
		| ((values & mapDpadUp->pinMask)    ? mapDpadUp->buttonMask    : 0)
		| ((values & mapDpadDown->pinMask)  ? mapDpadDown->buttonMask  : 0)
		| ((values & mapDpadLeft->pinMask)  ? mapDpadLeft->buttonMask  : 0)
		| ((values & mapDpadRight->pinMask) ? mapDpadRight->buttonMask : 0)
	;

	state.buttons = 0
		| ((values & mapButtonB1->pinMask)  ? mapButtonB1->buttonMask  : 0)
		| ((values & mapButtonB2->pinMask)  ? mapButtonB2->buttonMask  : 0)
		| ((values & mapButtonB3->pinMask)  ? mapButtonB3->buttonMask  : 0)
		| ((values & mapButtonB4->pinMask)  ? mapButtonB4->buttonMask  : 0)
		| ((values & mapButtonL1->pinMask)  ? mapButtonL1->buttonMask  : 0)
		| ((values & mapButtonR1->pinMask)  ? mapButtonR1->buttonMask  : 0)
		| ((values & mapButtonL2->pinMask)  ? mapButtonL2->buttonMask  : 0)
		| ((values & mapButtonR2->pinMask)  ? mapButtonR2->buttonMask  : 0)
		| ((values & mapButtonS1->pinMask)  ? mapButtonS1->buttonMask  : 0)
		| ((values & mapButtonS2->pinMask)  ? mapButtonS2->buttonMask  : 0)
		| ((values & mapButtonL3->pinMask)  ? mapButtonL3->buttonMask  : 0)
		| ((values & mapButtonR3->pinMask)  ? mapButtonR3->buttonMask  : 0)
		| ((values & mapButtonA1->pinMask)  ? mapButtonA1->buttonMask  : 0)
		| ((values & mapButtonA2->pinMask)  ? mapButtonA2->buttonMask  : 0)
	;

	state.lx = (values & mapButtonB1->pinMask) ? GAMEPAD_JOYSTICK_MAX : (values & mapButtonB2->pinMask) ? GAMEPAD_JOYSTICK_MIN : GAMEPAD_JOYSTICK_MID;
	state.ly = (values & mapButtonB3->pinMask) ? GAMEPAD_JOYSTICK_MAX : (values & mapButtonB4->pinMask) ? GAMEPAD_JOYSTICK_MIN : GAMEPAD_JOYSTICK_MID;
	state.rx = GAMEPAD_JOYSTICK_MID;
	state.ry = GAMEPAD_JOYSTICK_MID;
	state.lt = GAMEPAD_JOYSTICK_MID;
	state.rt = GAMEPAD_JOYSTICK_MID;
}
