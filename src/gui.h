#ifndef GUI_H
#define GUI_H

#include "simulation.h"

#define MAX_TABS 6

typedef struct
{
    float value;        // Numeric value
    char textValue[50]; // String representation
} NumericValue;

typedef struct
{
    int *value;         // Integer value
    char textValue[50]; // String representation
} IntValue;

typedef struct
{
    float *value;
    char textValue[50];
} FloatValue;

typedef struct
{
    bool *value; // Boolean value
} BoolValue;

typedef struct
{
    IntValue maxParticles;
    IntValue initialCapacity;
    int seed;
    BoolValue lifetime;
    BoolValue fragmentParticlesLive;
    BoolValue virtualParticles;
    FloatValue friction;

} GeneralTabParameters;

typedef struct
{
    ParticlePattern *initialPattern;

    bool dropDownOpen;

    // Vortex Pattern
    Vector2 *vortexCenter;
    FloatValue vortexStrength;
    FloatValue radialStrength;
    FloatValue vortexRadious;

    // Group Pattern
    IntValue numGroups;
    Vector2 *groupCenters;
    IntValue particlesPerGroup;
} PatternsTabParameters;

typedef struct
{
    FloatValue minParticleLifeTime;
    FloatValue maxParticleLifeTime;
    FloatValue minParticleSpeed;
    FloatValue maxParticleSpeed;
    FloatValue minParticleMass;
    FloatValue maxParticleMass;
    IntValue trailLength;
} ParticlesTabParameters;

typedef struct 
{
    IntValue minExplosionParticles;
    IntValue maxExplosionParticles;
} ExplosionTabParameters;


typedef struct {
    IntValue minVirtualParticleSpeed;
    IntValue maxVirtualParticleSpeed;
    IntValue minVirtualParticleLifeTime;
    IntValue maxVirtualParticleLifeTime;
    FloatValue minTimeBetweenVirtualPairs;
} VirtualParticlesTabParameters;

typedef struct {
    FloatValue g;
    FloatValue gUniversal;
    FloatValue maxGravityDistance;
    FloatValue maxForce;
    FloatValue kElectric;
    GravityType *gravityType;
    BoolValue electricForce;
    bool dropDownOpen;
} PhysicsTabParameters;

void generalTab(GeneralTabParameters *gtp);
void patternsTab(PatternsTabParameters *ptp);
void particlesTab(ParticlesTabParameters *particlesTabParameters);
void explosionTab(ExplosionTabParameters *etp);
void virtualParticlesTab(VirtualParticlesTabParameters *vtp);
void physicsTab(PhysicsTabParameters *phtp);

#endif // GUI_H
