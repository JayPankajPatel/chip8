#include "SDL2/SDL.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ORIGINAL_X_RESOLUTION 64
#define ORIGINAL_Y_RESOLUTION 32
typedef enum { PAUSED, RUNNING, QUIT } emu_state;
typedef struct {
  uint16_t opcode;
  uint16_t NNN; // 16 bit address
  uint8_t NN;   // 8 bit constant
  uint8_t N;    // 4 bit constant
  uint8_t X;    // 4 bit register id
  uint8_t Y;    // 4 bit register id

  // seperate vars for opcodes so more straight forward code rather than doing
  // unnecessary bitwise operations
} instruc_t;
typedef struct {
  emu_state state;
  uint8_t ram[0x1000];
  bool display[64 * 32]; // Emulate original CHIP-8 resolution
  uint16_t stack[12];    // call stack
  uint16_t *stack_ptr;   // ptr for call stack
  uint8_t V[0x10];       // variable for opcodes
  uint16_t I;            // Index Register
  uint16_t PC;           // Program Counter
  uint8_t delay_timer;   // game delay counts down from 60hz to zero
  uint8_t sound_timer;   // sound delay counts down from 60hz to zero
  bool keypad[0xF];   // keypad with 15 keys char *rom_name;        // Current
                      // running rom
  instruc_t instruct; // Currently running instructions
  char *rom_name;
} chip8_t;
typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
} sdl_t;

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

bool init_chip8(chip8_t *chip8, char rom_name[]) {
  const uint32_t ENTRY_POINT = 0x200;
  chip8->state = RUNNING;
  chip8->PC = ENTRY_POINT;
  chip8->rom_name = rom_name;
  chip8->stack_ptr = chip8->stack;
  const int8_t FONT[] = {0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
                         0x20, 0x60, 0x20, 0x20, 0x70,  // 1
                         0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
                         0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
                         0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
                         0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
                         0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
                         0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
                         0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
                         0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
                         0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
                         0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
                         0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
                         0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
                         0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
                         0xF0, 0x80, 0xF0, 0x80, 0x80}; // F
                                                        // Load Font
                                                        // Load rom

  memcpy(chip8->ram, &FONT, sizeof(FONT));
  FILE *rom = fopen(rom_name, "rb");
  if (!rom) {
    SDL_Log("Rom file %s loading error \n.", rom_name);
    return false;
  }

  // get rom size
  fseek(rom, 0, SEEK_END);
  const size_t rom_size = ftell(rom);
  rewind(rom);

  // read rom into memory
  if (!fread(&chip8->ram[ENTRY_POINT], rom_size, 1, rom)) {
    SDL_Log("Could not read rom %s into memory\n", rom_name);
    return false;
  }

  fclose(rom);

  return true;
}

void clear_screen(sdl_t sdl_objs) {
  // Use normal values and allow the function to convert to correct endianess
  const uint8_t RED = 0;
  const uint8_t GREEN = 0;
  const uint8_t BLUE = 0;
  const uint8_t ALPHA = 0;
  SDL_SetRenderDrawColor(sdl_objs.renderer, RED, GREEN, BLUE, ALPHA);
  SDL_RenderClear(sdl_objs.renderer);
}

void handle_input(chip8_t *chip8) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      chip8->state = QUIT;
      break;
    case SDL_KEYDOWN:
      // Quit using the Escape key
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        chip8->state = QUIT;
        break;
      case SDLK_p:
        if (chip8->state == RUNNING) {
          chip8->state = PAUSED;
          puts("PAUSED PAUSED PAUSED");
        } else {
          chip8->state = RUNNING;
          puts("RESUMED RESUMED RESUMED");
        }
        break;
      default:
        break;
      }
    }
  }
}
void update_screen(sdl_t sdl_objs) { SDL_RenderPresent(sdl_objs.renderer); }

void deinit_sdl(sdl_t *sdl_objs) {
  SDL_DestroyRenderer(sdl_objs->renderer);
  SDL_DestroyWindow(sdl_objs->window);
  SDL_Quit();
}

void emulate_instruction(chip8_t *chip8) {

  // Get next opcode from ram
  chip8->instruct.opcode =
      (chip8->ram[chip8->PC] << 8) | chip8->ram[chip8->PC + 1];
  chip8->PC += 2; // Pre-increment program counter for next opcode

  // Fill out current instruction format
  chip8->instruct.NNN = chip8->instruct.opcode & 0x0FFF;
  chip8->instruct.NN = chip8->instruct.opcode & 0x0FF;
  chip8->instruct.N = chip8->instruct.opcode & 0x0F;
  chip8->instruct.X = (chip8->instruct.opcode >> 8) & 0x0F;
  chip8->instruct.Y = (chip8->instruct.opcode >> 4) & 0x0F;

#ifdef DEBUG
  print_debug_info(chip8);
#endif
  printf("Address: 0x%04X, Opcode: 0x%04X Desc: \n", chip8->PC - 2,
         chip8->instruct.opcode);
  // Emulate opcode
  switch ((chip8->instruct.opcode >> 12) & 0x0F) {
  case 0x00:
    if (chip8->instruct.NN == 0xE0) {
      // 0x00E0: Clear the screen
      memset(&chip8->display[0], false, sizeof chip8->display);
      printf("Clear Screen \n");
    } else if (chip8->instruct.NN == 0xEE) {
      // 0x00EE: Return from subroutine
      // Set program counter to last address on subroutine stack ("pop" it off
      // the stack)
      //   so that next opcode will be gotten from that address.
      chip8->PC = *--chip8->stack_ptr;
      printf("Return from subroutine to address 0x%04X\n",
             *(chip8->stack_ptr - 1));

    } else {
      // Unimplemented/invalid opcode, may be 0xNNN for calling
      // machine code routine for RCA1802
      printf("Unimplemented opcode\n");
    }

    break;

  case 0x01:
    // 0x1NNN: Jump to address NNN
    chip8->PC = chip8->instruct.NNN; // Set program counter so that next opcode
                                     // is from NNN
    printf("Jump to address NNN (0x%04X)\n", chip8->instruct.NNN);
    break;

  case 0x02:
    // 0x2NNN: Call subroutine at NNN
    // Store current address to return to on subroutine stack
    // ("push" it on the stack)
    //   and set program counter to subroutine address so that
    //   the next opcode is gotten from there.
    *chip8->stack_ptr++ = chip8->PC;
    chip8->PC = chip8->instruct.NNN;
    printf("Call subroutine at NNN (0x%04X)\n", chip8->instruct.NNN);
    break;
  case 0x0A:
    // 0xANNN: Sets I to the address NNN
    chip8->I = chip8->instruct.NNN;
    printf("Sets I to the address NNN (0x%04X)\n", chip8->I);
    break;
  case 0x06:
    // 0x6XNN: Sets VX to NN
    *(chip8->V + chip8->instruct.X) = chip8->instruct.NN;
    printf("Sets VX to NN (0x%04X)\n", *(chip8->V + chip8->instruct.X));
    break;
  case 0x0D: {
    // Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and
    // a height of N pixels. Each row of 8 pixels is read as bit-coded starting
    // from memory location I; I value does not change after the execution of
    // this instruction. As described above, VF is set to 1 if any screen pixels
    // are flipped from set to unset when the sprite is drawn, and to 0 if that
    // does not happen

    uint8_t X_coor = *(chip8->V + chip8->instruct.X) % ORIGINAL_X_RESOLUTION;
    uint8_t Y_coor = *(chip8->V + chip8->instruct.Y) % ORIGINAL_Y_RESOLUTION;

    // Set carry flag to 0
    chip8->V[0xF] = 0;

    for (uint8_t i = 0; i < chip8->instruct.N; i++) {
      uint8_t sprite_data = chip8->ram[chip8->I + i];
      for (uint8_t j = 7; i <= 0; j--) {
        // check every bit, if spirte data's pixel is on and so is the pixel on
        // screen turn on the carry flag
        bool screen_pixel =
            chip8->display[Y_coor * ORIGINAL_X_RESOLUTION + X_coor];
        bool sprite_bit = sprite_data & (1 << j);
        if (sprite_bit && screen_pixel) {
          chip8->V[0xF] = 1;
        }
        screen_pixel ^= sprite_bit;
        if (++X_coor >= ORIGINAL_X_RESOLUTION)
          break;
        if (++Y_coor >= ORIGINAL_Y_RESOLUTION)
          break;
      }
    }
  }
  }
}
int main(int argc, char **argv) {

  // Setup SDL
  sdl_t sdl = {0};
  if (!init_sdl(&sdl)) {
    exit(EXIT_FAILURE);
  }

  // Setup CHIP-8
  chip8_t chip8 = {0};
  char *rom_name = argv[1];
  if (!init_chip8(&chip8, rom_name)) {
    exit(EXIT_FAILURE);
  }

  clear_screen(sdl);
  update_screen(sdl);

  // Emulator Start
  while (chip8.state != QUIT) {
    handle_input(&chip8);
    if (chip8.state == PAUSED)
      continue;
    emulate_instruction(&chip8);
    SDL_Delay(16);
    update_screen(sdl);
  }

  // Clean up
  deinit_sdl(&sdl);
  exit(EXIT_SUCCESS);
}
