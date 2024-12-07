#include <assert.h>
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "simulation.h"

// Cell grid[GRID_WIDTH][GRID_HEIGHT];
Cell **grid;
int globalParticleID = 0;       // Global particle ID counter
float timeSinceLastPair = 0.0f; // Global timer for particle generation

void AssignParticlesToCells(Particles *particles, int gridWidth, int gridHeight);
void ResolveCollision(Particle *p1, Particle *p2);
bool Collide(Particle *p1, Particle *p2);

// Helpers
float GetRandomFloat(float min, float max)
{
    return min + (max - min) * GetRandomValue(0, 10000) / 10000.0f;
}

float Vector2Length(Vector2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

Vector2 Vector2Normalize(Vector2 v)
{
    float length = Vector2Length(v);
    Vector2 normalized = {0.0f, 0.0f};
    if (length != 0.0f)
    {
        normalized.x = v.x / length;
        normalized.y = v.y / length;
    }
    return normalized;
}

Vector2 Vector2Scale(Vector2 v, float scalar)
{
    Vector2 scaled = {v.x * scalar, v.y * scalar};
    return scaled;
}

Vector2 Vector2Subtract(Vector2 a, Vector2 b)
{
    Vector2 result = {a.x - b.x, a.y - b.y};
    return result;
}

Vector2 Vector2Add(Vector2 a, Vector2 b)
{
    Vector2 result = {a.x + b.x, a.y + b.y};
    return result;
}

float Vector2DotProduct(Vector2 a, Vector2 b)
{
    return (a.x * b.x) + (a.y * b.y);
}

void ResetSimulation(const SimulationConfig *config, Particles *particles, int screenWidth, int screenHeight)
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
    for (int x = 0; x < GRID_WIDTH; x++)
    {
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            grid[x][y].count = 0;
        }
    }
}

// void InitParticles(const SimulationConfig *config, Particles *particles, int screenWidth, int screenHeight)
// {
//     assert(screenWidth > 100 && screenHeight > 100 && "Screen dimensions are too small!");

//     for (int i = 0; i < (*config).initialCapacity; i++)
//     {
//         float mass = GetRandomValue((*config).minParticleMass, (*config).maxParticleMass);
//         float charge = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;
//         int xDirection = (GetRandomValue(0, 1) == 0) ? -1 : 1;
//         int yDirection = (GetRandomValue(0, 1) == 0) ? -1 : 1;

//         Particle p = (Particle){
//             .id = globalParticleID++,
//             .mass = mass,
//             .size = cbrtf(mass),
//             .charge = charge,
//             .position = (Vector2){GetRandomValue(50, screenWidth - 50), GetRandomValue(50, screenHeight - 50)},
//             .velocity = (Vector2){
//                 GetRandomValue((int)(*config).minParticleSpeed, (int)(*config).maxParticleSpeed) / 50.0, // x
//                 GetRandomValue((int)(*config).minParticleSpeed, (int)(*config).maxParticleSpeed) / 50.0  // y
//             },
//             // .color = (charge > 0) ? RED : BLUE,
//             .color = (Color){GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255},
//             .lifetime = GetRandomValue((int)(*config).minParticleLifeTime, (int)(*config).maxParticleLifeTime),
//             .isFragment = false,
//             .isVirtual = false,
//         };
//         // Randomize direction
//         p.velocity.x *= xDirection;
//         p.velocity.y *= yDirection;

//         int maxTrailLength = sizeof(p.trail) / sizeof(p.trail[0]);
//         for (int j = 0; j < (int)config->trailLength && j < maxTrailLength; j++)
//         {
//             p.trail[j] = p.position;
//         }
//         p.trailIndex = 0;
//         ARRAY_APPEND(particles, p);
//     }
// }

void InitRandomParticles(const SimulationConfig *config, Particles *particles, int screenWidth, int screenHeight)
{
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
            .position = (Vector2){GetRandomValue(50, screenWidth - 50), GetRandomValue(50, screenHeight - 50)},
            .velocity = (Vector2){
                GetRandomValue((int)config->minParticleSpeed, (int)config->maxParticleSpeed) / 50.0f, // x
                GetRandomValue((int)config->minParticleSpeed, (int)config->maxParticleSpeed) / 50.0f  // y
            },
            .color = (Color){GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255},
            .lifetime = GetRandomValue((int)config->minParticleLifeTime, (int)config->maxParticleLifeTime),
            .isFragment = false,
            .isVirtual = false,
        };
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

void InitVortexParticles(const SimulationConfig *config, Particles *particles, int screenWidth, int screenHeight)
{
    for (int i = 0; i < config->numVortexParticles && i < config->initialCapacity; i++)
    {
        float mass = GetRandomValue(config->minParticleMass, config->maxParticleMass);
        float charge = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;

        // Posición alrededor del centro del vórtice
        float angle = GetRandomValue(0, 360) * DEG2RAD;
        float radius = GetRandomValue(50, 300); // Puedes ajustar los límites
        Vector2 position = (Vector2){
            config->vortexCenter.x + radius * cosf(angle),
            config->vortexCenter.y + radius * sinf(angle)};

        // Velocidad tangencial al vórtice
        float speed = config->vortexStrength; // Puedes ajustar la fuerza del vórtice
        Vector2 velocity = (Vector2){
            -speed * sinf(angle), // Componente x
            speed * cosf(angle)   // Componente y
        };

        float radialSpeed = GetRandomValue(-10, 10) / 100.0f; // Ajusta según la necesidad
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

void InitGroupParticles(const SimulationConfig *config, Particles *particles, int screenWidth, int screenHeight)
{
    for (int g = 0; g < config->numGroups && g < config->initialCapacity; g++)
    {
        for (int i = 0; i < config->particlesPerGroup && ARRAY_LEN(particles) < config->initialCapacity; i++)
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

void InitBlackHoleParticles(const SimulationConfig *config, Particles *particles, int screenWidth, int screenHeight)
{
    for (int i = 0; i < config->initialCapacity; i++)
    {
        float mass = GetRandomValue(config->minParticleMass, config->maxParticleMass);
        float charge = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;

        // Posición aleatoria alrededor del agujero negro, fuera del horizonte de eventos
        float angle = GetRandomValue(0, 360) * DEG2RAD;
        float minRadius = config->blackHoleRadius + 10.0f; // Evitar superposición inicial
        float maxRadius = screenWidth / 2.0f;              // Ajusta según el tamaño de la simulación
        float radius = GetRandomValue((int)minRadius, (int)maxRadius);
        Vector2 position = (Vector2){
            config->blackHoleCenter.x + radius * cosf(angle),
            config->blackHoleCenter.y + radius * sinf(angle)};

        // Velocidad dirigida hacia el agujero negro con una magnitud proporcional a la distancia
        float speed = config->blackHoleMass / (radius * radius + 1.0f); // Ajusta la fórmula según necesidad
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

        // Inicializar el rastro
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
        InitRandomParticles(config, particles, screenWidth, screenHeight);
        break;

    case PATTERN_VORTEX:
        InitVortexParticles(config, particles, screenWidth, screenHeight);
        break;

    case PATTERN_GROUP:
        InitGroupParticles(config, particles, screenWidth, screenHeight);
        break;

    case PATTERN_BLACKHOLE:
        InitBlackHoleParticles(config, particles, screenWidth, screenHeight);
        break;

    default:
        TraceLog(LOG_WARNING, "Pattern not recognized, using random pattern");
        InitRandomParticles(config, particles, screenWidth, screenHeight);
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

void InitGrid(const SimulationConfig *config, int gridWidth, int gridHeight)
{
    grid = (Cell **)malloc(gridWidth * sizeof(Cell *));
    if (!grid)
    {
        TraceLog(LOG_ERROR, "Error al asignar memoria para grid");
        exit(EXIT_FAILURE);
    }

    for (int x = 0; x < gridWidth; x++)
    {
        grid[x] = (Cell *)malloc(gridHeight * sizeof(Cell));
        if (!grid[x])
        {
            TraceLog(LOG_ERROR, "Error al asignar memoria para grid[%d]", x);
            exit(EXIT_FAILURE);
        }

        for (int y = 0; y < gridHeight; y++)
        {
            grid[x][y].items = (Particle **)malloc(config->initialCapacity * sizeof(Particle *));
            if (!grid[x][y].items)
            {
                TraceLog(LOG_ERROR, "Error al asignar memoria para grid[%d][%d].items", x, y);
                exit(EXIT_FAILURE);
            }
            grid[x][y].count = 0;
            grid[x][y].capacity = config->initialCapacity;
        }
    }
}

void AssignParticlesToCells(Particles *particles, int gridWidth, int gridHeight)
{
    for (int x = 0; x < gridWidth; x++)
    {
        for (int y = 0; y < gridHeight; y++)
        {
            grid[x][y].count = 0;
        }
    }

    for (int i = 0; i < particles->count; i++)
    {
        Particle *p = &particles->items[i];
        int cellX = p->position.x / CELL_SIZE;
        int cellY = p->position.y / CELL_SIZE;

        if (cellX >= 0 && cellX < gridWidth && cellY >= 0 && cellY < gridHeight)
        {
            ARRAY_APPEND_PTR(&grid[cellX][cellY], p);
        }
    }
}

void Simulate(const SimulationConfig *config, Particles *particles, int screenWidth, int screenHeight, int gridWidth, int gridHeight)
{
    Particles newParticles = {
        .items = (Particle *)malloc((*config).maxParticles * sizeof(Particle)),
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

        // Update color based on speed
        float speed = sqrtf(p->velocity.x * p->velocity.x + p->velocity.y * p->velocity.y);
        // p->color = ColorFromHSV(fmodf(speed * 10.0f, 360.0f), 1.0f, 1.0f);

        // Update size based on lifetime
        if ((*config).lifetime || p->isVirtual)
        {
            if (i % 2 == 0) {
                p->size = cbrtf(p->mass) * (p->lifetime / (*config).maxParticleLifeTime);
                p->lifetime--;
            }
        }

        // Update trail
        p->trail[p->trailIndex] = p->position;
        p->trailIndex = (p->trailIndex + 1) % (int)((*config).trailLength);

        if (p->lifetime <= 0)
        {
            if (p->isVirtual)
                continue;
            if (!p->isFragment || (*config).fragmentParticlesLive)
            {
                Vector2 explosionCenter = p->position;
                int numExplosionParticles = GetRandomValue((*config).minExplosionParticles, (*config).maxExplosionParticles);

                for (int j = 0; j < numExplosionParticles; j++)
                {
                    float mass = GetRandomValue((int)(*config).minParticleMass, (int)(*config).maxParticleMass);
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
                        .lifetime = GetRandomValue((int)(*config).minParticleLifeTime, (int)(*config).maxParticleLifeTime),
                        .isFragment = true,
                        .isVirtual = false,
                    };
                    for (int t = 0; t < (int)((*config).trailLength); t++)
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

    if (particles->count + newParticles.count <= (*config).maxParticles)
    {
        for (int i = 0; i < newParticles.count; i++)
        {
            ARRAY_APPEND(particles, newParticles.items[i]);
        }
    }
    else
    {
        int spaceLeft = (*config).maxParticles - particles->count;
        for (int i = 0; i < spaceLeft; i++)
        {
            ARRAY_APPEND(particles, newParticles.items[i]);
        }
    }

    free(newParticles.items);

    AssignParticlesToCells(particles, gridWidth, gridHeight);

    // Process collisions and forces
    for (int i = 0; i < particles->count; i++)
    {
        Particle *p = &particles->items[i];

        // Screen limits control
        if (p->position.y <= 0 || p->position.y >= screenHeight)
            p->velocity.y *= -1;
        if (p->position.x <= 0 || p->position.x >= screenWidth)
            p->velocity.x *= -1;

        // Gravity
        switch ((*config).gravityType)
        {
        case GRAVITY_NONE:
            break;
        case GRAVITY_CENTER:
        {
            Vector2 center = {screenWidth / 2.0f, screenWidth / 2.0f};
            Vector2 dir = {center.x - p->position.x, center.y - p->position.y};
            float distance = sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (distance != 0)
            {
                dir.x /= distance;
                dir.y /= distance;
                p->velocity.x += dir.x * (*config).g * 0.01f;
                p->velocity.y += dir.y * (*config).g * 0.01f;
            }
        }
        break;
        case GRAVITY_DOWN:
            p->velocity.y += (*config).g * 0.01f;
            break;
        case GRAVITY_UP:
            p->velocity.y -= (*config).g * 0.01f;
            break;
        case GRAVITY_LEFT:
            p->velocity.x -= (*config).g * 0.01f;
            break;
        case GRAVITY_RIGHT:
            p->velocity.x += (*config).g * 0.01f;
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
                // Normalizar la dirección
                Vector2 dirNormalized = (Vector2){direction.x / distance, direction.y / distance};

                // **Fuerza Tangencial:**
                // Rotar la dirección normalizada 90 grados para obtener la dirección tangencial
                Vector2 tangential = (Vector2){dirNormalized.y, -dirNormalized.x};

                // Aplicar la fuerza tangencial proporcional a la fuerza del vórtice y la inversa de la distancia
                Vector2 tangentialForce = (Vector2){tangential.x * config->vortexStrength / (distance + 1.0f), tangential.y * config->vortexStrength / (distance + 1.0f)};
                p->velocity = (Vector2){p->velocity.x + tangentialForce.x, p->velocity.y + tangentialForce.y};

                // **Fuerza Radial:**
                // Aplicar una fuerza radial para crear el efecto espiral
                // Puedes ajustar la dirección (hacia adentro o afuera) cambiando el signo
                Vector2 radialForce = (Vector2){dirNormalized.x * config->radialStrength / (distance + 1.0f), dirNormalized.y * config->radialStrength / (distance + 1.0f)};
                p->velocity = (Vector2){p->velocity.x + radialForce.x, p->velocity.y + radialForce.y};
            }
        }
        break;
        case PATTERN_GROUP:
            break;
        case PATTERN_BLACKHOLE:
        {
            // Calcular la dirección y distancia desde el agujero negro
            Vector2 direction = Vector2Subtract(config->blackHoleCenter, p->position);
            float distance = Vector2Length(direction);

            if (distance > 0)
            {
                // Normalizar la dirección
                Vector2 dirNormalized = Vector2Scale(direction, 1.0f / distance);

                // **Fuerza Gravitacional:**
                // Aplicar una fuerza gravitacional hacia el agujero negro
                // Puedes usar la ley de gravitación universal simplificada
                float gravitationalForce = (config->blackHoleMass) / (distance * distance + 1.0f); // +1.0f para evitar división por cero
                Vector2 gravityForce = Vector2Scale(dirNormalized, gravitationalForce);
                p->velocity = Vector2Add(p->velocity, Vector2Scale(gravityForce, GetFrameTime()));

                // **Verificar si la partícula cruza el horizonte de eventos**
                if (distance <= config->blackHoleRadius)
                {
                    // Eliminar la partícula de la simulación
                    particles->items[i] = particles->items[particles->count - 1];
                    particles->count--;
                    i--; // Revisar la nueva partícula en esta posición
                    continue;
                }
            }
        }
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

                if (neighborX >= 0 && neighborX < gridWidth && neighborY >= 0 && neighborY < gridHeight)
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
                                ResolveCollision(p, other);
                            }

                            // Compute forces
                            Vector2 delta = {
                                other->position.x - p->position.x,
                                other->position.y - p->position.y};

                            float distance = sqrtf(delta.x * delta.x + delta.y * delta.y);

                            if (distance > 1.0f && distance < (*config).maxGravityDistance)
                            {
                                delta.x /= distance;
                                delta.y /= distance;

                                float force = (*config).gUniversal * (p->mass * other->mass) / (distance * distance);

                                if (force > (*config).maxForce)
                                {
                                    force = (*config).maxForce;
                                }

                                p->velocity.x += force / p->mass * delta.x;
                                p->velocity.y += force / p->mass * delta.y;

                                if ((*config).electricForce)
                                {
                                    float force_electric = (*config).kElectric * (p->charge * other->charge) / (distance * distance);

                                    if (fabsf(force_electric) > (*config).maxForce)
                                    {
                                        force_electric = (force_electric > 0) ? (*config).maxForce : -(*config).maxForce;
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
        for (int t = 0; t < (int)((*config).trailLength) - 1; t++)
        {
            int index = (p->trailIndex + t) % (int)((*config).trailLength);
            int nextIndex = (index + 1) % (int)((*config).trailLength);
            DrawLineEx(
                p->trail[index],
                p->trail[nextIndex],
                2.0f,
                Fade(p->color, (float)t / (int)((*config).trailLength)));
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

void UpdateSimulation(const SimulationConfig *config, Particles *particles, RenderTexture2D *target, int screenWidth, int screenHeight, int gridWidth, int gridHeight)
{
    BeginTextureMode(*target);
    ClearBackground(BLACK);
    Simulate(config, particles, screenWidth, screenHeight, gridWidth, gridHeight);
    EndTextureMode();
}

void CleanupSimulation(Particles *particles, int gridWidth, int gridHeight)
{
    for (int x = 0; x < gridWidth; x++)
    {
        for (int y = 0; y < gridHeight; y++)
        {
            if (grid[x][y].items != NULL)
            {
                free(grid[x][y].items);
                grid[x][y].items = NULL;
            }
        }
    }
    if (particles->items != NULL)
    {
        free(particles->items);
        particles->items = NULL;
    }
}

void FreeGrid(int gridWidth, int gridHeight)
{
    for (int x = 0; x < gridWidth; x++)
    {
        for (int y = 0; y < gridHeight; y++)
        {
            if (grid[x][y].items != NULL)
                free(grid[x][y].items);
        }
        if (grid[x] != NULL)
            free(grid[x]);
    }
    if (grid != NULL)
        free(grid);
}