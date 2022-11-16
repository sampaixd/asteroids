#include "raylib.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>

const int screenWidth = 1920;
const int screenHeight = 1080;
const float drag = 0.99f;     // default drag when not pressing W
const float brakeDrag = 0.9f; // drag when pressing S
const int targetFPS = 60;
const int maxBullets = 300;             // amount of bullets shot before they are recycled
const float defaultFireCooldown = 0.1f; // time between shots in seconds
const int maxAsteroids = 100;           // amount of asteroids before they are recycled
const float timeBetweenAsteroidSpawn = 1;
const float asteroidSpeedConstant = 200; // min speed will be this/max size, max speed will be this/min size
const int asteroidMaxSize = 100;
const int asteroidMinSize = 30;
const float invincibleDuration = 2; // invincibility time after taking damage in seconds

typedef struct
{
    Vector2 center;
    Vector2 velocity;
    double angle;
    float rotation;
    float speed;
    float size;
    float hp;
} entity;

void InitGame(entity *player, entity *bullet, entity *asteroid, int *score)
{
    // init player
    *player = (entity){.center = (Vector2){screenWidth / 2, screenHeight / 2},
                       .velocity = (Vector2){0, 0},
                       .angle = 0,
                       .rotation = 0.1f,
                       .speed = 5.0f,
                       .size = 50.0f,
                       .hp = 5};

    // init bullets
    for (int i = 0; i < maxBullets; i++)
    {
        bullet[i] = (entity){
            .center = {-10, -10},
            .velocity = {0, 0},
            .angle = 0,
            .rotation = 0,
            .speed = 15,
            .size = 5,
            .hp = 1};
    }

    // init asteroids
    for (int i = 0; i < maxAsteroids; i++)
    {
        asteroid[i] = (entity){
            .center = {-25, -25},
            .velocity = {0, 0},
            .angle = 0,
            .rotation = 0,
            .speed = 3,
            .size = 25,
            .hp = 10};
    }

    score = 0;
}
void GetRandomAsteroidSpawn(Vector2 *center, float *angle, float size)
{
    switch (rand() % 4)
    {
    case 0: // start in left corner
        center->x = -size;
        center->y = rand() % screenHeight;
        *angle = (rand() % (int)(100 * PI) / 100) - PI / 2; // (-90)-90 degrees
        break;

    case 1: // start in right corner
        center->x = screenWidth + size;
        center->y = rand() % screenHeight;
        *angle = (rand() % (int)(100 * PI) / 100) + PI / 2; // 90-270 degrees
        break;

    case 2: // start in bottom
        center->x = rand() % screenWidth;
        center->y = -size;
        *angle = (rand() % (int)(100 * PI) / 100); // 0-180 degrees
        break;

    case 3: // start in top
        center->x = rand() % screenWidth;
        center->y = screenHeight + size;
        *angle = (rand() % (int)(100 * PI) / 100) + PI; // 180-360 degrees
        break;
    }
}
void SpawnAsteroid(entity *asteroid, Vector2 center, float size, float angle, int *asteroidPointer)
{
    float hp = size / 5;
    float speed = asteroidSpeedConstant / size;
    Vector2 velocity = {speed * cos(angle),
                        speed * sin(angle)};
    *asteroid = (entity){
        .center = center,
        .velocity = velocity,
        .angle = angle,
        .speed = speed,
        .size = size,
        .hp = hp};
        *asteroidPointer >= maxAsteroids - 1 ? *asteroidPointer = 0 : *asteroidPointer++;
}
int main()
{
    srand(time(NULL));
    InitWindow(screenWidth, screenHeight, "asteroids");
    SetTargetFPS(targetFPS);
    entity player;

    bool playerIsInvincible = false;
    bool playerIsWhite = false;
    float timeSpentInvincible = 0;
    float invincibleColorSwitchCheck = 0; // will switch from white to red ship based on how long you have been invincible
    int score = 0;

    entity bullet[maxBullets];
    int bulletPointer = 0;
    float timeSinceLastShot = 0;
    float fireCooldown = defaultFireCooldown;

    entity asteroid[maxAsteroids];
    int asteroidPointer = 0;
    float timeSinceLastAstroidSpawn = 0;
    InitGame(&player, bullet, asteroid, &score);
    while (!WindowShouldClose())
    {
        if (player.hp <= 0)
        {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText(TextFormat("GAME OVER!\n"
                                "YOUR SCORE IS: %i\n"
                                "PRESS R TO RESTART",
                                score),
                     (screenWidth - 100) / 2, (screenHeight - 100) / 2, 50, GREEN);
            if (IsKeyDown(KEY_R))
            {
                InitGame(&player, bullet, asteroid, &score);
                timeSpentInvincible = invincibleDuration; // this will essentially init invincibility
            }
            else
            {
                EndDrawing();
                continue;
            }
        }

        // time based variables updated
        if (playerIsInvincible)
        {
            timeSpentInvincible += GetFrameTime();
            invincibleColorSwitchCheck += powf(20, (timeSpentInvincible / invincibleDuration));
        }
        timeSinceLastShot += GetFrameTime();
        timeSinceLastAstroidSpawn += GetFrameTime();
        if (timeSpentInvincible >= invincibleDuration)
        {
            playerIsInvincible = false;
            playerIsWhite = false;
            timeSpentInvincible = 0;
            invincibleColorSwitchCheck = 0;
        }
        // get player input
        if (IsKeyDown(KEY_A))
            player.angle -= player.rotation;
        if (IsKeyDown(KEY_D))
            player.angle += player.rotation;
        if (IsKeyDown(KEY_W))
        {
            player.velocity.x = player.speed * cos(player.angle);
            player.velocity.y = player.speed * sin(player.angle);
        }
        else
        {
            player.velocity.x *= drag;
            player.velocity.y *= drag;
        }
        if (IsKeyDown(KEY_S))
        {
            player.velocity.x *= brakeDrag;
            player.velocity.y *= brakeDrag;
        }

        // player movement
        player.center.x += player.velocity.x;
        player.center.y += player.velocity.y;

        // draw triangle around player center
        Vector2 triangleA = {player.center.x + 2 * player.size / 3 * cos(player.angle + 2 * PI - ((4 * PI) / 6)),
                             player.center.y + 2 * player.size / 3 * sin(player.angle + 2 * PI - ((4 * PI) / 6))};

        Vector2 triangleB = {player.center.x + 2 * player.size / 3 * cos(player.angle + (4 * PI) / 6),
                             player.center.y + 2 * player.size / 3 * sin(player.angle + (4 * PI) / 6)};

        Vector2 triangleC = {player.center.x + player.size * cos(player.angle),
                             player.center.y + player.size * sin(player.angle)};

        if (IsKeyDown(KEY_SPACE))
        {
            if (timeSinceLastShot > fireCooldown)
            {
                bullet[bulletPointer].angle = player.angle;
                bullet[bulletPointer].velocity.x = bullet[bulletPointer].speed * cos(player.angle);
                bullet[bulletPointer].velocity.y = bullet[bulletPointer].speed * sin(player.angle);
                bullet[bulletPointer].center = triangleC;
                bullet[bulletPointer].hp = 1;
                if (bulletPointer >= maxBullets - 1)
                    bulletPointer = 0;
                else
                    bulletPointer++;
                timeSinceLastShot = 0;
            }
        }

        if (timeSinceLastAstroidSpawn > timeBetweenAsteroidSpawn)
        {
            float size = (rand() % (asteroidMaxSize - asteroidMinSize + 1)) + asteroidMinSize; // picks a random value between the min and max asteroid size
            timeSinceLastAstroidSpawn = 0;
            Vector2 center = {0, 0};
            float angle = 0;
            GetRandomAsteroidSpawn(&center, &angle, size);
            SpawnAsteroid(&asteroid[asteroidPointer], center, size, angle, &asteroidPointer);
            
        }

        int bulletsOnScreen[maxBullets];
        int bulletsOnScreenPointer = 0;
        int asteroidsOnScreen[maxAsteroids];
        int asteroidsOnScreenPointer = 0;
        BeginDrawing();
        ClearBackground(BLACK);

        // loop through all bullets
        for (int i = 0; i < maxBullets; i++)
        {
            if ((bullet[i].center.x >= -bullet[i].size && bullet[i].center.x <= screenWidth + bullet[i].size) &&
                (bullet[i].center.y >= -bullet[i].size && bullet[i].center.y <= screenHeight + bullet[i].size) &&
                bullet[i].hp > 0)
            {
                bulletsOnScreen[bulletsOnScreenPointer] = i;
                bulletsOnScreenPointer++;
                bullet[i].center.x += bullet[i].velocity.x;
                bullet[i].center.y += bullet[i].velocity.y;
                DrawCircleV(bullet[i].center, bullet[i].size, BLUE);
            }
        }
        for (int i = 0; i < maxAsteroids; i++)
        { // - 5 since starting pos will be -asteroid[i].size at times
            if ((asteroid[i].center.x >= -asteroid[i].size - 5 && asteroid[i].center.x <= screenWidth + asteroid[i].size + 5) &&
                (asteroid[i].center.y >= -asteroid[i].size - 5 && asteroid[i].center.y <= screenHeight + asteroid[i].size + 5) &&
                asteroid[i].hp > 0)
            {
                asteroidsOnScreen[asteroidsOnScreenPointer] = i;
                asteroidsOnScreenPointer++;
                asteroid[i].center.x += asteroid[i].velocity.x;
                asteroid[i].center.y += asteroid[i].velocity.y;
                DrawCircleV(asteroid[i].center, asteroid[i].size, BROWN);
                DrawText(TextFormat("%i-%.2f", i, asteroid[i].size), asteroid[i].center.x, asteroid[i].center.y, 10, GREEN);
                if ((CheckCollisionPointCircle(triangleA, asteroid[i].center, asteroid[i].size) ||
                     CheckCollisionPointCircle(triangleB, asteroid[i].center, asteroid[i].size) ||
                     CheckCollisionPointCircle(triangleC, asteroid[i].center, asteroid[i].size)) &&
                    !playerIsInvincible)
                {
                    player.hp--;
                    playerIsInvincible = true;
                    playerIsWhite = true;
                }
            }
        }

        for (int i = 0; i < bulletsOnScreenPointer; i++)
        {
            for (int j = 0; j < asteroidsOnScreenPointer; j++)
            {

                if (CheckCollisionCircles(bullet[bulletsOnScreen[i]].center, bullet[bulletsOnScreen[i]].size,
                                          asteroid[asteroidsOnScreen[j]].center, asteroid[asteroidsOnScreen[j]].size))
                {
                    bullet[bulletsOnScreen[i]].hp = 0;
                    asteroid[asteroidsOnScreen[j]].hp--;
                    if (asteroid[asteroidsOnScreen[j]].hp <= 0 && asteroid[asteroidsOnScreen[j]].size >= asteroidMinSize * 2) // add 2 new asteroids from the old asteroid
                    {
                        score += 10;
                        float newSpeed = asteroid[asteroidsOnScreen[j]].speed * 2;
                        float newSize = asteroid[asteroidsOnScreen[j]].size / 2;
                        float newHP = newSize / 5;
                        asteroid[asteroidPointer] = (entity){
                            .center = {asteroid[asteroidsOnScreen[j]].center.x + (asteroid[asteroidsOnScreen[j]].size * cos(bullet[bulletsOnScreen[i]].angle + PI / 2) / 2),
                                       asteroid[asteroidsOnScreen[j]].center.y + (asteroid[asteroidsOnScreen[j]].size * sin(bullet[bulletsOnScreen[i]].angle + PI / 2) / 2)},
                            .velocity = {newSpeed * cos(bullet[bulletsOnScreen[i]].angle + PI / 2),
                                         newSpeed * sin(bullet[bulletsOnScreen[i]].angle + PI / 2)},
                            .angle = bullet[bulletsOnScreen[i]].angle + PI / 2,
                            .speed = newSpeed,
                            .size = newSize,
                            .hp = newHP};
                        asteroidPointer >= maxAsteroids - 1 ? 0 : asteroidPointer++;
                        asteroid[asteroidPointer] = (entity){
                            .center = {asteroid[asteroidsOnScreen[j]].center.x + (asteroid[asteroidsOnScreen[j]].size * cos(bullet[bulletsOnScreen[i]].angle + (3 * PI / 2)) / 2),
                                       asteroid[asteroidsOnScreen[j]].center.y + (asteroid[asteroidsOnScreen[j]].size * sin(bullet[bulletsOnScreen[i]].angle + (3 * PI / 2)) / 2)},
                            .velocity = {newSpeed * cos(bullet[bulletsOnScreen[i]].angle + (3 * PI / 2)),
                                         newSpeed * sin(bullet[bulletsOnScreen[i]].angle + (3 * PI / 2))},
                            .angle = bullet[bulletsOnScreen[i]].angle + (3 * PI / 2),
                            .speed = newSpeed,
                            .size = newSize,
                            .hp = newHP};
                        asteroidPointer >= maxAsteroids - 1 ? 0 : asteroidPointer++;
                    }
                    else if (asteroid[asteroidsOnScreen[j]].hp <= 0)
                    {
                        score += 10;
                    }
                }
            }
        }
        DrawText(TextFormat("bullets on screen:   %i\n"
                            "bullet pointer:      %i\n"
                            "asteroids on screen: %i\n"
                            "asteroid pointer:    %i\n"
                            "asteroid 0 x.y :     %.2f.%.2f",
                            bulletsOnScreenPointer, bulletPointer, asteroidsOnScreenPointer, asteroidPointer, asteroid[0].center.x, asteroid[0].center.y),
                 10, 10, 25, GREEN);
        // DrawText(TextFormat("lives remaining: %.0f\n"
        //                     "score:            %i",
        //                     player.hp, score),
        //          10, 10, 25, GREEN);
        if (playerIsInvincible && invincibleColorSwitchCheck >= 60)
        {
            playerIsWhite = !playerIsWhite;
            invincibleColorSwitchCheck -= 60;
        }
        if (playerIsWhite)
        {
            DrawTriangle(triangleA, triangleB, triangleC, WHITE);
        }
        else
        {
            DrawTriangle(triangleA, triangleB, triangleC, RED);
        }
        DrawCircleV(triangleC, 3, GREEN);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}