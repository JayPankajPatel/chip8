
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

void handle_keyboard_event(emulator_state *state, chip8_t *chip8) {
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
        } // Map qwerty keys to CHIP8 keypad
        break;
      case SDLK_1:
        chip8->keypad[0x1] = true;
        break;
      case SDLK_2:
        chip8->keypad[0x2] = true;
        break;
      case SDLK_3:
        chip8->keypad[0x3] = true;
        break;
      case SDLK_4:
        chip8->keypad[0xC] = true;
        break;

      case SDLK_q:
        chip8->keypad[0x4] = true;
        break;
      case SDLK_w:
        chip8->keypad[0x5] = true;
        break;
      case SDLK_e:
        chip8->keypad[0x6] = true;
        break;
      case SDLK_r:
        chip8->keypad[0xD] = true;
        break;

      case SDLK_a:
        chip8->keypad[0x7] = true;
        break;
      case SDLK_s:
        chip8->keypad[0x8] = true;
        break;
      case SDLK_d:
        chip8->keypad[0x9] = true;
        break;
      case SDLK_f:
        chip8->keypad[0xE] = true;
        break;

      case SDLK_z:
        chip8->keypad[0xA] = true;
        break;
      case SDLK_x:
        chip8->keypad[0x0] = true;
        break;
      case SDLK_c:
        chip8->keypad[0xB] = true;
        break;
      case SDLK_v:
        chip8->keypad[0xF] = true;
        break;

      default:
        break;
      }
      break;
    case SDL_KEYUP:
      switch (event.key.keysym.sym) {
      // Map qwerty keys to chip-8 keypad
      case SDLK_1:
        chip8->keypad[0x1] = false;
        break;
      case SDLK_2:
        chip8->keypad[0x2] = false;
        break;
      case SDLK_3:
        chip8->keypad[0x3] = false;
        break;
      case SDLK_4:
        chip8->keypad[0xC] = false;
        break;

      case SDLK_q:
        chip8->keypad[0x4] = false;
        break;
      case SDLK_w:
        chip8->keypad[0x5] = false;
        break;
      case SDLK_e:
        chip8->keypad[0x6] = false;
        break;
      case SDLK_r:
        chip8->keypad[0xD] = false;
        break;

      case SDLK_a:
        chip8->keypad[0x7] = false;
        break;
      case SDLK_s:
        chip8->keypad[0x8] = false;
        break;
      case SDLK_d:
        chip8->keypad[0x9] = false;
        break;
      case SDLK_f:
        chip8->keypad[0xE] = false;
        break;

      case SDLK_z:
        chip8->keypad[0xA] = false;
        break;
      case SDLK_x:
        chip8->keypad[0x0] = false;
        break;
      case SDLK_c:
        chip8->keypad[0xB] = false;
        break;
      case SDLK_v:
        chip8->keypad[0xF] = false;
        break;

      default:
        break;
      }
      break;
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
