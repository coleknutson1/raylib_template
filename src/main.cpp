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
const float SQUARE_GRAVITY_STRENGTH = 100000.0f; // Square-to-square orbital pull strength
const float BLACK_HOLE_GRAVITY = 5000000.0f; // Black hole gravitational pull
const float BLACK_HOLE_RADIUS = 32.0f; // 16 pixel diameter = 8 radius
const float BLACK_HOLE_INTERVAL = 10.0f; // 30 seconds between black holes
const float BLACK_HOLE_DURATION = 15.0f; // 15 seconds duration
const float BACKGROUND_FADE_SPEED = 15.0f; // How fast background fades to black

// Black hole structure
struct BlackHole {
    float x, y;        // Position
    bool active;       // Is the black hole currently active
    float timer;       // Time remaining for current black hole
    float nextSpawn;   // Time until next black hole spawns
};

// Square structure
struct Square {
    float x, y;        // Position
    float vx, vy;      // Velocity
    Color color;       // Color
    bool destroyed;    // Is this square destroyed (waiting for respawn)
};

// Function declarations
void GenerateSquare(Square& square, int screenWidth, int screenHeight);
void UpdateSquare(Square& square, int screenWidth, int screenHeight);
void ApplySquareGravity(std::vector<Square>& squares);
void ApplyBlackHoleGravity(std::vector<Square>& squares, BlackHole& blackHole);
void CheckSquareCollisions(std::vector<Square>& squares);
void CheckBlackHoleCollisions(std::vector<Square>& squares, BlackHole& blackHole);
void UpdateBlackHole(BlackHole& blackHole, int screenWidth, int screenHeight);
void RespawnDestroyedSquares(std::vector<Square>& squares, int screenWidth, int screenHeight);
void DrawSquare(const Square& square);
void DrawBlackHole(const BlackHole& blackHole);
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

    // Initialize black hole
    BlackHole blackHole = {0, 0, false, 0.0f, BLACK_HOLE_INTERVAL};

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

        // Update black hole
        UpdateBlackHole(blackHole, screenWidth, screenHeight);

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

        // Apply black hole gravity
        if (blackHole.active) {
            ApplyBlackHoleGravity(squares, blackHole);
        }

        // Apply square-to-square orbital pull
        ApplySquareGravity(squares);

        // Check collisions between squares
        CheckSquareCollisions(squares);

        // Check collisions with black hole
        if (blackHole.active) {
            CheckBlackHoleCollisions(squares, blackHole);
        }

        // Respawn destroyed squares when black hole disappears
        if (!blackHole.active && blackHole.timer <= 0.0f) {
            RespawnDestroyedSquares(squares, screenWidth, screenHeight);
        }

        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Draw all active squares
            for (const auto& square : squares) {
                if (!square.destroyed) {
                    DrawSquare(square);
                }
            }

            // Draw black hole if active
            if (blackHole.active) {
                DrawBlackHole(blackHole);
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
    square.destroyed = false;
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

                // 10% chance to exchange colors
                if (rand() % 100 < 10) {
                    Color tempColor = s1.color;
                    s1.color = s2.color;
                    s2.color = tempColor;
                }

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

void UpdateBlackHole(BlackHole& blackHole, int screenWidth, int screenHeight) {
    blackHole.nextSpawn -= GetFrameTime();

    if (blackHole.active) {
        // Move black hole slowly towards center
        float centerX = screenWidth / 2.0f;
        float centerY = screenHeight / 2.0f;

        float dx = centerX - blackHole.x;
        float dy = centerY - blackHole.y;
        float distance = sqrt(dx*dx + dy*dy);

        if (distance > 1.0f) {
            // Move towards center at a slow speed
            float moveSpeed = 50.0f; // pixels per second
            float moveX = (dx / distance) * moveSpeed * GetFrameTime();
            float moveY = (dy / distance) * moveSpeed * GetFrameTime();

            blackHole.x += moveX;
            blackHole.y += moveY;
        }

        blackHole.timer -= GetFrameTime();
        if (blackHole.timer <= 0.0f) {
            blackHole.active = false;
            blackHole.nextSpawn = BLACK_HOLE_INTERVAL;
        }
    } else if (blackHole.nextSpawn <= 0.0f) {
        // Spawn new black hole at random position
        blackHole.x = rand() % (screenWidth - 32) + 16; // Keep away from edges
        blackHole.y = rand() % (screenHeight - 32) + 16;
        blackHole.active = true;
        blackHole.timer = BLACK_HOLE_DURATION;
    }
}

void ApplyBlackHoleGravity(std::vector<Square>& squares, BlackHole& blackHole) {
    for (auto& square : squares) {
        if (square.destroyed) continue;

        // Calculate distance and direction to black hole
        float dx = blackHole.x - (square.x + SQUARE_SIZE/2);
        float dy = blackHole.y - (square.y + SQUARE_SIZE/2);
        float distance = sqrt(dx*dx + dy*dy);

        // Avoid division by zero
        if (distance > 1.0f) {
            // Calculate gravitational force (inverse square law)
            float force = BLACK_HOLE_GRAVITY / (distance * distance);

            // Normalize direction and apply force
            float dirX = dx / distance;
            float dirY = dy / distance;

            // Apply force to velocity
            square.vx += dirX * force * GetFrameTime();
            square.vy += dirY * force * GetFrameTime();
        }
    }
}

void CheckBlackHoleCollisions(std::vector<Square>& squares, BlackHole& blackHole) {
    for (auto& square : squares) {
        if (square.destroyed) continue;

        // Calculate distance between square center and black hole center
        float dx = blackHole.x - (square.x + SQUARE_SIZE/2);
        float dy = blackHole.y - (square.y + SQUARE_SIZE/2);
        float distance = sqrt(dx*dx + dy*dy);

        // If square touches black hole (distance <= radius + half square size)
        if (distance <= BLACK_HOLE_RADIUS + SQUARE_SIZE/2) {
            square.destroyed = true;
        }
    }
}

void RespawnDestroyedSquares(std::vector<Square>& squares, int screenWidth, int screenHeight) {
    for (auto& square : squares) {
        if (square.destroyed) {
            // Respawn from random edge
            int edge = rand() % 4; // 0=top, 1=right, 2=bottom, 3=left

            switch (edge) {
                case 0: // Top edge
                    square.x = rand() % screenWidth;
                    square.y = -SQUARE_SIZE;
                    square.vx = (rand() % 200 - 100) / 10.0f;
                    square.vy = abs((rand() % 100) / 10.0f) + 5.0f; // Always shoot downward
                    break;
                case 1: // Right edge
                    square.x = screenWidth;
                    square.y = rand() % screenHeight;
                    square.vx = -abs((rand() % 100) / 10.0f) - 5.0f; // Always shoot leftward
                    square.vy = (rand() % 200 - 100) / 10.0f;
                    break;
                case 2: // Bottom edge
                    square.x = rand() % screenWidth;
                    square.y = screenHeight;
                    square.vx = (rand() % 200 - 100) / 10.0f;
                    square.vy = -abs((rand() % 100) / 10.0f) - 5.0f; // Always shoot upward
                    break;
                case 3: // Left edge
                    square.x = -SQUARE_SIZE;
                    square.y = rand() % screenHeight;
                    square.vx = abs((rand() % 100) / 10.0f) + 5.0f; // Always shoot rightward
                    square.vy = (rand() % 200 - 100) / 10.0f;
                    break;
            }

            square.destroyed = false;
        }
    }
}

void DrawSquare(const Square& square) {
    DrawRectangle(square.x, square.y, SQUARE_SIZE, SQUARE_SIZE, square.color);
}

void DrawBlackHole(const BlackHole& blackHole) {
    DrawCircle(blackHole.x, blackHole.y, BLACK_HOLE_RADIUS, BLACK);
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