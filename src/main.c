#include "SDL2/SDL.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef enum { PAUSED, RUNNING, QUIT } emu_state;
typedef struct {
  emu_state state;
} chip8_t;
typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
} sdl_t;
uint8_t swap_8bitendianess(uint8_t number) {
  // necessary because of x86 target little endianess
  return ((number & 0x0F) << 4) | ((number & 0xF0) >> 4);
}
bool init_sdl(sdl_t *sdl_objs) {

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS |
               SDL_INIT_TIMER) != 0) {
    SDL_Log("Unable to initialize SDL: %s \n", SDL_GetError());
    return false;
  }

  // Disply Creation with original CHIP-8 X and Y resolutions
  const uint32_t WINDOW_WIDTH = 960;
  const uint32_t WINDOW_HEIGHT = 540;

  // See https://wiki.libsdl.org/SDL3/SDL_CreateWindow
  // Create a resizeable window in the middle of the screen
  sdl_objs->window = SDL_CreateWindow(
      "CHIP-8 Interperter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
  if (!sdl_objs->window) {
    SDL_Log("Unable to create a SDL Window: %s \n", SDL_GetError());
    return false;
  }

  // Creates renderer to display objects on the screen
  sdl_objs->renderer =
      SDL_CreateRenderer(sdl_objs->window, -1, SDL_RENDERER_ACCELERATED);
  if (!sdl_objs->renderer) {
    SDL_Log("Unable to create a SDL Renderer: %s \n", SDL_GetError());
    return false;
  }

  return true;
}

bool init_chip8(chip8_t *chip8) {
  chip8->state = RUNNING;
  return true;
}

void clear_screen(sdl_t sdl_objs) {
  // Use normal values and allow the function to convert to correct endianess
  const uint8_t RED = 210;
  const uint8_t GREEN = 194;
  const uint8_t BLUE = 209;
  const uint8_t ALPHA = 1;
  SDL_SetRenderDrawColor(sdl_objs.renderer, RED, GREEN, BLUE, ALPHA);
  SDL_RenderClear(sdl_objs.renderer);
}

void handle_input(chip8_t *chip8) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      chip8->state = QUIT;
      return;
    case SDL_KEYDOWN:
      // Quit using the Escape key
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        chip8->state = QUIT;
        return;
      default:
        return;
      }
      return;
    }
  }
}
void update_screen(sdl_t sdl_objs) { SDL_RenderPresent(sdl_objs.renderer); }

void deinit_sdl(sdl_t *sdl_objs) {
  SDL_DestroyRenderer(sdl_objs->renderer);
  SDL_DestroyWindow(sdl_objs->window);
  SDL_Quit();
}
int main(void) {

  // Setup
  sdl_t sdl = {0};
  if (!init_sdl(&sdl)) {
    exit(EXIT_FAILURE);
  }

  chip8_t chip8 = {.state = RUNNING};

  clear_screen(sdl);
  update_screen(sdl);

  // Emulator Start
  while (chip8.state != QUIT) {

    handle_input(&chip8);
    SDL_Delay(16);
  }

  // Clean up
  deinit_sdl(&sdl);
  return 0;
}
