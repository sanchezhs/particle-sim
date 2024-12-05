#include <assert.h>
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define MAX_PARTICLES 10
#define INITIAL_CAPACITY 3
#define MIN_LIFETIME 100
#define MAX_LIFETIME 500
#define MIX_EXPLOSION_PARTICLES 0
#define MAX_EXPLOSION_PARTICLES 5
#define MIN_PARTICLE_SPEED -100
#define MAX_PARTICLE_SPEED 100
#define MIN_VIRTUAL_PARTICLE_SPEED (1.0f / 2.0f * MIN_PARTICLE_SPEED)
#define MAX_VIRTUAL_PARTICLE_SPEED (1.0f / 2.0f * MAX_PARTICLE_SPEED)
#define MIN_VIRTUAL_PARTICLE_LIFETIME (1.0f / 2.0f * MIN_LIFETIME)
#define MAX_VIRTUAL_PARTICLE_LIFETIME (1.0f / 2.0f * MAX_LIFETIME)
#define MIN_MASS 100
#define MAX_MASS 200
#define TRAIL_LENGTH 10
#define WIDTH 800
#define HEIGHT 800
#define G 9.81f
#define K_ELECTRIC 250.0f
#define G_UNIVERSAL 6.67e11
#define MAX_GRAVITY_DISTANCE 500.0f
#define MAX_FORCE 10.0f
#define CELL_SIZE 50
#define GRID_WIDTH ((WIDTH + CELL_SIZE - 1) / CELL_SIZE)
#define GRID_HEIGHT ((HEIGHT + CELL_SIZE - 1) / CELL_SIZE)
#define MIN_TIME_BETWEEN_VIRTUAL_PAIRS 2.5f // Minimum time in seconds between pairs

#define ARRAY_APPEND_PTR(array, item)                                                              \
    do                                                                                             \
    {                                                                                              \
        if ((array)->count >= (array)->capacity)                                                   \
        {                                                                                          \
            (array)->capacity = (array)->capacity == 0 ? INITIAL_CAPACITY : (array)->capacity * 2; \
            (array)->items = realloc((array)->items, (array)->capacity * sizeof(*(array)->items)); \
            assert((array)->items != NULL && "Buy more RAM lol");                                  \
        }                                                                                          \
        (array)->items[(array)->count++] = (item);                                                 \
    } while (0)

#define ARRAY_APPEND(array, item)                                                                  \
    do                                                                                             \
    {                                                                                              \
        if ((array)->count >= (array)->capacity)                                                   \
        {                                                                                          \
            (array)->capacity = (array)->capacity == 0 ? INITIAL_CAPACITY : (array)->capacity * 2; \
            (array)->items = realloc((array)->items, (array)->capacity * sizeof(*(array)->items)); \
            assert((array)->items != NULL && "Buy more RAM lol");                                  \
        }                                                                                          \
                                                                                                   \
        (array)->items[(array)->count++] = (item);                                                 \
    } while (0)

int globalParticleID = 0;       // Global particle ID counter
float timeSinceLastPair = 0.0f; // Global timer for particle generation

typedef struct Particle
{
    int id;
    Vector2 position;
    Vector2 velocity;
    Color color;
    float lifetime;
    float size;
    float mass;
    float charge;
    Vector2 trail[TRAIL_LENGTH];
    int trailIndex;
    bool isFragment;
    bool isVirtual;
} Particle;

typedef struct Particles
{
    Particle *items;
    int count;
    int capacity;
} Particles;

typedef struct Cell
{
    Particle **items;
    int count;
    int capacity;
} Cell;

Cell grid[GRID_WIDTH][GRID_HEIGHT];

typedef enum GravityType
{
    GRAVITY_CENTER,
    GRAVITY_DOWN,
    GRAVITY_UP,
    GRAVITY_LEFT,
    GRAVITY_RIGHT,
    GRAVITY_NONE
} GravityType;

void InitSimulation(Particles *particles, Shader *glowShader, RenderTexture2D *target);
void UpdateSimulation(Particles *particles, RenderTexture2D *target, bool *lifetime, GravityType gravityType, bool *electricForce, bool *fragmentParticlesLive);
void RenderSimulation(Particles *particles, Shader glowShader, RenderTexture2D target);
void HandleInput(Particles *particles);
void CleanupSimulation(Particles *particles, Shader glowShader, RenderTexture2D target);
void InitParticles(Particles *particles);
void InitGrid();
void AssignParticlesToCells(Particles *particles);
void ResolveCollition(Particle *p1, Particle *p2);
bool Collide(Particle *p1, Particle *p2);
// Helpers
float GetRandomFloat(float min, float max)
{
    return min + (max - min) * GetRandomValue(0, 10000) / 10000.0f;
}

void InitParticles(Particles *particles)
{
    for (int i = 0; i < INITIAL_CAPACITY; i++)
    {
        float mass = GetRandomValue(MIN_MASS, MAX_MASS);
        float charge = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;

        Particle p = (Particle){
            .id = globalParticleID++,
            .mass = mass,
            .size = cbrtf(mass),
            .charge = charge,
            .position = (Vector2){GetRandomValue(50, WIDTH - 50), GetRandomValue(50, HEIGHT - 50)},
            .velocity = (Vector2){GetRandomValue(MIN_PARTICLE_SPEED, MAX_PARTICLE_SPEED) / 50.0, GetRandomValue(MIN_PARTICLE_SPEED, MAX_PARTICLE_SPEED) / 50.0},
            .color = (charge > 0) ? RED : BLUE,
            .lifetime = GetRandomValue(MIN_LIFETIME, MAX_LIFETIME),
            .isFragment = false,
            .isVirtual = false,
        };
        for (int j = 0; j < TRAIL_LENGTH; j++)
        {
            p.trail[j] = p.position;
        }
        p.trailIndex = 0;
        ARRAY_APPEND(particles, p);
    }
}

void GenerateVirtualParticles(Particles *particles, float delta)
{
    timeSinceLastPair += delta;
    if (timeSinceLastPair < MIN_TIME_BETWEEN_VIRTUAL_PAIRS)
    {
        return;
    }

    float prob = GetRandomFloat(0, 1);

    if (prob > 0.01)
    {
        return;
    }

    timeSinceLastPair = 0.0f;

    int lifetime = GetRandomValue(MIN_VIRTUAL_PARTICLE_LIFETIME, MAX_VIRTUAL_PARTICLE_LIFETIME);
    int randx = GetRandomValue(0, WIDTH);
    int randy = GetRandomValue(0, HEIGHT);
    Vector2 pos1 = (Vector2){randx, randy};
    Vector2 pos2 = (Vector2){randx + GetRandomValue(-1, 1), randy + GetRandomValue(-1, 1)};
    float mass = GetRandomValue(1, 10);

    // Create positive particle
    Particle positiveParticle = {
        .id = globalParticleID++,
        .mass = mass,
        .size = cbrtf(mass),
        .charge = 0.1f,
        .position = pos1,
        .velocity = (Vector2){GetRandomValue(MIN_VIRTUAL_PARTICLE_SPEED, MAX_VIRTUAL_PARTICLE_SPEED) / 100.0f, GetRandomValue(MIN_VIRTUAL_PARTICLE_SPEED, MAX_VIRTUAL_PARTICLE_SPEED) / 100.0f},
        .color = LIGHTGRAY,
        .lifetime = lifetime,
        .isFragment = false,
        .isVirtual = true,
    };

    // Create negative particle
    Particle negativeParticle = {
        .id = globalParticleID++,
        .mass = mass,
        .size = cbrtf(mass),
        .charge = -0.1f,
        .position = pos2,
        .velocity = (Vector2){-positiveParticle.velocity.x, -positiveParticle.velocity.y},
        .color = DARKGRAY,
        .lifetime = lifetime,
        .isFragment = false,
        .isVirtual = true,
    };

    for (int j = 0; j < TRAIL_LENGTH; j++)
    {
        positiveParticle.trail[j] = positiveParticle.position;
        negativeParticle.trail[j] = negativeParticle.position;
    }
    positiveParticle.trailIndex = 0;
    negativeParticle.trailIndex = 0;

    ARRAY_APPEND(particles, positiveParticle);
    ARRAY_APPEND(particles, negativeParticle);
}

bool Collide(Particle *p1, Particle *p2)
{
    float dx = p2->position.x - p1->position.x;
    float dy = p2->position.y - p1->position.y;
    float distance = sqrt(dx * dx + dy * dy);
    return distance <= (p1->size + p2->size);
}

void ResolveCollition(Particle *p1, Particle *p2)
{
    Vector2 delta = {p2->position.x - p1->position.x, p2->position.y - p1->position.y};
    float distance = sqrt(delta.x * delta.x + delta.y * delta.y);

    if (distance == 0.0f)
        return;

    Vector2 normal = {delta.x / distance, delta.y / distance};

    Vector2 relative_velocity = {p1->velocity.x - p2->velocity.x, p1->velocity.y - p2->velocity.y};
    float speed = (relative_velocity.x * normal.x + relative_velocity.y * normal.y);

    if (speed > 0)
        return;

    float overlap = (p1->size + p2->size) - distance;
    p1->position.x -= normal.x * overlap / 2;
    p1->position.y -= normal.y * overlap / 2;
    p2->position.x += normal.x * overlap / 2;
    p2->position.y += normal.y * overlap / 2;

    float impulse = 2 * speed / 2; // Mass = 1 for both particles
    p1->velocity.x -= impulse * normal.x;
    p1->velocity.y -= impulse * normal.y;
    p2->velocity.x += impulse * normal.x;
    p2->velocity.y += impulse * normal.y;
}

void InitGrid()
{
    for (int x = 0; x < GRID_WIDTH; x++)
    {
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            grid[x][y].items = (Particle **)malloc(INITIAL_CAPACITY * sizeof(Particle *));
            grid[x][y].count = 0;
            grid[x][y].capacity = INITIAL_CAPACITY;
        }
    }
}

void AssignParticlesToCells(Particles *particles)
{
    for (int x = 0; x < GRID_WIDTH; x++)
    {
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            grid[x][y].count = 0;
        }
    }

    for (int i = 0; i < particles->count; i++)
    {
        Particle *p = &particles->items[i];
        int cellX = p->position.x / CELL_SIZE;
        int cellY = p->position.y / CELL_SIZE;

        if (cellX >= 0 && cellX < GRID_WIDTH && cellY >= 0 && cellY < GRID_HEIGHT)
        {
            ARRAY_APPEND_PTR(&grid[cellX][cellY], p);
        }
    }
}

void Simulate(Particles *particles, bool lifetime, GravityType gravityType, bool electricForce, bool fragmentParticlesLive)
{
    Particles newParticles = {
        .items = (Particle *)malloc(INITIAL_CAPACITY * sizeof(Particle)),
        .count = 0,
        .capacity = INITIAL_CAPACITY,
    };

    int alive = 0;

    for (int i = 0; i < particles->count; i++)
    {
        Particle *p = &particles->items[i];

        // Update position
        p->position.x += p->velocity.x;
        p->position.y += p->velocity.y;

        // Update color based on speed
        float speed = sqrtf(p->velocity.x * p->velocity.x + p->velocity.y * p->velocity.y);
        p->color = ColorFromHSV(fmodf(speed * 10.0f, 360.0f), 1.0f, 1.0f);

        // Update size based on lifetime
        if (lifetime)
        {
            p->size = cbrtf(p->mass) * (p->lifetime / (float)MAX_LIFETIME);
            // Update lifetime
            p->lifetime--;
        }

        // Update trail
        p->trail[p->trailIndex] = p->position;
        p->trailIndex = (p->trailIndex + 1) % TRAIL_LENGTH;

        if (p->lifetime <= 0)
        {
            if (p->isVirtual)
                continue;
            if (!p->isFragment || fragmentParticlesLive)
            {
                Vector2 explosionCenter = p->position;
                int numExplosionParticles = GetRandomValue(MIX_EXPLOSION_PARTICLES, MAX_EXPLOSION_PARTICLES);

                for (int j = 0; j < numExplosionParticles; j++)
                {
                    float mass = GetRandomValue(MIN_MASS, MAX_MASS);
                    float charge = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;

                    Particle newParticle = {
                        .id = globalParticleID++,
                        .mass = mass,
                        .charge = charge,
                        .size = cbrtf(mass),
                        .position = explosionCenter,
                        .velocity = (Vector2){
                            GetRandomValue(-100, 100) / 20.0f,
                            GetRandomValue(-100, 100) / 20.0f},
                        .color = (Color){GetRandomValue(150, 255), GetRandomValue(150, 255), 0, 255},
                        .lifetime = GetRandomValue(MIN_LIFETIME, MAX_LIFETIME),
                        .isFragment = true,
                        .isVirtual = false,
                    };
                    for (int t = 0; t < TRAIL_LENGTH; t++)
                    {
                        newParticle.trail[t] = newParticle.position;
                    }
                    newParticle.trailIndex = 0;

                    ARRAY_APPEND(&newParticles, newParticle);
                }
            }
            continue;
        }
        particles->items[alive++] = particles->items[i];
    }

    particles->count = alive;

    if (particles->count + newParticles.count <= MAX_PARTICLES)
    {
        for (int i = 0; i < newParticles.count; i++)
        {
            ARRAY_APPEND(particles, newParticles.items[i]);
        }
    }
    else
    {
        int spaceLeft = MAX_PARTICLES - particles->count;
        for (int i = 0; i < spaceLeft; i++)
        {
            ARRAY_APPEND(particles, newParticles.items[i]);
        }
    }

    free(newParticles.items);

    AssignParticlesToCells(particles);

    // Process collisions and forces
    for (int i = 0; i < particles->count; i++)
    {
        Particle *p = &particles->items[i];

        // Screen limits control
        if (p->position.x <= 0 || p->position.x >= WIDTH)
            p->velocity.x *= -1;
        if (p->position.y <= 0 || p->position.y >= HEIGHT)
            p->velocity.y *= -1;

        // Gravity
        switch (gravityType)
        {
        case GRAVITY_NONE:
            break;
        case GRAVITY_CENTER:
        {
            Vector2 center = {WIDTH / 2.0f, HEIGHT / 2.0f};
            Vector2 dir = {center.x - p->position.x, center.y - p->position.y};
            float distance = sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (distance != 0)
            {
                dir.x /= distance;
                dir.y /= distance;
                p->velocity.x += dir.x * G * 0.01f;
                p->velocity.y += dir.y * G * 0.01f;
            }
        }
        break;
        case GRAVITY_DOWN:
            p->velocity.y += G * 0.01f;
            break;
        case GRAVITY_UP:
            p->velocity.y -= G * 0.01f;
            break;
        case GRAVITY_LEFT:
            p->velocity.x -= G * 0.01f;
            break;
        case GRAVITY_RIGHT:
            p->velocity.x += G * 0.01f;
            break;
        default:
            break;
        }

        // Computes the current cell of the particle
        int cellX = p->position.x / CELL_SIZE;
        int cellY = p->position.y / CELL_SIZE;

        // Iterate over the adjacent cells
        for (int offsetX = -1; offsetX <= 1; offsetX++)
        {
            for (int offsetY = -1; offsetY <= 1; offsetY++)
            {
                int neighborX = cellX + offsetX;
                int neighborY = cellY + offsetY;

                if (neighborX >= 0 && neighborX < GRID_WIDTH && neighborY >= 0 && neighborY < GRID_HEIGHT)
                {
                    Cell *cell = &grid[neighborX][neighborY];

                    for (int j = 0; j < cell->count; j++)
                    {
                        Particle *other = cell->items[j];

                        if (p == other)
                            continue;

                        if (p->id < other->id)
                        {
                            if (Collide(p, other))
                            {
                                ResolveCollition(p, other);
                            }

                            // Compute forces
                            Vector2 delta = {
                                other->position.x - p->position.x,
                                other->position.y - p->position.y};

                            float distance = sqrtf(delta.x * delta.x + delta.y * delta.y);

                            if (distance > 1.0f && distance < MAX_GRAVITY_DISTANCE)
                            {
                                delta.x /= distance;
                                delta.y /= distance;

                                float force = G_UNIVERSAL * (p->mass * other->mass) / (distance * distance);

                                if (force > MAX_FORCE)
                                {
                                    force = MAX_FORCE;
                                }

                                p->velocity.x += force / p->mass * delta.x;
                                p->velocity.y += force / p->mass * delta.y;

                                if (electricForce)
                                {
                                    float force_electric = K_ELECTRIC * (p->charge * other->charge) / (distance * distance);

                                    if (fabsf(force_electric) > MAX_FORCE)
                                    {
                                        force_electric = (force_electric > 0) ? MAX_FORCE : -MAX_FORCE;
                                    }
                                    p->velocity.x += force_electric / p->mass * delta.x;
                                    p->velocity.y += force_electric / p->mass * delta.y;
                                }
                            }
                        }
                    }
                }
            }
        }

        DrawCircleV(p->position, p->size, p->color);
        for (int t = 0; t < TRAIL_LENGTH - 1; t++)
        {
            int index = (p->trailIndex + t) % TRAIL_LENGTH;
            int nextIndex = (index + 1) % TRAIL_LENGTH;
            DrawLineEx(
                p->trail[index],
                p->trail[nextIndex],
                2.0f,
                Fade(p->color, (float)t / TRAIL_LENGTH));
        }
    }
}

void InitSimulation(Particles *particles, Shader *glowShader, RenderTexture2D *target)
{
    InitWindow(WIDTH, HEIGHT, "Particle System");
    SetTargetFPS(60);

    InitParticles(particles);
    InitGrid();

    *glowShader = LoadShader(0, "glow.fs");
    *target = LoadRenderTexture(WIDTH, HEIGHT);
}

void HandleInput(Particles *particles)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        float mass = GetRandomValue(MIN_MASS, MAX_MASS);
        float charge = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;

        Particle newParticle = {
            .id = globalParticleID++,
            .mass = mass,
            .size = cbrtf(mass),
            .charge = charge,
            .position = GetMousePosition(),
            .velocity = (Vector2){GetRandomValue(-50, 50) / 10.0f, GetRandomValue(-50, 50) / 10.0f},
            .color = (charge > 0) ? RED : BLUE,
            .lifetime = GetRandomValue(MIN_LIFETIME, MAX_LIFETIME),
            .isFragment = false,
        };
        for (int j = 0; j < TRAIL_LENGTH; j++)
        {
            newParticle.trail[j] = newParticle.position;
        }
        newParticle.trailIndex = 0;
        ARRAY_APPEND(particles, newParticle);
    }
}

void UpdateSimulation(Particles *particles, RenderTexture2D *target, bool *lifetime, GravityType gravityType, bool *electricForce, bool *fragmentParticlesLive)
{

    BeginTextureMode(*target);
    ClearBackground(BLACK);
    Simulate(particles, *lifetime, gravityType, *electricForce, *fragmentParticlesLive);
    EndTextureMode();
}

void RenderSimulation(Particles *particles, Shader glowShader, RenderTexture2D target)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginShaderMode(glowShader);
    DrawTextureRec(target.texture, (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height}, (Vector2){0, 0}, WHITE);
    EndShaderMode();

    EndDrawing();
}

void CleanupSimulation(Particles *particles, Shader glowShader, RenderTexture2D target)
{
    UnloadShader(glowShader);
    UnloadRenderTexture(target);

    for (int x = 0; x < GRID_WIDTH; x++)
    {
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            free(grid[x][y].items);
        }
    }
    free(particles->items);

    CloseWindow();
}

int main(void)
{
    SetRandomSeed(GetRandomValue(0, 10000));

    GravityType gravityType = GRAVITY_NONE;
    bool lifetime = false;
    bool electricForce = false;
    bool fragmentParticlesLive = false;
    bool virtualParticles = false;

    Particles particles = {
        .items = (Particle *)malloc(MAX_PARTICLES * sizeof(Particle)),
        .count = 0,
        .capacity = INITIAL_CAPACITY,
    };
    Shader glowShader;
    RenderTexture2D target;

    InitSimulation(&particles, &glowShader, &target);

    while (!WindowShouldClose())
    {
        float delta = GetFrameTime();
        HandleInput(&particles);
        if (virtualParticles)
        {
            GenerateVirtualParticles(&particles, delta);
        }
        UpdateSimulation(&particles, &target, &lifetime, gravityType, &electricForce, &fragmentParticlesLive);
        RenderSimulation(&particles, glowShader, target);
    }

    CleanupSimulation(&particles, glowShader, target);

    return 0;
}