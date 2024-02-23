#ifndef __CHIP_8_H__
#define __CHIP_8_H__
#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum { RUNNING, PAUSED, QUIT } emulator_state;
typedef struct {
  // the chip8 has 4096 of ram
  uint8_t ram[0x1000];
  // 16 8bit registers
  uint8_t V[16];
  // The original interpreters had 16 two byte entries
  uint16_t stack[16];
  // used to point to the top of the stack
  uint16_t *stack_pointer;
  // Program counter, points to current instruction in memory
  uint16_t PC;
  // original resolution of the chip-8
  bool display[64 * 32];
  // input keypad supports 16 keys in the original chip-8
  uint8_t keypad[16];

  uint8_t delay_timer;
  uint8_t sound_timer;

  uint8_t draw_flag;
  uint8_t sound_flag;
} chip8_t;

bool init_chip8(emulator_state *state, const char *, chip8_t *chip8);
// bool load_rom(const char *filename, chip8_t *chip8);

/**
 * @brief Handles Events that are classified by SDL, specifically keyboard
 * events
 * @return void
 */
#endif // __CHIP_8_H__