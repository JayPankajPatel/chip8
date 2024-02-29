#include "aux.h"
#include "chip8.h"
#include <SDL_timer.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
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

  // srand setup for the 0xCXNN opcode
  srand(time(NULL));

  // In practice, a standard speed of around 700 CHIP-8 instructions per second
  static const uint16_t INSTRUCTIONS_PER_SECOND = 600;
  while (state != QUIT) {
    handle_keyboard_event(&state, &chip8);
    if (state == PAUSED)
      continue;
    const uint32_t time_start = SDL_GetPerformanceCounter();

    // We divide by 60 to get the value of the amount of instructions that will
    // run at 60 Hz

    for (uint32_t i = 0; i < INSTRUCTIONS_PER_SECOND / 60; i++)
      emulate_instructions(&chip8);
    const uint32_t time_end = SDL_GetPerformanceCounter();
    // returns counts per second, to get ms, which is what SDL_Delay accepts, we
    // must multiply by 1000
    const double time_elasped = (double)((time_end - time_start) * 1000) /
                                SDL_GetPerformanceFrequency();
    // We want to execute instructions at 60 Hz so we will conditionally run
    // delays if the instruction took more than 16.67 ms, we will run it
    // immediately
    SDL_Delay(16.67f > time_elasped ? 16.67f - time_elasped : 0);

    if (chip8.draw_flag) {
      draw_display(chip8);
    }
    update_timers(&chip8);
  }

  deinit_SDL();

  exit(EXIT_SUCCESS);
}
