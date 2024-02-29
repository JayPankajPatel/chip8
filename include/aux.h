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
void handle_keyboard_event(emulator_state *state, chip8_t *chip8);
void draw_display(chip8_t chip8);
void emulate_instructions(chip8_t *chip8);
#endif // __AUX_H__
