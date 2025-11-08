# Raylib Hello World C++

A simple hello world project using raylib in C++ with VS Code integration.

## Prerequisites

- raylib (already installed on your system)
- g++ or clang++ with C++17 support
- make
- gdb (for debugging)
- VS Code with C++ extension

## Building and Running

### Using VS Code Tasks (Recommended)

1. Open the project in VS Code
2. Press `Ctrl+Shift+B` to build the project
3. Press `Ctrl+Shift+P` and select "Tasks: Run Task" then "run" to execute

### Using Terminal

```bash
# Build the project
make

# Build with debug symbols
make debug

# Build optimized release
make release

# Run the executable
make run

# Clean build artifacts
make clean
```

## Debugging

1. Set breakpoints in your code
2. Press `F5` to start debugging (this will build in debug mode and launch the debugger)
3. Use VS Code's debug panel to step through code, inspect variables, etc.

## Project Structure

```text
raylib_template/
├── .vscode/
│   ├── tasks.json          # Build, run, and clean tasks
│   ├── launch.json         # Debug configuration
│   └── c_cpp_properties.json  # IntelliSense configuration
├── src/
│   └── main.cpp            # Main hello world source
├── Makefile                # Build system
├── .gitignore              # Git ignore file
└── README.md               # This file
```

## Features

- 800x600 window titled "Raylib Hello World"
- Displays "Hello, Raylib!" text in the center
- Draws a red circle below the text
- Runs at 60 FPS
- Closes on ESC key or window close button

## Next Steps

- Add more raylib features (textures, sounds, input handling)
- Create game objects and classes
- Implement game logic and mechanics
- Add more complex graphics and animations