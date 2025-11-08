#include "raylib.h"

int main() {
    // Initialize window
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Raylib Hello World");

    // Set target FPS
    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        // TODO: Update your variables here

        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Hello, Raylib!", screenWidth/2 - MeasureText("Hello, Raylib!", 20)/2, screenHeight/2 - 10, 20, BLACK);
            DrawCircle(screenWidth/2, screenHeight/2 + 50, 25, RED);
        EndDrawing();
    }

    // De-initialize
    CloseWindow();

    return 0;
}