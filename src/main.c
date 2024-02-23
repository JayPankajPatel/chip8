#include "aux.h"
#include "chip8.h"
#include <SDL_timer.h>
#include <stdlib.h>

emulator_state state;
chip8_t chip8;

int main(int argc, char *argv[]) {

  if (argc < 2) {
    puts("Usage: chip8 rom.ch8");
    exit(EXIT_FAILURE);
  }

  if (!init_SDL()) {
    exit(EXIT_FAILURE);
  }

  if (!init_chip8(&state, argv[1], &chip8)) {
    exit(EXIT_FAILURE);
  }

  while (state != QUIT) {
    handle_keyboard_event(&state);
    emulate_instructions(&chip8);
    if (chip8.draw_flag) {
      draw_display(chip8);
    }
    SDL_Delay(16);

    if (state == PAUSED)
      continue;
  }

  deinit_SDL();

  exit(EXIT_SUCCESS);
}
