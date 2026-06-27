# Introduction to Opcodes

An **opcode** (operation code) is the portion of a machine language instruction that specifies the operation to be performed. In physical hardware, the opcode is fed directly into a control unit, which uses the binary digits to toggle microscopic logic gates, routing electrical signals through the ALU (Arithmetic Logic Unit), memory, and registers. 

When you run a program written in C, Python, or Rust, the code is eventually compiled (or interpreted) down to these raw, binary opcodes. The CPU knows absolutely nothing about strings, arrays, or objects. It only knows about binary instructions like "Add Register 1 to Register 2" or "Load Memory Address 0x200 into Index Register".

In the CHIP-8 virtual machine, there are exactly 35 distinct instructions. Your job as an emulator engineer is to build a software control unit—a massive decoder ring—that looks at these binary opcodes and executes their corresponding behavior using C code.

---

# Instruction Encoding

Every single CHIP-8 instruction is exactly **16 bits** (2 bytes) long.

Because reading 16 binary digits (e.g., `1010001011110000`) is agonizing for humans, we universally represent CHIP-8 opcodes in **hexadecimal**, where every 4 bits (a nibble) maps to a single hex digit. 

Thus, a 16-bit instruction is represented by exactly 4 hexadecimal digits.
Example: `0xA2F0`.

### The Anatomy of an Instruction
The CHIP-8 instruction set uses a specific notation to describe its opcodes. You must memorize these placeholders:

* **`N`**: A 4-bit value (one nibble), representing the lowest 4 bits of the instruction.
* **`NN`**: An 8-bit value (one byte), representing the lowest 8 bits of the instruction.
* **`NNN`**: A 12-bit value, representing the lowest 12 bits of the instruction. Used almost exclusively for memory addresses.
* **`X`**: A 4-bit value representing the lower nibble of the high byte. Always used as a register index (to look up `V[X]`).
* **`Y`**: A 4-bit value representing the upper nibble of the low byte. Always used as a register index (to look up `V[Y]`).

Let us break down the instruction `0xD456` using the template `DXYN`:
* The leading `D` tells the CPU this is a "Draw" instruction.
* `X` = `4`. The CPU will read the X-coordinate from register `V4`.
* `Y` = `5`. The CPU will read the Y-coordinate from register `V5`.
* `N` = `6`. The CPU will draw a sprite that is 6 pixels tall.

---

# Decoder Fundamentals

To extract these fields (`X`, `Y`, `N`, `NN`, `NNN`) from a raw 16-bit integer, we use Bitwise AND masking and Bitwise Right Shifting.

### Bit Masking (AND `&`)
The bitwise AND operator isolates specific bits. If you AND a value with `1`, the bit passes through. If you AND a value with `0`, the bit is erased.

To extract the lowest 12 bits (`NNN`) from the instruction `0xA2F0`:
```c
// 0xA2F0 & 0x0FFF
//   1010 0010 1111 0000 (0xA2F0)
// & 0000 1111 1111 1111 (0x0FFF)
// ---------------------
//   0000 0010 1111 0000 (0x02F0)
uint16_t nnn = opcode & 0x0FFF;
```

### Bit Shifting (`>>`)
If we want to extract the `X` field from `0xD456`, masking alone is not enough.
`0xD456 & 0x0F00` gives us `0x0400`. We don't want the number `1024` (`0x0400`). We want the number `4` (`0x0004`).
We must shift the bits to the right by 8 positions.

```c
// 0xD456 & 0x0F00 -> 0x0400
// 0x0400 >> 8     -> 0x0004
uint8_t x = (opcode & 0x0F00) >> 8;
```

---

# Instruction Family Deep Dives

We will now dissect every single instruction in the CHIP-8 architecture.

### `00E0` - Clear Screen
* **Action:** Clears the display.
* **Execution:** Iterates over the entire 64x32 display buffer and sets all pixels to `0` (false).
* **C Implementation:** `memset(vm->display, 0, sizeof(vm->display));`
* **Common Bugs:** Misidentifying this as a jump instruction because it starts with 0. 

### `00EE` - Return from Subroutine
* **Action:** Returns from a subroutine.
* **Execution:** Decrements the stack pointer (`sp`), then sets the Program Counter (`pc`) to the value stored at `stack[sp]`.
* **C Implementation:**
  ```c
  vm->sp--;
  vm->pc = vm->stack[vm->sp];
  ```
* **Edge Case:** Returning when the stack pointer is 0 (Stack Underflow).

### `1NNN` - Jump
* **Action:** Sets the program counter to `NNN`.
* **Execution:** Extracts the lowest 12 bits and assigns them directly to `pc`.
* **C Implementation:** `vm->pc = opcode & 0x0FFF;`
* **Note:** Unlike standard instructions, this does *not* increment the PC by 2 afterward.

### `2NNN` - Call Subroutine
* **Action:** Calls a subroutine at `NNN`.
* **Execution:** Saves the current `pc` to the stack, increments `sp`, and sets `pc` to `NNN`.
* **C Implementation:**
  ```c
  vm->stack[vm->sp] = vm->pc;
  vm->sp++;
  vm->pc = opcode & 0x0FFF;
  ```
* **Common Bug:** Forgetting that `pc` must already point to the *next* instruction when calling, otherwise the CPU will return into an infinite loop.

### `3XNN` - Skip if VX == NN
* **Action:** Skips the next instruction if register `V[X]` equals `NN`.
* **Execution:** Compares the values. If true, increments `pc` by 2 (effectively skipping the standard fetch increment).
* **C Implementation:**
  ```c
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t nn = opcode & 0x00FF;
  if (vm->V[x] == nn) {
      vm->pc += 2;
  }
  ```

### `4XNN` - Skip if VX != NN
* **Action:** Skips the next instruction if `V[X]` does not equal `NN`.

### `5XY0` - Skip if VX == VY
* **Action:** Skips the next instruction if `V[X]` equals `V[Y]`.

### `6XNN` - Set VX
* **Action:** Sets `V[X]` to `NN`.
* **C Implementation:** `vm->V[x] = opcode & 0x00FF;`

### `7XNN` - Add to VX (No Carry)
* **Action:** Adds `NN` to `V[X]`.
* **Crucial Edge Case:** This instruction specifically does **NOT** change the `VF` carry flag. If `V[X]` is 250 and we add 10, `V[X]` wraps around to 4. `VF` is untouched. 

---

# The `8XY_` Arithmetic Logic Unit (ALU) Family

All instructions starting with `8` involve two registers, `X` and `Y`, and perform a mathematical or bitwise operation. The specific operation is determined by the last nibble.

### `8XY0` - Set
`vm->V[x] = vm->V[y];`

### `8XY1` - Binary OR
`vm->V[x] = vm->V[x] | vm->V[y];`

### `8XY2` - Binary AND
`vm->V[x] = vm->V[x] & vm->V[y];`

### `8XY3` - Binary XOR
`vm->V[x] = vm->V[x] ^ vm->V[y];`

### `8XY4` - Add with Carry
* **Action:** Adds `V[Y]` to `V[X]`. If the result is greater than 255, sets `VF` to 1. Otherwise, sets `VF` to 0.
* **Implementation Warning:** You must calculate the result and update `VF` safely without overwriting `V[X]` prematurely if `X` happens to be `F`.
* **C Implementation:**
  ```c
  uint16_t sum = vm->V[x] + vm->V[y];
  vm->V[x] = sum & 0xFF; // Store only the lower 8 bits
  vm->V[0xF] = (sum > 255) ? 1 : 0;
  ```

### `8XY5` - Subtract Y from X
* **Action:** Subtracts `V[Y]` from `V[X]`. If `V[X] > V[Y]`, there is NO borrow, so `VF` is set to 1. Otherwise, `0`.

### `8XY6` - Shift Right
* **Action:** Shifts `V[X]` right by 1. `VF` is set to the least significant bit prior to the shift (the bit that was pushed off the edge).
* **Historical Quirk:** In original COSMAC VIP CHIP-8, this instruction assigned `V[Y]` to `V[X]` before shifting. In modern CHIP-8 (SuperChip), it just shifts `V[X]` in place. Modern emulators usually ignore `Y`.

### `8XY7` - Subtract X from Y
* **Action:** Sets `V[X]` to `V[Y] - V[X]`. `VF` is 1 if there is no borrow (if `V[Y] > V[X]`).

### `8XYE` - Shift Left
* **Action:** Shifts `V[X]` left by 1. `VF` is set to the most significant bit prior to the shift.

---

### `9XY0` - Skip if VX != VY
* **Action:** Skips the next instruction if `V[X]` does not equal `V[Y]`.

### `ANNN` - Set Index
* **Action:** Sets the `I` register to `NNN`.
* **Purpose:** This is the primary way to point the CPU at a sprite or memory payload before executing a Draw or Memory Load instruction.

### `BNNN` - Jump with Offset
* **Action:** Jumps to the address `NNN` plus the value of `V0`.
* **Historical Quirk:** In original CHIP-8, this was `BNNN` (Jump to NNN + V0). In SUPERCHIP, it is interpreted as `BXNN` (Jump to XNN + VX). A good emulator implements the original `BNNN` logic.

### `CXNN` - Random
* **Action:** Generates a random 8-bit number, ANDs it with `NN`, and puts the result in `V[X]`.

---

# Drawing Opcode Deep Dive (`DXYN`)

The `DXYN` instruction is the crown jewel of the CHIP-8 instruction set. It handles rendering sprites to the 64x32 display. 

* **`X`:** The register containing the X coordinate.
* **`Y`:** The register containing the Y coordinate.
* **`N`:** The height of the sprite in rows (pixels).

Every sprite in CHIP-8 is exactly 8 pixels (1 byte) wide. 
The sprite data is located in memory, starting at the address stored in the `I` register.

### The Rendering Process
1. Initialize the `VF` collision flag to 0.
2. Loop `row` from 0 to `N - 1`.
3. Read the byte of sprite data from `memory[I + row]`.
4. Loop `col` from 0 to 7 (the 8 bits in the byte).
5. Extract the specific bit from the sprite byte (checking if the pixel should be ON).
6. Calculate the screen coordinate: `screen_x = (V[X] + col) % 64` and `screen_y = (V[Y] + row) % 32`. 
   *(Note the `% 64` and `% 32`! This wraps the sprite across the screen boundaries).*
7. Look at the current screen pixel. If the sprite pixel is ON, and the screen pixel is ON, a collision occurred. Set `VF = 1`.
8. XOR the sprite pixel onto the screen pixel.

### C Implementation
```c
case 0xD000: {
    uint8_t x_pos = vm->V[(opcode & 0x0F00) >> 8] % 64;
    uint8_t y_pos = vm->V[(opcode & 0x00F0) >> 4] % 32;
    uint8_t height = opcode & 0x000F;
    
    vm->V[0xF] = 0; // Reset collision flag
    
    for (int row = 0; row < height; row++) {
        uint8_t sprite_byte = vm->memory[vm->I + row];
        
        for (int col = 0; col < 8; col++) {
            // Extract the bit at 'col' from left to right
            uint8_t sprite_pixel = sprite_byte & (0x80 >> col);
            
            // CHIP-8 wraps sprites starting off-screen, but clips parts that extend past the edge
            // In standard modern emulation, we clip at the edges of the display:
            if ((x_pos + col) >= 64 || (y_pos + row) >= 32) {
                continue;
            }
            
            // Calculate 1D index for the 2D array
            int screen_idx = (y_pos + row) * 64 + (x_pos + col);
            
            if (sprite_pixel) {
                if (vm->display[screen_idx]) {
                    vm->V[0xF] = 1; // Collision!
                }
                vm->display[screen_idx] ^= true; // XOR
            }
        }
    }
    break;
}
```

---

# Keyboard Opcodes

### `EX9E` - Skip if Key Pressed
* **Action:** Skips the next instruction if the key corresponding to the hexadecimal value in `V[X]` is currently held down.
* **Example:** If `V[5]` contains `0xA`, the CPU checks `keypad[0xA]`. If it is true, `pc += 2`.

### `EXA1` - Skip if Key Not Pressed
* **Action:** Skips the next instruction if the key corresponding to `V[X]` is NOT held down.

### `FX0A` - Wait for Key
* **Action:** Stops execution and waits for a key press. When a key is pressed, stores the key's hexadecimal value in `V[X]`.
* **Emulator Trick:** Instead of actually halting the C program (which would crash the windowing system), we simply execute an infinite loop by rewinding the PC.
```c
bool key_pressed = false;
for (int i = 0; i < 16; i++) {
    if (vm->keypad[i]) {
        vm->V[x] = i;
        key_pressed = true;
        break;
    }
}
if (!key_pressed) {
    vm->pc -= 2; // Stay on this instruction until a key is pressed
}
```

---

# Timers and System Opcodes

### `FX07` - Read Delay Timer
* **Action:** Sets `V[X]` to the current value of the delay timer.

### `FX15` - Set Delay Timer
* **Action:** Sets the delay timer to the value in `V[X]`.

### `FX18` - Set Sound Timer
* **Action:** Sets the sound timer to the value in `V[X]`.

### `FX1E` - Add to Index
* **Action:** Adds `V[X]` to the `I` register.

### `FX29` - Get Font Character
* **Action:** Sets `I` to the memory address of the hexadecimal font sprite corresponding to the value in `V[X]`.
* **Execution:** Since we loaded fonts at `0x050`, and each font is 5 bytes long, the math is simply: `I = 0x050 + (V[X] * 5)`.

### `FX33` - Binary-Coded Decimal (BCD) Conversion
* **Action:** Takes the number in `V[X]` (from 0 to 255) and breaks it down into its hundreds, tens, and ones digits. It stores the hundreds digit at memory address `I`, the tens digit at `I+1`, and the ones digit at `I+2`.
* **Purpose:** This was used to draw the score on the screen. The CPU could not draw the number "125". It had to draw "1", then "2", then "5".
* **C Implementation:**
```c
uint8_t value = vm->V[x];
vm->memory[vm->I]     = value / 100;
vm->memory[vm->I + 1] = (value / 10) % 10;
vm->memory[vm->I + 2] = value % 10;
```

### `FX55` - Store Registers
* **Action:** Copies the values of registers `V[0]` through `V[X]` into memory, starting at the address in `I`.
* **Historical Quirk:** Original hardware incremented the `I` register during this operation, so that `I` ended up as `I + X + 1`. Modern emulators often leave `I` unchanged.

### `FX65` - Load Registers
* **Action:** Copies values from memory starting at address `I` into registers `V[0]` through `V[X]`.

---

# Complete Decoder Architecture

A highly performant emulator utilizes a nested switch statement to construct a robust Control Unit. 

```c
void cpu_decode_execute(Chip8 *vm) {
    uint16_t opcode = vm->opcode;
    
    // Extract common fields ahead of time for clean code
    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t y  = (opcode & 0x00F0) >> 4;
    uint8_t n  =  opcode & 0x000F;
    uint8_t nn =  opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;

    // Decode the family (First Nibble)
    switch (opcode & 0xF000) {
        case 0x0000:
            if (opcode == 0x00E0) {
                // Clear Screen
            } else if (opcode == 0x00EE) {
                // Return
            }
            break;
            
        case 0x1000: // Jump
            vm->pc = nnn;
            break;
            
        case 0x8000: // ALU Operations
            switch (n) {
                case 0x0: vm->V[x] = vm->V[y]; break;
                case 0x4: // Add with carry
                    // logic
                    break;
            }
            break;
            
        // ... all other cases
    }
}
```
This architecture is virtually identical to how a real silicon Control Unit directs data. The first 4 bits flow into a massive demultiplexer (the outer `switch`), which routes the electrical signals (the code execution) to a specific subsystem. The remaining bits (`X`, `Y`, `N`) act as selectors for the Register File multiplexers.

---

# Common Emulator Bugs

1. **The VF Overwrite Bug:** Implementing `8XY4` (Add) by writing the result to `V[X]` before calculating the `VF` carry flag. If `X` happens to be `F`, the carry calculation will read the newly modified result instead of the old value, resulting in corrupt math.
2. **The 7XNN Carry Bug:** Setting the `VF` flag during the `7XNN` add instruction. `7XNN` must NEVER modify `VF`. 
3. **The Logical VF Bug:** Resetting `VF` to 0 after `8XY1`, `8XY2`, and `8XY3`. The original COSMAC VIP interpreter reset `VF` during these bitwise operations, but the modern SuperChip interpreter does not. Both implementations exist, leading to ROM incompatibility.
4. **Display Wrapping vs Clipping:** Modern CHIP-8 games assume sprites are clipped at the edges of the screen, not wrapped to the other side.
5. **The PC Increment Double-Step:** Forgetting that Skip instructions (`3XNN`, `4XNN`) must advance the PC by 2, on top of the standard PC += 2 during the Fetch phase.

# Final Mental Model

You now possess the entire blueprint of instruction decoding. You understand how 16 bits of binary data can represent an endless sequence of logic, mathematics, rendering, and timing. 

To rebuild this from memory:
1. Fetch 2 bytes, bitwise OR them into a `uint16_t`.
2. Extract the Most Significant Nibble using `& 0xF000`.
3. Route to the correct instruction logic via a `switch` statement.
4. Extract arguments using `& 0x0F00`, `& 0x00F0`, etc.
5. Mutate the central state struct safely.
6. Handle `VF` with extreme caution. 

This completes the ultimate guide to the CHIP-8 instruction set architecture.
