#include <assert.h>
#include <math.h>
#include "./external/raylib/src/raylib.h"
#include "./external/raylib/src/raymath.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "simulation.h"

int globalParticleID = 0;       // Global particle ID counter
float timeSinceLastPair = 0.0f; // Global timer for particle generation

void ResolveCollision(Particle *p1, Particle *p2);
bool Collide(Particle *p1, Particle *p2);
float GetRandomFloat(float min, float max);
unsigned int HashFunction(int cellX, int cellY);

// Helpers
float GetRandomFloat(float min, float max)
{
    return min + (max - min) * GetRandomValue(0, 10000) / 10000.0f;
}

unsigned int HashFunction(int cellX, int cellY)
{
    unsigned int hash = 5381;
    hash = ((hash << 5) + hash) + cellX; // hash * 33 + cellX
    hash = ((hash << 5) + hash) + cellY; // hash * 33 + cellY
    return hash % HASH_TABLE_SIZE;
}

HashTable* InitHashTable(void) {
    HashTable *table = malloc(sizeof(HashTable));
    memset(table->buckets, 0, sizeof(table->buckets));
    return table;
}

GridCell *GetOrCreateGridCell(HashTable *table, int cellX, int cellY)
{
    unsigned int index = HashFunction(cellX, cellY);
    GridCell *cell = table->buckets[index];

    while (cell != NULL)
    {
        if (cell->cellX == cellX && cell->cellY == cellY)
        {
            return cell;
        }
        cell = cell->next;
    }

    cell = malloc(sizeof(GridCell));
    cell->cellX = cellX;
    cell->cellY = cellY;
    cell->particles = NULL;
    cell->next = table->buckets[index];
    table->buckets[index] = cell;
    return cell;
}

GridCell *GetGridCell(HashTable *table, int cellX, int cellY)
{
    unsigned int index = HashFunction(cellX, cellY);
    GridCell *cell = table->buckets[index];

    while (cell != NULL)
    {
        if (cell->cellX == cellX && cell->cellY == cellY)
        {
            return cell;
        }
        cell = cell->next;
    }

    return NULL;
}

void ClearHashTable(HashTable *table)
{
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        GridCell *cell = table->buckets[i];
        while (cell != NULL)
        {
            ParticleNode *node = cell->particles;
            while (node != NULL)
            {
                ParticleNode *temp = node;
                node = node->next;
                free(temp);
            }
            GridCell *tempCell = cell;
            cell = cell->next;
            free(tempCell);
        }
        table->buckets[i] = NULL;
    }
}

void FreeHashTable(HashTable *table)
{
    ClearHashTable(table);
    free(table);
}

void ResetSimulation(const SimulationConfig *config, Particles *particles, HashTable *grid, int screenWidth, int screenHeight)
{
    particles->count = 0;
    globalParticleID = 0;

    if (particles->items != NULL)
    {
        free(particles->items);
        particles->items = NULL;
    }

    particles->capacity = config->initialCapacity;

    particles->items = (Particle *)malloc(particles->capacity * sizeof(Particle));
    if (particles->items == NULL)
    {
        TraceLog(LOG_ERROR, "Memory allocation failed during Resetsimulation");
        return;
    }

    InitParticles(config, particles, screenWidth, screenHeight);

    // Reset grid
    ClearHashTable(grid);
}

void InitRandomParticles(const SimulationConfig *config, Particles *particles)
{
    int initialRange = 10000;
    for (int i = 0; i < config->initialCapacity; i++)
    {
        float mass = GetRandomValue(config->minParticleMass, config->maxParticleMass);
        float charge = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;
        int xDirection = (GetRandomValue(0, 1) == 0) ? -1 : 1;
        int yDirection = (GetRandomValue(0, 1) == 0) ? -1 : 1;

        Particle p = (Particle){
            .id = globalParticleID++,
            .mass = mass,
            .size = cbrtf(mass),
            .charge = charge,
            .position = (Vector2){GetRandomValue(-initialRange, initialRange), GetRandomValue(-initialRange, initialRange)},
            .velocity = (Vector2){
                GetRandomValue((int)config->minParticleSpeed, (int)config->maxParticleSpeed) / 50.0f, // x
                GetRandomValue((int)config->minParticleSpeed, (int)config->maxParticleSpeed) / 50.0f  // y
            },
            .color = (Color){GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255},
            .lifetime = GetRandomValue((int)config->minParticleLifeTime, (int)config->maxParticleLifeTime),
            .isFragment = false,
            .isVirtual = false,
        };

        // Avoid zero velocity
        if (p.velocity.x == 0)
            p.velocity.x += 1.0f;
        if (p.velocity.y == 0)
            p.velocity.y += 1.0f;

        // Randomize direction
        p.velocity.x *= xDirection;
        p.velocity.y *= yDirection;

        int maxTrailLength = sizeof(p.trail) / sizeof(p.trail[0]);
        for (int j = 0; j < (int)config->trailLength && j < maxTrailLength; j++)
        {
            p.trail[j] = p.position;
        }
        p.trailIndex = 0;
        ARRAY_APPEND(particles, p);
    }
}

void InitVortexParticles(const SimulationConfig *config, Particles *particles)
{
    for (int i = 0; i < config->initialCapacity; i++)
    {
        float mass = GetRandomValue(config->minParticleMass, config->maxParticleMass);
        float charge = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;

        // Position around vortex center
        float angle = GetRandomValue(0, 360) * DEG2RAD;
        float radius = config->vortexRadius;
        Vector2 position = (Vector2){
            config->vortexCenter.x + radius * cosf(angle),
            config->vortexCenter.y + radius * sinf(angle)};

        // Tangent speed towards vortex
        float speed = config->vortexStrength;
        Vector2 velocity = (Vector2){
            -speed * sinf(angle),
            speed * cosf(angle)};

        float radialSpeed = GetRandomValue(-10, 10) / 150.0f;
        velocity = (Vector2){velocity.x + radialSpeed * cosf(angle), velocity.y + radialSpeed * sinf(angle)};

        Particle p = (Particle){
            .id = globalParticleID++,
            .mass = mass,
            .size = cbrtf(mass),
            .charge = charge,
            .position = position,
            .velocity = velocity,
            .color = (Color){GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255},
            .lifetime = GetRandomValue((int)config->minParticleLifeTime, (int)config->maxParticleLifeTime),
            .isFragment = false,
            .isVirtual = false,
        };

        int maxTrailLength = sizeof(p.trail) / sizeof(p.trail[0]);
        for (int j = 0; j < (int)config->trailLength && j < maxTrailLength; j++)
        {
            p.trail[j] = p.position;
        }
        p.trailIndex = 0;
        ARRAY_APPEND(particles, p);
    }
}

void InitGroupParticles(const SimulationConfig *config, Particles *particles)
{
    for (int g = 0; g < config->numGroups && g < config->initialCapacity; g++)
    {
        for (int i = 0; i < config->particlesPerGroup && particles->count < config->initialCapacity; i++)
        {
            float mass = GetRandomValue(config->minParticleMass, config->maxParticleMass);
            float charge = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;

            // Posición alrededor del centro del grupo
            float angle = GetRandomValue(0, 360) * DEG2RAD;
            float radius = GetRandomValue(0, 150); // Radio pequeño para agrupar
            Vector2 position = (Vector2){
                config->groupCenters[g].x + radius * cosf(angle),
                config->groupCenters[g].y + radius * sinf(angle)};

            // Velocidad aleatoria
            float speed = GetRandomValue((int)config->minParticleSpeed, (int)config->maxParticleSpeed) / 25.0f;
            Vector2 velocity = (Vector2){
                GetRandomValue(-speed, speed),
                GetRandomValue(-speed, speed)};

            Particle p = (Particle){
                .id = globalParticleID++,
                .mass = mass,
                .size = cbrtf(mass),
                .charge = charge,
                .position = position,
                .velocity = velocity,
                .color = (Color){GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255},
                .lifetime = GetRandomValue((int)config->minParticleLifeTime, (int)config->maxParticleLifeTime),
                .isFragment = false,
                .isVirtual = false,
            };

            int maxTrailLength = sizeof(p.trail) / sizeof(p.trail[0]);
            for (int j = 0; j < (int)config->trailLength && j < maxTrailLength; j++)
            {
                p.trail[j] = p.position;
            }
            p.trailIndex = 0;
            ARRAY_APPEND(particles, p);
        }
    }
}

void InitBlackHoleParticles(const SimulationConfig *config, Particles *particles, int screenWidth)
{
    for (int i = 0; i < config->initialCapacity; i++)
    {
        float mass = GetRandomValue(config->minParticleMass, config->maxParticleMass);
        float charge = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;

        float angle = GetRandomValue(0, 360) * DEG2RAD;
        float minRadius = config->blackHoleRadius + 10.0f; 
        float maxRadius = screenWidth / 2.0f;              
        float radius = GetRandomValue((int)minRadius, (int)maxRadius);
        Vector2 position = (Vector2){
            config->blackHoleCenter.x + radius * cosf(angle),
            config->blackHoleCenter.y + radius * sinf(angle)};

        float speed = config->blackHoleMass / (radius * radius + 1.0f);
        Vector2 direction = Vector2Subtract(config->blackHoleCenter, position);
        float distance = Vector2Length(direction);
        Vector2 dirNormalized = Vector2Scale(direction, 1.0f / distance);
        Vector2 velocity = Vector2Scale(dirNormalized, speed);

        Particle p = (Particle){
            .id = globalParticleID++,
            .mass = mass,
            .size = cbrtf(mass),
            .charge = charge,
            .position = position,
            .velocity = velocity,
            .color = (Color){GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255},
            .lifetime = GetRandomValue((int)config->minParticleLifeTime, (int)config->maxParticleLifeTime),
            .isFragment = false,
            .isVirtual = false,
        };

        int maxTrailLength = sizeof(p.trail) / sizeof(p.trail[0]);
        for (int j = 0; j < (int)config->trailLength && j < maxTrailLength; j++)
        {
            p.trail[j] = p.position;
        }
        p.trailIndex = 0;

        ARRAY_APPEND(particles, p);
    }
}

void InitParticles(const SimulationConfig *config, Particles *particles, int screenWidth, int screenHeight)
{
    assert(screenWidth > 100 && screenHeight > 100 && "Screen dimensions are too small!");
    switch (config->initialPattern)
    {
    case PATTERN_RANDOM:
        InitRandomParticles(config, particles);
        break;

    case PATTERN_VORTEX:
        InitVortexParticles(config, particles);
        break;

    case PATTERN_GROUP:
        InitGroupParticles(config, particles);
        break;

    case PATTERN_BLACKHOLE:
        InitBlackHoleParticles(config, particles, screenWidth);
        break;

    default:
        TraceLog(LOG_WARNING, "Pattern not recognized, using random pattern");
        InitRandomParticles(config, particles);
        break;
    }
}

void GenerateVirtualParticles(const SimulationConfig *config, Particles *particles, float delta, int screenWidth, int screenHeight)
{
    timeSinceLastPair += delta;
    if (timeSinceLastPair < (*config).minTimeBetweenVirtualPairs)
    {
        return;
    }

    float prob = GetRandomFloat(0, 1);

    if (prob > 0.01)
    {
        return;
    }

    timeSinceLastPair = 0.0f;

    int lifetime = GetRandomValue((int)(*config).minVirtualParticleLifeTime, (int)(*config).maxVirtualParticleLifeTime);
    int randx = GetRandomValue(0, screenWidth);
    int randy = GetRandomValue(0, screenHeight);
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
        .velocity = (Vector2){GetRandomValue((int)(*config).minVirtualParticleSpeed, (int)(*config).maxVirtualParticleSpeed) / 100.0f, GetRandomValue((int)(*config).minVirtualParticleSpeed, (int)(*config).maxVirtualParticleSpeed) / 100.0f},
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

    for (int j = 0; j < (int)((*config).trailLength); j++)
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

void ResolveCollision(Particle *p1, Particle *p2)
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

void AssignParticlesToCells(HashTable *grid, Particles *particles, float cellSize)
{
    ClearHashTable(grid);

    for (int i = 0; i < particles->count; i++)
    {
        Particle *p = &particles->items[i];
        int cellX = floor(p->position.x / cellSize);
        int cellY = floor(p->position.y / cellSize);
        GridCell *cell = GetOrCreateGridCell(grid, cellX, cellY);
        ParticleNode *newNode = malloc(sizeof(ParticleNode));
        newNode->particle = p;
        newNode->next = cell->particles;
        cell->particles = newNode;
    }
}

void RenderParticles(HashTable *grid, Camera2D camera, float cellSize)
{
    Vector2 topLeft = GetScreenToWorld2D((Vector2){0, 0}, camera);
    Vector2 bottomRight = GetScreenToWorld2D((Vector2){GetScreenWidth(), GetScreenHeight()}, camera);
    
    int minCellX = floor(topLeft.x / cellSize);
    int maxCellX = floor(bottomRight.x / cellSize);
    int minCellY = floor(topLeft.y / cellSize);
    int maxCellY = floor(bottomRight.y / cellSize);
    
    for (int cellX = minCellX; cellX <= maxCellX; cellX++)
    {
        for (int cellY = minCellY; cellY <= maxCellY; cellY++)
        {
            GridCell *cell = GetGridCell(grid, cellX, cellY);
            if (cell != NULL && cell->particles != NULL)
            {
                ParticleNode *node = cell->particles;
                while (node != NULL)
                {
                    Particle *p = node->particle;
                    DrawCircleV(p->position, p->size, p->color);
                    node = node->next;
                }
            }
        }
    }
}

void UpdateSimulation(const SimulationConfig *config, Particles *particles, HashTable *grid, int screenWidth, int screenHeight)
{
    Particles newParticles = {
        .items = (Particle *)malloc(config->maxParticles * sizeof(Particle)),
        .count = 0,
        .capacity = (*config).initialCapacity,
    };

    int alive = 0;

    for (int i = 0; i < particles->count; i++)
    {
        Particle *p = &particles->items[i];

        // Update position
        p->position.x += p->velocity.x;
        p->position.y += p->velocity.y;

        // Update size based on lifetime
        if (config->lifetime || p->isVirtual)
        {
            if (i % 2 == 0)
            {
                p->size = cbrtf(p->mass) * (p->lifetime / config->maxParticleLifeTime);
                p->lifetime--;
            }
        }

        // Update trail
        p->trail[p->trailIndex] = p->position;
        p->trailIndex = (p->trailIndex + 1) % (int)(config->trailLength);

        if (p->lifetime <= 0)
        {
            if (p->isVirtual)
                continue;
            if (!p->isFragment || config->fragmentParticlesLive)
            {
                Vector2 explosionCenter = p->position;
                int numExplosionParticles = GetRandomValue(config->minExplosionParticles, config->maxExplosionParticles);

                for (int j = 0; j < numExplosionParticles; j++)
                {
                    float mass = GetRandomValue((int)config->minParticleMass, (int)config->maxParticleMass);
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
                        .lifetime = GetRandomValue((int)config->minParticleLifeTime, (int)config->maxParticleLifeTime),
                        .isFragment = true,
                        .isVirtual = false,
                    };
                    for (int t = 0; t < (int)(config->trailLength); t++)
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

    if (particles->count + newParticles.count <= config->maxParticles)
    {
        for (int i = 0; i < newParticles.count; i++)
        {
            ARRAY_APPEND(particles, newParticles.items[i]);
        }
    }
    else
    {
        int spaceLeft = config->maxParticles - particles->count;
        for (int i = 0; i < spaceLeft; i++)
        {
            ARRAY_APPEND(particles, newParticles.items[i]);
        }
    }

    free(newParticles.items);

    AssignParticlesToCells(grid, particles, CELL_SIZE);

    // Process collisions and forces
    for (int i = 0; i < particles->count; i++)
    {
        Particle *p = &particles->items[i];

        if (config->friction != 1)
        {
            p->velocity = Vector2Scale(p->velocity, config->friction);
        }

        // // Screen limits control
        // if (p->position.y <= 0 || p->position.y >= screenHeight)
        //     p->velocity.y *= -1;
        // if (p->position.x <= 0 || p->position.x >= screenWidth)
        //     p->velocity.x *= -1;

        // Gravity
        switch (config->gravityType)
        {
        case GRAVITY_NONE:
            break;
        case GRAVITY_CENTER:
        {
            Vector2 center = {screenHeight / 2.0f, screenWidth / 2.0f};
            Vector2 dir = {center.x - p->position.x, center.y - p->position.y};
            float distance = sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (distance != 0)
            {
                dir.x /= distance;
                dir.y /= distance;
                p->velocity.x += dir.x * config->g * 0.01f;
                p->velocity.y += dir.y * config->g * 0.01f;
            }
        }
        break;
        case GRAVITY_DOWN:
            p->velocity.y += config->g * 0.01f;
            break;
        case GRAVITY_UP:
            p->velocity.y -= config->g * 0.01f;
            break;
        case GRAVITY_LEFT:
            p->velocity.x -= config->g * 0.01f;
            break;
        case GRAVITY_RIGHT:
            p->velocity.x += config->g * 0.01f;
            break;
        default:
            break;
        }

        switch (config->initialPattern)
        {
        case PATTERN_RANDOM:
            break;
        case PATTERN_VORTEX:
        {
            Vector2 direction = (Vector2){config->vortexCenter.x - p->position.x, config->vortexCenter.y - p->position.y};
            float distance = sqrtf(direction.x * direction.x + direction.y * direction.y);

            if (distance > 0)
            {
                Vector2 dirNormalized = Vector2Scale(direction, 1.0f / distance);

                // Tangent force
                Vector2 tangential = (Vector2){dirNormalized.y, -dirNormalized.x};

                Vector2 tangentialForce = (Vector2){tangential.x * config->vortexStrength / (distance + 1.0f), tangential.y * config->vortexStrength / (distance + 1.0f)};
                p->velocity = Vector2Add(p->velocity, tangentialForce);

                // Radial force
                Vector2 radialForce = (Vector2){dirNormalized.x * config->radialStrength / (distance + 1.0f), dirNormalized.y * config->radialStrength / (distance + 1.0f)};
                p->velocity = Vector2Add(p->velocity, radialForce);
            }
        }
        break;
        case PATTERN_GROUP:
            break;
        case PATTERN_BLACKHOLE:
        {
            Vector2 direction = Vector2Subtract(config->blackHoleCenter, p->position);
            float distance = Vector2Length(direction);

            if (distance > 0)
            {
                Vector2 dirNormalized = Vector2Scale(direction, 1.0f / distance);

                float gravitationalForce = (config->blackHoleMass) / (distance * distance + 1.0f);
                Vector2 gravityForce = Vector2Scale(dirNormalized, gravitationalForce);
                p->velocity = Vector2Add(p->velocity, Vector2Scale(gravityForce, GetFrameTime()));

                // Particle enters event horizon
                if (distance <= config->blackHoleRadius)
                {
                    particles->items[i] = particles->items[particles->count - 1];
                    particles->count--;
                    i--;
                    continue;
                }
            }
        }
        default:
            break;
        }

        // Computes the current cell of the particle
        int cellX = floor(p->position.x / CELL_SIZE);
        int cellY = floor(p->position.y / CELL_SIZE);

        // Iterate over adjacent cells (including the current cell)
        for (int offsetX = -1; offsetX <= 1; offsetX++)
        {
            for (int offsetY = -1; offsetY <= 1; offsetY++)
            {
                int neighborX = cellX + offsetX;
                int neighborY = cellY + offsetY;

                // Get neighbor cell without creating it if it doesn't exist
                GridCell *neighborCell = GetGridCell(grid, neighborX, neighborY);

                if (neighborCell != NULL)
                {
                    ParticleNode *node = neighborCell->particles;
                    while (node != NULL)
                    {
                        Particle *other = node->particle;

                        if (p == other)
                        {
                            node = node->next;
                            continue;
                        }

                        if (p->id < other->id)
                        {
                            if (Collide(p, other))
                            {
                                ResolveCollision(p, other);
                            }

                            // Compute forces
                            Vector2 delta = {
                                other->position.x - p->position.x,
                                other->position.y - p->position.y};

                            float distance = sqrtf(delta.x * delta.x + delta.y * delta.y);

                            if (distance > 1.0f && distance < config->maxGravityDistance)
                            {
                                delta.x /= distance;
                                delta.y /= distance;

                                float force = config->gUniversal * (p->mass * other->mass) / (distance * distance);

                                if (force > config->maxForce)
                                {
                                    force = config->maxForce;
                                }

                                p->velocity.x += force / p->mass * delta.x;
                                p->velocity.y += force / p->mass * delta.y;

                                if (config->electricForce)
                                {
                                    float force_electric = config->kElectric * (p->charge * other->charge) / (distance * distance);

                                    if (fabsf(force_electric) > config->maxForce)
                                    {
                                        force_electric = (force_electric > 0) ? config->maxForce : -config->maxForce;
                                    }
                                    p->velocity.x += force_electric / p->mass * delta.x;
                                    p->velocity.y += force_electric / p->mass * delta.y;
                                }
                            }
                        }
                        node = node->next;
                    }
                }
            }
        }
    }
}

void HandleInput(const SimulationConfig *config, Particles *particles, int screenWidth, int screenHeight)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Vector2 mousePos = GetMousePosition();

        if (mousePos.x > 0 && mousePos.x < screenWidth && mousePos.y > 0 && mousePos.y < screenHeight)
        {
            float mass = GetRandomValue((*config).minParticleMass, (*config).maxParticleMass);
            float charge = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;

            Particle newParticle = {
                .id = globalParticleID++,
                .mass = mass,
                .size = cbrtf(mass),
                .charge = charge,
                .position = GetMousePosition(),
                .velocity = (Vector2){GetRandomValue(-50, 50) / 10.0f, GetRandomValue(-50, 50) / 10.0f},
                .color = (charge > 0) ? RED : BLUE,
                .lifetime = GetRandomValue((*config).minParticleLifeTime, (*config).maxParticleLifeTime),
                .isFragment = false,
            };
            for (int j = 0; j < (int)(*config).trailLength; j++)
            {
                newParticle.trail[j] = newParticle.position;
            }
            newParticle.trailIndex = 0;
            ARRAY_APPEND(particles, newParticle);
        }
    }
}
