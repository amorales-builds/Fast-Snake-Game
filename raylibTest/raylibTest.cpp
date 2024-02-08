// Amisadai Morales
// 7/Feb/2024
// followed "Programming With Nick" YouTube tutorial, plus added pause and high score system

#include <iostream>
#include "raylib.h"
#include "raymath.h"
#include <deque>

using namespace std;

Color green = {126, 175, 96, 255};
Color darkGreen = {43, 51, 24, 255};
Color red = { 185, 71, 35, 255 };
Color yellow = { 240, 250, 10, 255 };

int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;

bool ElementInDeque(Vector2 element, deque<Vector2> deque) {
    for (unsigned int i = 0; i < deque.size(); i++) {
        if (Vector2Equals(deque[i], element)) {
            return true;
        }
    }
    return false;
}

bool eventTriggered(double interval) {
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval) {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

class Snake {

public:
    deque<Vector2> body = { Vector2{6,9}, Vector2{5,9}, Vector2{4,9} };
    Vector2 direction = { 1,0 };
    bool addSegment = false;

    void Draw() {
        for (unsigned int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{ offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize}; 
            DrawRectangleRounded(segment, 0.5, 6, darkGreen);
            //DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, darkGreen);
        }
    }
    void Update() {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true) {
            addSegment = false;
        }
        else {
            body.pop_back();
        }
    }
    void Reset() {
        body = { Vector2{6,9}, Vector2{5,9}, Vector2{4,9} };
        direction = { 1,0 };
    }
};

class Food {

public:

    Vector2 position;
    Texture2D texture;

    /*          USED FOR IMAGE ( I don't have a good one rn )
    Food(deque<Vector2> snakeBody) {
        Image image = LoadImage("C:/Users/ami10/Pictures/notebook_button_hover.PNG");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody);
    }
    

    ~Food() {
        UnloadTexture(texture);
    }
    */

    Food(deque<Vector2> snakeBody) {
        position = GenerateRandomPos(snakeBody);
    }

    void Draw() {
        Rectangle ball = Rectangle{ (offset + position.x * cellSize) + 2, (offset + position.y * cellSize) + 2, (float)cellSize - 4, (float)cellSize - 4 };
        DrawRectangleRounded(ball, 1, 10, yellow); // to curve rectangle into ball

        //DrawRectangle(position.x * cellSize, position.y * cellSize, cellSize, cellSize, yellow); //used when no image
        //DrawTexture(texture, position.x * cellSize, position.y * cellSize, WHITE); // used when image
    }

    Vector2 GenerateRandomCell() {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x,y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody) {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody)) {
            position = GenerateRandomCell();
        }
        return position;
    }

};

class Game {

public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    bool pause = false;
    bool reset = false;
    bool newHighScore = false;
    int score = 0;
    int highScore = 0;
    Sound eatSound;
    Sound wallSound;
    Sound pauseSound;
    Sound resumeSound;
    Sound highScoreSound;

    Game() {
        InitAudioDevice();
        eatSound = LoadSound("C:/Users/ami10/Music/terminalBell.mp3");
        wallSound = LoadSound("C:/Users/ami10/Music/break.mp3");
        pauseSound = LoadSound("C:/Users/ami10/Music/quickFixes.mp3");
        resumeSound = LoadSound("C:/Users/ami10/Music/dingUp.mp3");
        highScoreSound = LoadSound("C:/Users/ami10/Music/dingUp2.mp3");
    }

    ~Game() {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        UnloadSound(pauseSound);
        CloseAudioDevice();
    }

    void Draw() {
        food.Draw();
        snake.Draw();
    }

    void Update() {
        if (running) {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }        
    }

    void CheckCollisionWithFood() {
        if (Vector2Equals(snake.body[0], food.position)) {
            food.position = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;
            score ++;            
            if (score > highScore) {
                highScore++;
                if (newHighScore == true) {
                    PlaySound(highScoreSound);
                    newHighScore = false;
                }
            }   
            else {
                if (score == highScore) {
                    newHighScore = true;
                }
            }
            PlaySound(eatSound);
        }
    }
    void CheckCollisionWithEdges() {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1) {
            GameOver();
        }
        if (snake.body[0].y == cellCount || snake.body[0].y == -1) {
            GameOver();
        }
    }
    
    void CheckCollisionWithTail() {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody)) {
            GameOver();
        }
    }

    void GameOver() {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        reset = true;
        score = 0;
        PlaySound(wallSound);
    }
    void GamePause() {
        running = !running; 
        pause = !pause;
    }
};

int main()
{
    cout << "Starting the game..." << endl;
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Retro Snake Game");
    SetTargetFPS(60);

    Game game = Game();

    while (WindowShouldClose() == false) {

        BeginDrawing();

        if (IsKeyPressed(KEY_SPACE) && game.reset == false) {
            game.GamePause();
            // sounds:
            if (game.pause == true) {
                PlaySound(game.pauseSound);
            }
            else {
                PlaySound(game.resumeSound);
            }
        }

        if (eventTriggered(0.2 - (game.score * .0025))) {
            game.Update();
        }

        if (IsKeyPressed(KEY_UP) && game.pause == false && game.snake.direction.y != 1) {
            game.snake.direction = { 0, -1 };
            game.running = true;
            game.reset = false;
        }

        if (IsKeyPressed(KEY_DOWN) && game.pause == false && game.snake.direction.y != -1) {
            game.snake.direction = { 0, 1 };
            game.running = true;
            game.reset = false;
        }

        if (IsKeyPressed(KEY_LEFT) && game.pause == false && game.snake.direction.x != 1) {
            game.snake.direction = { -1, 0 };
            game.running = true;
            game.reset = false;
        }

        if (IsKeyPressed(KEY_RIGHT) && game.pause == false && game.snake.direction.x != -1) {
            game.snake.direction = { 1, 0 };
            game.running = true;
            game.reset = false;
        }

        // Drawing
        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 3, darkGreen);
        DrawText("Fast Snake Game", offset - 5, 20, 40, darkGreen);
        DrawText(TextFormat("score: %i", game.score), 680, 28, 30, darkGreen);
        DrawText(TextFormat("High Score: %i", game.highScore), offset - 5, offset + cellSize * cellCount + 20, 20, yellow);
        if (game.pause == true) {
            DrawText("PAUSE", 645, offset + cellSize * cellCount + 12 , 50, red);
        }
        if (game.reset == true) {
            DrawText("Press ARROW KEYS to continue", 320, offset + cellSize * cellCount + 20, 30, darkGreen);
        }

        game.Draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

