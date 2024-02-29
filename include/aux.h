#ifndef __AUX_H__
#define __AUX_H__
#include "SDL2/SDL.h"
#include "chip8.h"
#include <stdbool.h>

#define RESOLUTION_WIDTH 64
#define RESOLUTION_HEIGHT 32

extern SDL_Renderer *renderer;
extern SDL_Window *window;
extern SDL_Event event;
extern emulator_state state;
/**
 * @brief Initialize the renderer, window and input handling
 * @return True if Initialization was successful, False if not
 */
bool init_SDL();
/**
 * @brief Closes all windows and renderers and deinitializes the SDL library.
 * @return void
 */
void deinit_SDL();
/**
 * @brief Handles Events that are classified by SDL, specifically keyboard
 * events
 * @return void
 */
/**
 * @brief Read inputs from the keyboard to allow the user to interact with the
 * programs
 *
 * @param state Allow user keyboard input to modify the state of the emulator
 * @param chip8 Allow user keyboard input to modify the keypad of the chip-8
 * */
void handle_keyboard_event(emulator_state *state, chip8_t *chip8);
/**
 * @brief Draw display from chip8 struct member display
 *
 * @param chip8 Read chip8 struct display to draw on screen using SDL
 */
void draw_display(chip8_t chip8);
/**
 * @brief Emulate cycles and operation of chip-8
 *
 * @param chip8 Modify and read the chip-8 machine as per spec
 */
void emulate_instructions(chip8_t *chip8);
#endif // __AUX_H__
