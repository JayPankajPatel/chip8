#include "chip8.h"
#include "aux.h"
#include <SDL_log.h>
#include <asm-generic/errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

bool init_chip8(emulator_state *state, const char *rom_name, chip8_t *chip8) {
  const uint16_t ENTRY_POINT = 0x200;
  const uint8_t FONT[] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80, // F
  };

  // clear ram
  memset(chip8, 0, sizeof(chip8_t));
  memcpy(chip8->ram, FONT, sizeof(FONT));
  // set the program counter to the start of the program
  chip8->PC = ENTRY_POINT;
  // set the stack pointer to the start of the stack
  chip8->stack_pointer = chip8->stack;
  // Load rom
  FILE *rom = fopen(rom_name, "rb");
  if (!rom) {
    SDL_Log("%s was invalid, corrupted or does not exist!\n", rom_name);
    return false;
  }

  fseek(rom, 0L, SEEK_END);

  const size_t rom_size = ftell(rom);
  rewind(rom);

  // copy the rom and start storing at 0x200 as per chip-8 specifications
  if (!fread(chip8->ram + ENTRY_POINT, rom_size, 1, rom)) {
    SDL_Log("Could not read %s ROM into memory", rom_name);
    fclose(rom);
    return false;
  }

  fclose(rom);
  // set the chip8 into run state after init
  *state = RUNNING;
  return true;
}
void emulate_instructions(chip8_t *chip8) {
  // All opcodes are two bytes long and stored in big-endian
  // however the ram is 8bit wide so we have to combine two consecutive
  // locations in memory
  chip8->draw_flag = 0;
  chip8->sound_flag = 0;

  uint16_t opcode = chip8->ram[chip8->PC] << 8 | chip8->ram[chip8->PC + 1];
  chip8->PC += 2;
  // The opcodes have the following symbols NNN: address
  // NN: 8-bit constant
  // N: 4-bit constant
  // X and Y: 4-bit register identifier
  // I: 12 bit register For memory address (similar to void pointer)
  // VN: one of the 16 available variables. N may be 0 to F in hex
  uint16_t NNN = opcode & 0x0FFF;
  uint8_t NN = opcode & 0x0FF;
  uint8_t N = opcode & 0x0F;
  // 0x0F is technically unnecessary but just a precaution
  uint8_t X = (opcode >> 8) & 0x0F;
  uint8_t Y = (opcode >> 4) & 0x0F;
  // Will be set in an instruction
  uint16_t I = 0;

  switch ((opcode >> 12) & 0x0F) {
  case 0x00:
    if (NN == 0xE0) {

      // 00E0 Clear the screen
      memset(chip8->display, false, sizeof(chip8->display));
    } else if (NN == 0xEE) {
      // 00EE Return from subroutine

      chip8->PC = *--chip8->stack_pointer;
    } else {
      SDL_Log("Invalid opcode, not to be used in programs targetting this "
              "system.\n");
    }
    break;

  case 0x01:
    // 1NNN: Jump to Address NNN
    chip8->PC = NNN;
    break;

  case 0x02:
    // 2NNN: Calls subroutine at address NNN
    *chip8->stack_pointer++ = chip8->PC;
    chip8->PC = NNN;
    break;
  case 0x03:
    // 3XNN: Skips the next instruction if VX equals NN
    if (chip8->V[X] == NN)
      chip8->PC += 2;
    break;
  case 0x04:
    // 4XNN: Skips the next instruction if VX does not equal NN
    if (chip8->V[X] != NN)
      chip8->PC += 2;
    break;

  case 0x05:
    // 5XY0: Skips the next instruction if VX equals VY
    if (chip8->V[X] == chip8->V[Y])
      chip8->PC += 2;
    break;
  case 0x09:
    // 9XY0: Skips the next instruction if VX  does not equal VY
    if (chip8->V[X] != chip8->V[Y])
      chip8->PC += 2;
    break;
  case 0x06:
    // 6XNN: Sets Vx to NN
    chip8->V[X] = NN;
    break;
  case 0x07:
    // 7XNN: Add the value of NN to VX
    chip8->V[X] += NN;
    break;
  case 0x0A:
    // ANNN: Sets I to the address NNN
    I = NNN;
    break;
  case 0x0D: {
    // DXYN Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels
    // and a height of N pixels. Each row of 8 pixels is read as bit-coded
    // starting from memory location I; I value does not change after the
    // execution of this instruction. As described above, VF is set to 1 if any
    // screen pixels are flipped from set to unset when the sprite is drawn, and
    // to 0 if that does not happen
    //
    chip8->draw_flag = 1;
    chip8->V[0xF] = 0;
    for (uint16_t yline = 0; yline < N; yline++) {
      uint16_t pixel = chip8->ram[I + yline];

      for (uint16_t xline = 0; xline <= 7; xline++) {
        if ((pixel & (0x80 >> xline))) {
          if (chip8->display[(chip8->V[X] + xline +
                              ((chip8->V[Y] + yline) * RESOLUTION_WIDTH))]) {
            chip8->V[0xF] = 1;
          }

          chip8->display[chip8->V[X] + xline +
                         ((chip8->V[Y] + yline) * RESOLUTION_WIDTH)] ^= true;
        }
      }
    }
    break;
  }

  default:
    SDL_Log("Unimplemented opcode");
    break;
  }
}