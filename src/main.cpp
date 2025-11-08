#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

// Constants
const int SQUARE_SIZE = 16;
const int INITIAL_SQUARES = 100;
const int SQUARE_INCREMENT = 50;
const float INPUT_INTERVAL = 0.25f; // 0.5 seconds
const float MOUSE_GRAVITY_STRENGTH = 0.0f; // Weak orbital pull strength
const float SQUARE_GRAVITY_STRENGTH = 500000.0f; // Square-to-square orbital pull strength

// Square structure
struct Square {
    float x, y;        // Position
    float vx, vy;      // Velocity
    Color color;       // Color
};

// Function declarations
void GenerateSquare(Square& square, int screenWidth, int screenHeight);
void UpdateSquare(Square& square, int screenWidth, int screenHeight);
void ApplySquareGravity(std::vector<Square>& squares);
void CheckSquareCollisions(std::vector<Square>& squares);
void DrawSquare(const Square& square);
void AddSquares(std::vector<Square>& squares, int count, int screenWidth, int screenHeight);
void RemoveSquares(std::vector<Square>& squares, int count);

int main() {
    // Initialize window
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "Raylib Stress Test");

    // Set to full screen
    ToggleFullscreen();

    // Set target FPS
    // SetTargetFPS(60);

    // Initialize random seed
    srand(time(NULL));

    // Create initial squares
    std::vector<Square> squares;
    AddSquares(squares, INITIAL_SQUARES, screenWidth, screenHeight);

    // Input timing variables
    float lastInputTime = 0.0f;
    bool upPressed = false;
    bool downPressed = false;

    // Main game loop
    while (!WindowShouldClose()) {
        float currentTime = GetTime();

        // Handle input with timing
        if (IsKeyDown(KEY_UP)) {
            if (!upPressed || (currentTime - lastInputTime) >= INPUT_INTERVAL) {
                AddSquares(squares, SQUARE_INCREMENT, screenWidth, screenHeight);
                lastInputTime = currentTime;
                upPressed = true;
            }
        } else {
            upPressed = false;
        }

        if (IsKeyDown(KEY_DOWN)) {
            if (!downPressed || (currentTime - lastInputTime) >= INPUT_INTERVAL) {
                RemoveSquares(squares, SQUARE_INCREMENT);
                lastInputTime = currentTime;
                downPressed = true;
            }
        } else {
            downPressed = false;
        }

        // Update squares
        for (auto& square : squares) {
            UpdateSquare(square, screenWidth, screenHeight);
        }

        // Apply mouse orbital pull
        Vector2 mousePos = GetMousePosition();
        for (auto& square : squares) {
            // Calculate distance and direction to mouse
            float dx = mousePos.x - (square.x + SQUARE_SIZE/2);
            float dy = mousePos.y - (square.y + SQUARE_SIZE/2);
            float distance = sqrt(dx*dx + dy*dy);

            // Avoid division by zero and apply minimum distance
            if (distance > 1.0f) {
                // Calculate gravitational force (inverse square law)
                float force = MOUSE_GRAVITY_STRENGTH / (distance * distance);

                // Normalize direction and apply force
                float dirX = dx / distance;
                float dirY = dy / distance;

                // Apply force to velocity (weak orbital pull)
                square.vx += dirX * force * GetFrameTime();
                square.vy += dirY * force * GetFrameTime();
            }
        }

        // Apply square-to-square orbital pull
        ApplySquareGravity(squares);

        // Check collisions between squares
        CheckSquareCollisions(squares);

        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Draw all squares
            for (const auto& square : squares) {
                DrawSquare(square);
            }

            // Draw FPS counter
            int fps = GetFPS();
            DrawText(TextFormat("FPS: %d", fps), 10, 10, 20, BLACK);

            // Draw body count
            DrawText(TextFormat("Bodies: %d", (int)squares.size()), 10, 35, 20, BLACK);

        EndDrawing();
    }

    // De-initialize
    CloseWindow();

    return 0;
}

// Function implementations
void GenerateSquare(Square& square, int screenWidth, int screenHeight) {
    square.x = rand() % (screenWidth - SQUARE_SIZE);
    square.y = rand() % (screenHeight - SQUARE_SIZE);
    square.vx = (rand() % 1000 - 100) / 10.0f; // Random velocity between -10 and 10
    square.vy = (rand() % 1000 - 100) / 10.0f;
    square.color = Color{
        (unsigned char)(rand() % 256),
        (unsigned char)(rand() % 256),
        (unsigned char)(rand() % 256),
        255
    };
}

void UpdateSquare(Square& square, int screenWidth, int screenHeight) {
    // Update position (frame-rate independent)
    square.x += square.vx * GetFrameTime();
    square.y += square.vy * GetFrameTime();

    // Bounce off walls
    if (square.x <= 0 || square.x >= screenWidth - SQUARE_SIZE) {
        square.vx = -square.vx;
        square.x = square.x <= 0 ? 0 : screenWidth - SQUARE_SIZE;
    }
    if (square.y <= 0 || square.y >= screenHeight - SQUARE_SIZE) {
        square.vy = -square.vy;
        square.y = square.y <= 0 ? 0 : screenHeight - SQUARE_SIZE;
    }
}

void CheckSquareCollisions(std::vector<Square>& squares) {
    for (size_t i = 0; i < squares.size(); ++i) {
        for (size_t j = i + 1; j < squares.size(); ++j) {
            Square& s1 = squares[i];
            Square& s2 = squares[j];

            // Check if squares overlap
            if (s1.x < s2.x + SQUARE_SIZE && s1.x + SQUARE_SIZE > s2.x &&
                s1.y < s2.y + SQUARE_SIZE && s1.y + SQUARE_SIZE > s2.y) {

                // Simple elastic collision - swap velocities
                float tempVx = s1.vx;
                float tempVy = s1.vy;
                s1.vx = s2.vx;
                s1.vy = s2.vy;
                s2.vx = tempVx;
                s2.vy = tempVy;

                // Separate overlapping squares
                float overlapX = (s1.x + SQUARE_SIZE/2) - (s2.x + SQUARE_SIZE/2);
                float overlapY = (s1.y + SQUARE_SIZE/2) - (s2.y + SQUARE_SIZE/2);

                if (abs(overlapX) > abs(overlapY)) {
                    if (overlapX > 0) {
                        s1.x = s2.x + SQUARE_SIZE;
                    } else {
                        s1.x = s2.x - SQUARE_SIZE;
                    }
                } else {
                    if (overlapY > 0) {
                        s1.y = s2.y + SQUARE_SIZE;
                    } else {
                        s1.y = s2.y - SQUARE_SIZE;
                    }
                }
            }
        }
    }
}

void ApplySquareGravity(std::vector<Square>& squares) {
    // Calculate gravitational forces between all pairs of squares
    for (size_t i = 0; i < squares.size(); ++i) {
        for (size_t j = i + 1; j < squares.size(); ++j) {
            Square& s1 = squares[i];
            Square& s2 = squares[j];

            // Calculate distance and direction between squares
            float dx = (s2.x + SQUARE_SIZE/2) - (s1.x + SQUARE_SIZE/2);
            float dy = (s2.y + SQUARE_SIZE/2) - (s1.y + SQUARE_SIZE/2);
            float distance = sqrt(dx*dx + dy*dy);

            // Avoid division by zero and apply minimum distance
            if (distance > SQUARE_SIZE) {
                // Calculate gravitational force (inverse square law)
                float force = SQUARE_GRAVITY_STRENGTH / (distance * distance);

                // Normalize direction
                float dirX = dx / distance;
                float dirY = dy / distance;

                // Apply force to both squares (equal and opposite)
                s1.vx += dirX * force * GetFrameTime();
                s1.vy += dirY * force * GetFrameTime();
                s2.vx -= dirX * force * GetFrameTime();
                s2.vy -= dirY * force * GetFrameTime();
            }
        }
    }
}

void DrawSquare(const Square& square) {
    DrawRectangle(square.x, square.y, SQUARE_SIZE, SQUARE_SIZE, square.color);
}

void AddSquares(std::vector<Square>& squares, int count, int screenWidth, int screenHeight) {
    for (int i = 0; i < count; ++i) {
        Square newSquare;
        GenerateSquare(newSquare, screenWidth, screenHeight);
        squares.push_back(newSquare);
    }
}

void RemoveSquares(std::vector<Square>& squares, int count) {
    for (int i = 0; i < count && !squares.empty(); ++i) {
        squares.pop_back();
    }
}