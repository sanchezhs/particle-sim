#ifndef SIMULATION_H
#define SIMULATION_H

#include <stdbool.h>
#ifdef __cplusplus
extern "C"
{
#endif

#include "raylib.h"

#ifdef __cplusplus
}
#endif

// TODO: Move these to a settings file
#define TRAIL_LENGTH 10

#define INITIAL_CAPACITY 5
#define WIDTH 800
#define HEIGHT 800
#define CELL_SIZE 50
#define GRID_WIDTH ((WIDTH + CELL_SIZE - 1) / CELL_SIZE)
#define GRID_HEIGHT ((HEIGHT + CELL_SIZE - 1) / CELL_SIZE)

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

// Enumeration for gravity types
typedef enum
{
    GRAVITY_NONE,
    GRAVITY_CENTER,
    GRAVITY_DOWN,
    GRAVITY_UP,
    GRAVITY_LEFT,
    GRAVITY_RIGHT
} GravityType;

// Structure to store all simulation parameters
typedef struct
{
    // General
    int maxParticles;           // Maximum number of particles
    int initialCapacity;        // Initial capacity of the particle system
    bool lifetime;              // Enable/disable particle lifetime
    bool fragmentParticlesLive; // Fragments particles have their own lifetime
    bool virtualParticles;      // Enable virtual particles

    // Particles
    float minParticleLifeTime; // Minimum particle lifetime
    float maxParticleLifeTime; // Maximum particle lifetime
    float minParticleSpeed;    // Minimum particle speed
    float maxParticleSpeed;    // Maximum particle speed
    float minParticleMass;     // Minimum particle mass
    float maxParticleMass;     // Maximum particle mass
    float trailLength;         // Length of particle trails

    // Explosions
    int minExplosionParticles; // Minimum number of particles in an explosion
    int maxExplosionParticles; // Maximum number of particles in an explosion

    // Virtual Particles
    int minVirtualParticleSpeed;      // Minimum speed of virtual particles
    int maxVirtualParticleSpeed;      // Maximum speed of virtual particles
    int minVirtualParticleLifeTime;   // Minimum lifetime of virtual particles
    int maxVirtualParticleLifeTime;   // Maximum lifetime of virtual particles
    float minTimeBetweenVirtualPairs; // Minimum time between creation of virtual particle pairs

    // Physics
    float g;                  // Standard gravitational acceleration
    float gUniversal;         // Universal gravitational constant
    float maxGravityDistance; // Maximum distance for gravity to act
    float kElectric;          // Electric force constant
    float maxForce;           // Maximum allowed force
    GravityType gravityType;  // Gravity type
    bool electricForce;       // Enable/disable electric force
} SimulationConfig;

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

void InitGrid(const SimulationConfig *config, int gridWidth, int gridHeight);
void InitParticles(const SimulationConfig *config, Particles *particles, int screenWidth, int screenHeight);
void ResetParticles(const SimulationConfig *config, Particles *particles, int screenWidth, int screenHeight);
void Simulate(const SimulationConfig *config, Particles *particles, int screenWidth, int screenHeight, int gridWidth, int gridHeight);
void UpdateSimulation(const SimulationConfig *config, Particles *particles, RenderTexture2D *target, int screenWidth, int screenHeight, int gridWidth, int gridHeight);
void RenderSimulation(Shader glowShader, RenderTexture2D target);
void CleanupSimulation(Particles *particles, Shader glowShader, RenderTexture2D target, int gridWidth, int gridHeight);
void FreeGrid(int gridWidth, int gridHeight);
void GenerateVirtualParticles(const SimulationConfig *config, Particles *particles, float delta, int screenWidth, int screenHeight);
void HandleInput(const SimulationConfig *config, Particles *particles);

#endif // SIMULATION_H
