#include "raylib.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>

// comment out developer mode to hide developer overlay
#define DEVELOPER_MODE

const int screenWidth = 1920;
const int screenHeight = 1080;
const float drag = 0.99f;     // default drag when not pressing W
const float brakeDrag = 0.9f; // drag when pressing S
const int targetFPS = 60;
const int maxBullets = 300;             // amount of bullets shot before they are recycled
const float defaultFireCooldown = 0.1f; // time between shots in seconds
const int maxAsteroids = 2;             // amount of asteroids before they are recycled
const float timeBetweenAsteroidSpawn = 10;
const float asteroidSpeedConstant = 200; // min speed will be this/max size, max speed will be this/min size
const int asteroidMaxSize = 100;
const int asteroidMinSize = 30;
const float invincibleDuration = 2; // invincibility time after taking damage in seconds

typedef enum game_state_e
{
    gameStatePlaying,
    gameStateDead
} game_state_e;

game_state_e state = gameStatePlaying;

typedef struct entity_t
{
    Vector2 center;
    Vector2 velocity;
    double angle;
    float rotation;
    float speed;
    float size;
    float hp;
} entity_t;

typedef struct player_data_t
{
    entity_t *player;
    bool *playerIsInvincible;
    bool *playerIsWhite;
    float *timeSpentInvincible;
    float *invincibleColorSwitchCheck;
    int *score;
    Vector2 *triangleA;
    Vector2 *triangleB;
    Vector2 *triangleC;
} player_data_t;

typedef struct bullet_data_t
{
    entity_t *bullet;
    int *bulletPointer;
    float *timeSinceLastShot;
    float *fireCooldown;
    int *bulletsOnScreen;
    int *bulletsOnScreenPointer;
} bullet_data_t;

typedef struct asteroid_data_t
{
    entity_t *asteroid;
    int *asteroidPointer;
    float *timeSinceLastAsteroidSpawn;
    int *asteroidsOnScreen;
    int *asteroidsOnScreenPointer;
} asteroid_data_t;
// TODO figure out where to put this
/*void CheckForPlayerAsteroidCollision() {
    if ((CheckCollisionPointCircle(triangleA, asteroid.center, asteroid.size) ||
                     CheckCollisionPointCircle(triangleB, asteroid.center, asteroid.size) ||
                     CheckCollisionPointCircle(triangleC, asteroid.center, asteroid.size)) &&
                    !playerIsInvincible)
                {
                    player.hp--;
                    playerIsInvincible = true;
                    playerIsWhite = true;
                }
}*/

void RenderAsteroids(asteroid_data_t asteroidData) {
    for (int i = 0; i < maxAsteroids; i++)
        { // - 5 since starting pos will be -asteroid[i].size at times
            entity_t asteroid = asteroidData.asteroid[i];
            if ((asteroid.center.x >= -asteroid.size - 5 && asteroid.center.x <= screenWidth + asteroid.size + 5) &&
                (asteroid.center.y >= -asteroid.size - 5 && asteroid.center.y <= screenHeight + asteroid.size + 5) &&
                asteroid.hp > 0)
            {
                asteroidData.asteroidsOnScreen[*asteroidData.asteroidsOnScreenPointer] = i;
                *asteroidData.asteroidsOnScreenPointer++;
                asteroid.center.x += asteroid.velocity.x;
                asteroid.center.y += asteroid.velocity.y;
                DrawCircleV(asteroid.center, asteroid.size, BROWN);
                DrawText(TextFormat("%i-%.2f", i, asteroid.size), asteroid.center.x, asteroid.center.y, 10, GREEN);
            }
        }
}

void RenderBullets(bullet_data_t bulletData) {
    for (int i = 0; i < maxBullets; i++)
        {
            entity_t bullet = bulletData.bullet[i];

            if ((bullet.center.x >= -bullet.size && bullet.center.x <= screenWidth + bullet.size) &&
                (bullet.center.y >= -bullet.size && bullet.center.y <= screenHeight + bullet.size) &&
                bullet.hp > 0)
            {
                bulletData.bulletsOnScreen[*bulletData.bulletsOnScreenPointer] = i;
                *bulletData.bulletsOnScreenPointer++;
                bullet.center.x += bullet.velocity.x;
                bullet.center.y += bullet.velocity.y;
                DrawCircleV(bullet.center, bullet.size, BLUE);
            }
        }
}

void Render(player_data_t playerData, bullet_data_t bulletData, asteroid_data_t asteroidData)
{
    switch (state)
    {
    case gameStatePlaying:
        RenderBullets(bulletData);
        break;
    case gameStateDead:
        break;
    }
}
void SpawnAsteroid(entity_t *asteroid, int *asteroidPointer)
{
    float hp = asteroid->size / 5;
    float speed = asteroidSpeedConstant / asteroid->size;
    Vector2 velocity = {speed * cos(asteroid->angle),
                        speed * sin(asteroid->angle)};
    *asteroid = (entity_t){
        .velocity = velocity,
        .speed = speed,
        .hp = hp};
    *asteroidPointer = *asteroidPointer >= (maxAsteroids - 1) ? 0 : *asteroidPointer + 1;
}
void SpawnAsteroid(entity_t *asteroid, Vector2 center, float size, float angle, int *asteroidPointer)
{
    float hp = size / 5;
    float speed = asteroidSpeedConstant / size;
    Vector2 velocity = {speed * cos(angle),
                        speed * sin(angle)};
    *asteroid = (entity_t){
        .center = center,
        .velocity = velocity,
        .angle = angle,
        .speed = speed,
        .size = size,
        .hp = hp};
    *asteroidPointer = *asteroidPointer >= (maxAsteroids - 1) ? 0 : *asteroidPointer + 1;
}

void GetRandomAsteroidSpawn(entity_t *asteroid)
{
    float size = (rand() % (asteroidMaxSize - asteroidMinSize + 1)) + asteroidMinSize; // picks a random value between the min and max asteroid size
    asteroid->size = size;
    switch (rand() % 4)
    {
    case 0: // start in left corner
        asteroid->center.x = -asteroid->size;
        asteroid->center.y = rand() % screenHeight;
        asteroid->angle = (rand() % (int)(100 * PI) / 100) - PI / 2; // (-90)-90 degrees
        break;

    case 1: // start in right corner
        asteroid->center.x = screenWidth + size;
        asteroid->center.y = rand() % screenHeight;
        asteroid->angle = (rand() % (int)(100 * PI) / 100) + PI / 2; // 90-270 degrees
        break;

    case 2: // start in bottom
        asteroid->center.x = rand() % screenWidth;
        asteroid->center.y = -size;
        asteroid->angle = (rand() % (int)(100 * PI) / 100); // 0-180 degrees
        break;

    case 3: // start in top
        asteroid->center.x = rand() % screenWidth;
        asteroid->center.y = screenHeight + size;
        asteroid->angle = (rand() % (int)(100 * PI) / 100) + PI; // 180-360 degrees
        break;
    }
}
// checks if astroid should spawn, and spawns one if it should
void HandleAstroidSpawn(asteroid_data_t asteroidData)
{
    if (*asteroidData.timeSinceLastAsteroidSpawn >= *asteroidData.timeSinceLastAsteroidSpawn)
    {
        GetRandomAsteroidSpawn(&asteroidData.asteroid[*asteroidData.asteroidPointer]);
        SpawnAsteroid(&asteroidData.asteroid[*asteroidData.asteroidPointer], asteroidData.asteroidPointer);
    }
}


void UpdatePlayerPosition(entity_t *player)
{
    player->center.x += player->velocity.x;
    player->center.y += player->velocity.y;
}

void SpawnBullet(entity_t *bullet, int *bulletPointer, double playerAngle, Vector2 playerFront)
{
    bullet[*bulletPointer].angle = playerAngle;
    bullet[*bulletPointer].velocity.x = bullet[*bulletPointer].speed * cos(playerAngle);
    bullet[*bulletPointer].velocity.y = bullet[*bulletPointer].speed * sin(playerAngle);
    bullet[*bulletPointer].center = playerFront;
    bullet[*bulletPointer].hp = 1;
}

void CalculatePlayerPosition(player_data_t playerData)
{
    entity_t player = *playerData.player; // copied for simplicity in code
    *playerData.triangleA = (Vector2){player.center.x + 2 * player.size / 3 * cos(player.angle + 2 * PI - ((4 * PI) / 6)),
                                      player.center.y + 2 * player.size / 3 * sin(player.angle + 2 * PI - ((4 * PI) / 6))};

    *playerData.triangleB = (Vector2){player.center.x + 2 * player.size / 3 * cos(player.angle + (4 * PI) / 6),
                                      player.center.y + 2 * player.size / 3 * sin(player.angle + (4 * PI) / 6)};

    *playerData.triangleC = (Vector2){player.center.x + player.size * cos(player.angle),
                                      player.center.y + player.size * sin(player.angle)};
}

void HandlePlayerInput(entity_t *player, bullet_data_t bulletData, Vector2 playerFront)
{
    if (IsKeyDown(KEY_A))
        player->angle -= player->rotation;
    if (IsKeyDown(KEY_D))
        player->angle += player->rotation;
    if (IsKeyDown(KEY_W))
    {
        player->velocity.x = player->speed * cos(player->angle);
        player->velocity.y = player->speed * sin(player->angle);
    }
    else
    {
        player->velocity.x *= drag;
        player->velocity.y *= drag;
    }
    if (IsKeyDown(KEY_S))
    {
        player->velocity.x *= brakeDrag;
        player->velocity.y *= brakeDrag;
    }
    if (IsKeyDown(KEY_SPACE))
    {
        if (*bulletData.timeSinceLastShot >= *bulletData.fireCooldown)
        {
            SpawnBullet(bulletData.bullet, bulletData.bulletPointer, player->angle, playerFront);
        }
    }
}

void HandleInvincibility(player_data_t playerData)
{
    if (*playerData.playerIsInvincible)
    {
        *playerData.timeSpentInvincible += GetFrameTime();
        // this will make the ship switch between white and red faster and faster
        *playerData.invincibleColorSwitchCheck += powf(20, (*playerData.timeSpentInvincible / invincibleDuration));
    }
    // if invincible duration is over
    if (*playerData.timeSpentInvincible >= invincibleDuration)
    {
        *playerData.playerIsInvincible = false;
        *playerData.playerIsWhite = false;
        *playerData.timeSpentInvincible = 0;
        *playerData.invincibleColorSwitchCheck = 0;
    }
}

void UpdateTimeVariables(float *timeSinceLastShot, float *timeSinceLastAstroidSpawn)
{
    *timeSinceLastShot += GetFrameTime();
    *timeSinceLastAstroidSpawn += GetFrameTime();
}

void Update(player_data_t playerData, bullet_data_t bulletData, asteroid_data_t asteroidData)
{
    switch (state)
    {
    case gameStatePlaying:
        UpdateTimeVariables(bulletData.timeSinceLastShot, asteroidData.timeSinceLastAsteroidSpawn);
        HandleInvincibility(playerData);
        HandlePlayerInput(playerData.player, bulletData, *playerData.triangleC);
        UpdatePlayerPosition(playerData.player);
        HandleAstroidSpawn(asteroidData);
        break;
    case gameStateDead:
        break;
    }
}

void InitGame(entity_t *player, entity_t *bullet, entity_t *asteroid, int *score)
{
    // init player
    *player = (entity_t){.center = (Vector2){screenWidth / 2, screenHeight / 2},
                         .velocity = (Vector2){0, 0},
                         .angle = 0,
                         .rotation = 0.1f,
                         .speed = 5.0f,
                         .size = 50.0f,
                         .hp = 5};

    // init bullets
    for (int i = 0; i < maxBullets; i++)
    {
        bullet[i] = (entity_t){
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
        asteroid[i] = (entity_t){
            .center = {-25, -25},
            .velocity = {0, 0},
            .angle = 0,
            .rotation = 0,
            .speed = 3,
            .size = 25,
            .hp = 10};
    }

    *score = 0;
    state = gameStatePlaying;
}
int main()
{

    srand(time(NULL));
    InitWindow(screenWidth, screenHeight, "asteroids");
    SetTargetFPS(targetFPS);
    entity_t player;

    bool playerIsInvincible = false;
    bool playerIsWhite = false;
    float timeSpentInvincible = 0;
    float invincibleColorSwitchCheck = 0; // will switch from white to red ship based on how long you have been invincible
    int score = 0;

    Vector2 triangleA = {player.center.x + 2 * player.size / 3 * cos(player.angle + 2 * PI - ((4 * PI) / 6)),
                         player.center.y + 2 * player.size / 3 * sin(player.angle + 2 * PI - ((4 * PI) / 6))};

    Vector2 triangleB = {player.center.x + 2 * player.size / 3 * cos(player.angle + (4 * PI) / 6),
                         player.center.y + 2 * player.size / 3 * sin(player.angle + (4 * PI) / 6)};

    Vector2 triangleC = {player.center.x + player.size * cos(player.angle),
                         player.center.y + player.size * sin(player.angle)};

    // values are assigned their own structs for better readability and less parameters for some functions
    player_data_t playerData = {
        .player = &player,
        .playerIsInvincible = &playerIsInvincible,
        .playerIsWhite = &playerIsWhite,
        .timeSpentInvincible = &timeSpentInvincible,
        .invincibleColorSwitchCheck = &invincibleColorSwitchCheck,
        .score = &score,
        .triangleA = &triangleA,
        .triangleB = &triangleB,
        .triangleC = &triangleC};

    entity_t bullet[maxBullets];
    int bulletPointer = 0;
    float timeSinceLastShot = 0;
    float fireCooldown = defaultFireCooldown;
    int bulletsOnScreen[maxBullets];
    int bulletsOnScreenPointer = 0;

    bullet_data_t bulletData = {
        .bullet = bullet,
        .bulletPointer = &bulletPointer,
        .timeSinceLastShot = &timeSinceLastShot,
        .fireCooldown = &fireCooldown,
        .bulletsOnScreen = bulletsOnScreen,
        .bulletsOnScreenPointer = &bulletsOnScreenPointer};

    entity_t asteroid[maxAsteroids];
    int asteroidPointer = 0;
    float timeSinceLastAstroidSpawn = 0;
    int asteroidsOnScreen[maxAsteroids];
    int asteroidsOnScreenPointer = 0;

    asteroid_data_t asteroidData = {
        .asteroid = asteroid,
        .asteroidPointer = &asteroidPointer,
        .timeSinceLastAsteroidSpawn = &timeSinceLastAstroidSpawn,
        .asteroidsOnScreen = asteroidsOnScreen,
        .asteroidsOnScreenPointer = &asteroidsOnScreenPointer};

    InitGame(playerData.player, bulletData.bullet, asteroidData.asteroid, playerData.score);

    while (!WindowShouldClose())
    {

        Update(playerData, bulletData, asteroidData);
        Render(playerData, bulletData, asteroidData);

        // RENDER ////////////////////////////////////////////////////////////////////
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
                        asteroid[asteroidPointer] = (entity_t){
                            .center = {asteroid[asteroidsOnScreen[j]].center.x + (asteroid[asteroidsOnScreen[j]].size * cos(bullet[bulletsOnScreen[i]].angle + PI / 2) / 2),
                                       asteroid[asteroidsOnScreen[j]].center.y + (asteroid[asteroidsOnScreen[j]].size * sin(bullet[bulletsOnScreen[i]].angle + PI / 2) / 2)},
                            .velocity = {newSpeed * cos(bullet[bulletsOnScreen[i]].angle + PI / 2),
                                         newSpeed * sin(bullet[bulletsOnScreen[i]].angle + PI / 2)},
                            .angle = bullet[bulletsOnScreen[i]].angle + PI / 2,
                            .speed = newSpeed,
                            .size = newSize,
                            .hp = newHP};
                        asteroidPointer = asteroidPointer >= maxAsteroids - 1 ? 0 : asteroidPointer + 1;
                        asteroid[asteroidPointer] = (entity_t){
                            .center = {asteroid[asteroidsOnScreen[j]].center.x + (asteroid[asteroidsOnScreen[j]].size * cos(bullet[bulletsOnScreen[i]].angle + (3 * PI / 2)) / 2),
                                       asteroid[asteroidsOnScreen[j]].center.y + (asteroid[asteroidsOnScreen[j]].size * sin(bullet[bulletsOnScreen[i]].angle + (3 * PI / 2)) / 2)},
                            .velocity = {newSpeed * cos(bullet[bulletsOnScreen[i]].angle + (3 * PI / 2)),
                                         newSpeed * sin(bullet[bulletsOnScreen[i]].angle + (3 * PI / 2))},
                            .angle = bullet[bulletsOnScreen[i]].angle + (3 * PI / 2),
                            .speed = newSpeed,
                            .size = newSize,
                            .hp = newHP};
                        asteroidPointer = asteroidPointer >= maxAsteroids - 1 ? 0 : asteroidPointer + 1;
                    }
                    else if (asteroid[asteroidsOnScreen[j]].hp <= 0)
                    {
                        score += 10;
                    }
                }
            }
        }
#ifdef DEVELOPER_MODE
        DrawText(TextFormat("bullets on screen:   %i\n"
                            "bullet pointer:      %i\n"
                            "asteroids on screen: %i\n"
                            "asteroid pointer:    %i\n"
                            "asteroid 0 x.y :     %.2f.%.2f",
                            bulletsOnScreenPointer, bulletPointer, asteroidsOnScreenPointer, asteroidPointer, asteroid[0].center.x, asteroid[0].center.y),
                 10, 10, 25, GREEN);
#endif
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
        DrawCircleV(player.center, 3, GREEN);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}