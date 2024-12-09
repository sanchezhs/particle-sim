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
            void *temp = realloc((array)->items, (array)->capacity * sizeof(*(array)->items));     \
            if (temp == NULL)                                                                      \
            {                                                                                      \
                fprintf(stderr, "Memory allocation failed. Exiting...\n");                         \
                exit(EXIT_FAILURE);                                                                \
            }                                                                                      \
            (array)->items = temp;                                                                 \
        }                                                                                          \
                                                                                                   \
        (array)->items[(array)->count++] = (item);                                                 \
    } while (0)

#define ARRAY_LEN(array) (sizeof(array) / sizeof((array)[0]))
#define UNUSED(x) (void)(x)

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

typedef enum {
    PATTERN_RANDOM,
    PATTERN_VORTEX,
    PATTERN_GROUP,
    PATTERN_BLACKHOLE
} ParticlePattern;

// Structure to store all simulation parameters
typedef struct
{
    ParticlePattern initialPattern;
    Vector2 vortexCenter;
    float vortexStrength;
    float radialStrength;
    float vortexRadius;

    int numGroups;
    Vector2 *groupCenters;
    int particlesPerGroup;

    Vector2 blackHoleCenter;       // Posición del agujero negro
    float blackHoleMass;           // Masa del agujero negro
    float blackHoleRadius;   

    // General
    int maxParticles;           // Maximum number of particles
    int initialCapacity;        // Initial capacity of the particle system
    bool lifetime;              // Enable/disable particle lifetime
    bool fragmentParticlesLive; // Fragments particles have their own lifetime
    bool virtualParticles;      // Enable virtual particles
    float friction;             // Simulation friction value for particle's speed

    // Particles
    float minParticleLifeTime; // Minimum particle lifetime
    float maxParticleLifeTime; // Maximum particle lifetime
    float minParticleSpeed;    // Minimum particle speed
    float maxParticleSpeed;    // Maximum particle speed
    float minParticleMass;     // Minimum particle mass
    float maxParticleMass;     // Maximum particle mass
    int trailLength;           // Length of particle trails

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

typedef struct ParticleNode {
    Particle *particle;
    struct ParticleNode *next;
} ParticleNode;

typedef struct GridCell {
    int cellX;
    int cellY;
    ParticleNode *particles;
    struct GridCell *next;
} GridCell;

#define HASH_TABLE_SIZE 10007

typedef struct {
    GridCell *buckets[HASH_TABLE_SIZE];
} HashTable;

// Spatial partitioning
HashTable *InitHashTable(void);
void AssignParticlesToCells(HashTable *table, Particles *particles, float cellSize);
void ClearHashTable(HashTable *table);
void FreeHashTable(HashTable *table);
GridCell *GetOrCreateGridCell(HashTable *table, int cellX, int cellY);
GridCell *GetGridCell(HashTable *table, int cellX, int cellY);

// Particles
void InitParticles(const SimulationConfig *config, Particles *particles, int screenWidth, int screenHeight);
void RenderParticles(HashTable *grid, Camera2D camera, float cellSize);
void GenerateVirtualParticles(const SimulationConfig *config, Particles *particles, float delta, int screenWidth, int screenHeight);

// Simulation
void UpdateSimulation(const SimulationConfig *config, Particles *particles, HashTable *grid, int screenWidth, int screenHeight);
void ResetSimulation(const SimulationConfig *config, Particles *particles, HashTable *grid, int screenWidth, int screenHeight);

// Handlers
void HandleInput(const SimulationConfig *config, Particles *particles, int screenWidth, int screenHeight);

#endif // SIMULATION_H
