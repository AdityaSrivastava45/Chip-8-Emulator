# CHIP-8 Emulator

A fully functional CHIP-8 emulator written in **C** using **SDL2**, designed to emulate the original CHIP-8 virtual machine with accurate instruction execution, graphics rendering, keyboard input handling, and timer management.

This project was built to gain a deeper understanding of computer architecture, CPU design, memory management, and low-level systems programming by recreating an entire virtual machine from scratch.

---

## Demo

> <img width="485" height="263" alt="image" src="https://github.com/user-attachments/assets/816ec49f-0d86-4ca8-9859-eac64b78149f" />

```
assets/demo.gif
```

---

## Features

* Complete CHIP-8 CPU implementation
* Fetch → Decode → Execute instruction pipeline
* 4KB memory architecture
* 16 general-purpose registers (V0–VF)
* Index register (I)
* Program Counter (PC)
* Stack and Stack Pointer
* Delay Timer (60 Hz)
* Sound Timer (60 Hz)
* 64 × 32 monochrome display
* SDL2 graphics rendering
* Keyboard input mapping
* ROM loading
* Sprite rendering using XOR logic
* Collision detection
* Fontset loading
* Configurable execution speed
* Cross-platform architecture

---

# Project Structure

```
chip8-emulator/
│
├── include/
│   ├── chip8.h
│   ├── display.h
│   ├── input.h
│   └── config.h
│
├── src/
│   ├── chip8.c
│   ├── display.c
│   ├── input.c
│   ├── instruction.c
│   ├── memory.c
│   └── main.c
│
├── roms/
│
├── assets/
│
├── screenshots/
│
├── Makefile
│
└── README.md
```

---

# CHIP-8 Architecture

The emulator recreates the original CHIP-8 virtual machine, originally developed in the 1970s.

## Memory

The CHIP-8 contains **4096 bytes (4KB)** of memory.

```
0x000 ───────────────
        Interpreter

0x050 ───────────────
        Font Set

0x200 ───────────────
        Program ROM

0xFFF ───────────────
```

Programs are loaded beginning at **0x200**, matching the original hardware specification.

---

## Registers

| Register | Purpose                      |
| -------- | ---------------------------- |
| V0–VF    | 16 General Purpose Registers |
| I        | Index Register               |
| PC       | Program Counter              |
| SP       | Stack Pointer                |
| DT       | Delay Timer                  |
| ST       | Sound Timer                  |

---

## Display

Resolution:

```
64 × 32 pixels
```

Each pixel is either:

```
ON
OFF
```

Rendering is performed using SDL2.

Sprites are drawn using XOR operations exactly as defined by the CHIP-8 specification.

---

## Input

The emulator supports the original hexadecimal keypad.

```
1 2 3 C
4 5 6 D
7 8 9 E
A 0 B F
```

Mapped to a modern keyboard for ease of use.

---

# Fetch → Decode → Execute Cycle

Every CPU cycle consists of three stages.

## 1. Fetch

Read two consecutive bytes from memory.

```
opcode =
(memory[PC] << 8)
| memory[PC + 1]
```

Example:

```
Memory

0x200 : A2
0x201 : F0

↓

Opcode

0xA2F0
```

---

## 2. Decode

The opcode is separated using bit masks.

Example:

```
Opcode

0x6A05

↓

Instruction

6XNN

↓

Register

VA

↓

Value

05
```

---

## 3. Execute

The decoded instruction performs the corresponding operation.

Example:

```
6A05

↓

VA = 5
```

After execution the Program Counter advances to the next instruction unless modified by a jump or call.

---

# Supported Instructions

The emulator implements the complete CHIP-8 instruction set, including:

* Screen operations
* Arithmetic instructions
* Bitwise operations
* Conditional branching
* Random number generation
* Sprite drawing
* Keyboard input
* Timer instructions
* Memory manipulation
* Stack operations
* Subroutine calls
* Program flow control

---

# Graphics Pipeline

```
ROM

↓

CPU

↓

Decode Opcode

↓

Draw Sprite

↓

Framebuffer

↓

SDL Renderer

↓

Window
```

Sprites are rendered using XOR drawing.

If a collision occurs, register **VF** is automatically updated.

---

# Timers

The emulator implements both hardware timers.

### Delay Timer

Counts down at **60Hz**.

Used for animation timing.

---

### Sound Timer

Counts down at **60Hz**.

Produces a beep while non-zero.

---

# ROM Loading

The emulator loads external CHIP-8 ROMs into memory beginning at address:

```
0x200
```

Safety checks include:

* File existence
* Maximum ROM size
* Memory overflow prevention

---

# Build Instructions

## Linux

Install SDL2

```bash
sudo apt install libsdl2-dev
```

Compile

```bash
make
```

Run

```bash
./chip8 roms/PONG.ch8
```

---

## Windows

Using MinGW

```bash
gcc src/*.c -lSDL2 -o chip8.exe
```

Run

```bash
./build/chip8.exe roms/space_invaders.ch8
```

---

# Example ROMs

Popular CHIP-8 ROMs include:

* Pong
* Space Invaders
* Tetris
* IBM Logo
* Maze
* Brick

---

# Testing

The emulator has been tested using multiple public CHIP-8 ROMs to verify:

* Correct opcode execution
* Graphics rendering
* Collision detection
* Keyboard input
* Timer accuracy
* Stack behavior
* Memory operations

---

# Acknowledgements

* CHIP-8 Technical Reference
* Cowgod's CHIP-8 Technical Reference
* Tobias V. Langhoff's CHIP-8 Guide
* SDL2 Documentation

---

# License

This project is licensed under the MIT License.

---

## Author

**Aditya Srivastava**

Computer Science Engineering Student

Interested in Machine Learning, Artificial Intelligence, Systems Programming, Emulator Development, and High-Performance Computing.

If you found this project interesting, consider giving it a ⭐.
