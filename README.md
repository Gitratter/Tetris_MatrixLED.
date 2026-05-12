# Tetris Matrix LED

A Tetris game implemented on an LED matrix using Arduino-compatible microcontrollers.

## Overview

This project is a hardware-based Tetris game controlled by a joystick and displayed on LED matrices.  
The purpose of this project is to learn embedded systems programming, matrix LED control, real-time processing, and game logic implementation in C/C++.

The system combines multiple electronic components including:

- LED matrix display
- Joystick input
- 7-segment display
- I2C LCD
- Sound module

## Features

- Real-time Tetris gameplay
- Joystick-based intuitive controls
- Tetromino rotation and collision detection
- Line clear processing
- Score management
- Time limit display using LCD
- Sound feedback during operation

## Hardware

- Arduino / Raspberry Pi Pico W
- 8x8 LED Matrix ×2
- Joystick module
- 4-digit 7-segment display
- I2C LCD1602
- Buzzer / sound module

## Software

- Language: C / C++
- Development Environment: Arduino IDE
- Embedded programming
- Real-time input processing

## System Structure

```text
Joystick
   ↓
Microcontroller
   ├── LED Matrix (game screen)
   ├── LCD1602 (score / timer)
   ├── 7-segment LED
   └── Buzzer
```

## Technical Challenges

### Matrix LED Control
Implemented dynamic control of multiple LED matrices while maintaining smooth gameplay performance.

### Collision Detection
Designed collision logic for wall boundaries, stacked blocks, and rotation handling.

### Resource Constraints
Optimized processing for microcontroller environments with limited memory and performance.

## Future Improvements

- Add hold function
- Add next block preview
- Improve sound effects
- Implement difficulty levels
- FPGA implementation in the future

## Demo

(Add images or videos here)

## Learning Outcomes

Through this project, I learned:

- Embedded systems programming
- Real-time control
- Hardware-software integration
- State management
- Game logic implementation
- Debugging on microcontrollers

## License

MIT License
