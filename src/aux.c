
#include "aux.h"
#include "chip8.h"
#include <SDL_rect.h>
#include <SDL_render.h>
#include <stdint.h>

SDL_Renderer *renderer;
SDL_Window *window;
SDL_Event event;

// Increase the window size from the original CHIP-8 resolution so the window
// is usable
const uint8_t SCALE_FACTOR = 8;

bool init_SDL() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS |
               SDL_INIT_TIMER) != 0) {
    SDL_Log("Unable to initialize SDL: %s \n", SDL_GetError());
    return false;
  }

  window = SDL_CreateWindow(
      "CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      RESOLUTION_WIDTH * SCALE_FACTOR, RESOLUTION_HEIGHT * SCALE_FACTOR, 0);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  return true;
}

void deinit_SDL() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void handle_keyboard_event(emulator_state *state) {
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      *state = QUIT;
      break;
    case SDL_KEYDOWN:
      // switch based on key symbols
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        // Quit program when ESC is pressed down
        *state = QUIT;
        break;
      case SDLK_p:
        // P toggles emulator from paused
        if (*state == RUNNING) {
          puts("PAUSED PAUSED PAUSED");
          *state = PAUSED;
        } else {
          *state = RUNNING;
          puts("RESUMED RESUMED RESUMED");
        }

        break;
      }
    default:
      break;
    }
  }
}
void draw_display(chip8_t chip8) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  // Clears current display with the drawing color that is set
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  SDL_Rect rect;
  // read display data and draw pixels
  for (uint8_t y = 0; y < RESOLUTION_HEIGHT; y++) {
    for (uint8_t x = 0; x < RESOLUTION_WIDTH; x++) {
      if (chip8.display[x + (y * RESOLUTION_WIDTH)]) {
        rect.x = x * SCALE_FACTOR;
        rect.y = y * SCALE_FACTOR;
        rect.w = SCALE_FACTOR;
        rect.h = SCALE_FACTOR;

        SDL_RenderFillRect(renderer, &rect);
      }
    }
  }

  // Update Screen
  SDL_RenderPresent(renderer);
}
