#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>
#include "raygui.h"
#include "gui.h"

void generalTab(int *maxParticles, int *initialCapacity, int *seed, bool *lifetime, bool *fragmentParticlesLive, bool *virtualParticles)
{
    float screenWidth = (float)GetScreenWidth();
    float screenHeight = (float)GetScreenHeight();

    float xOffset = screenWidth * 0.05f;
    float yOffset = screenHeight * 0.1f;

    float controlHeight = screenHeight * 0.03f;
    float labelWidth = screenWidth * 0.25f;
    float controlWidth = screenWidth * 0.375f;
    float verticalSpacing = screenHeight * 0.05f;

    yOffset += verticalSpacing;
    float maxParticlesTemp = (float)(*maxParticles);
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Maximum particles");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &maxParticlesTemp, 1.0f, 10000.0f);
    *maxParticles = (int)maxParticlesTemp;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, maxParticles, 1, 10000, false);

    yOffset += verticalSpacing;
    float initialCapacityTemp = (float)(*initialCapacity);
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Initial Particles");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &initialCapacityTemp, 1.0f, 10000.0f);
    *initialCapacity = (int)initialCapacityTemp;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, initialCapacity, 1, 10000, false);

    yOffset += verticalSpacing;
    if (GuiButton((Rectangle){xOffset, yOffset, labelWidth * 0.75f, controlHeight}, "Set Seed"))
    {
        *seed = GetRandomValue(0, 10000);
        SetRandomSeed(*seed);
        TraceLog(LOG_INFO, "Random seed set");
    }
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth * 0.4f, controlHeight}, NULL, seed, 0, 10000, false);

    yOffset += verticalSpacing;
    if (GuiCheckBox((Rectangle){xOffset, yOffset, controlHeight, controlHeight}, "Lifetime", lifetime))
    {
        TraceLog(LOG_INFO, "Lifetime: %s", *lifetime ? "true" : "false");
    }

    yOffset += verticalSpacing;
    if (GuiCheckBox((Rectangle){xOffset, yOffset, controlHeight, controlHeight}, "Fragment particles live", fragmentParticlesLive))
    {
        TraceLog(LOG_INFO, "Fragment particles live: %s", *fragmentParticlesLive ? "true" : "false");
    }

    yOffset += verticalSpacing;
    if (GuiCheckBox((Rectangle){xOffset, yOffset, controlHeight, controlHeight}, "Virtual particles", virtualParticles))
    {
        TraceLog(LOG_INFO, "Virtual particles: %s", *virtualParticles ? "true" : "false");
    }
}


void particlesTab(float *minParticleLifeTime, float *maxParticleLifeTime, float *minParticleSpeed, float *maxParticleSpeed, float *minParticleMass, float *maxParticleMass, float *trailLength)
{
    float screenWidth = (float)GetScreenWidth();
    float screenHeight = (float)GetScreenHeight();

    float xOffset = screenWidth * 0.05f;
    float yOffset = screenHeight * 0.1f;
    float controlHeight = screenHeight * 0.03f;
    float labelWidth = screenWidth * 0.25f;
    float controlWidth = screenWidth * 0.375f;
    float verticalSpacing = screenHeight * 0.05f;

    // Min Particle Lifetime
    yOffset += verticalSpacing;
    float minParticleLifeTimeTemp = *minParticleLifeTime;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Min Particle Lifetime");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &minParticleLifeTimeTemp, 1.0f, *maxParticleLifeTime - 1.0f);
    *minParticleLifeTime = minParticleLifeTimeTemp;
    int minParticleLifeTimeTempInt = (int)*minParticleLifeTime;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, &minParticleLifeTimeTempInt, 1, (int)(*maxParticleLifeTime - 1), false);
    *minParticleLifeTime = (float)minParticleLifeTimeTempInt;

    // Max Particle Lifetime
    yOffset += verticalSpacing;
    float maxParticleLifeTimeTemp = *maxParticleLifeTime;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Particle Lifetime");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &maxParticleLifeTimeTemp, *minParticleLifeTime + 1.0f, 500.0f);
    *maxParticleLifeTime = maxParticleLifeTimeTemp;
    int maxParticleLifeTimeTempInt = (int)*maxParticleLifeTime;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, &maxParticleLifeTimeTempInt, (int)(*minParticleLifeTime + 1), 500, false);
    *maxParticleLifeTime = (float)maxParticleLifeTimeTempInt;

    // Min Particle Speed
    yOffset += verticalSpacing;
    float minParticleSpeedTemp = *minParticleSpeed;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Min Particle Speed");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &minParticleSpeedTemp, 0.1f, *maxParticleSpeed - 0.1f);
    *minParticleSpeed = minParticleSpeedTemp;
    int minParticleSpeedTempInt = (int)*minParticleSpeed;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, &minParticleSpeedTempInt, 1, (int)(*maxParticleSpeed - 0.1f), false);
    *minParticleSpeed = (float)minParticleSpeedTempInt;

    // Max Particle Speed
    yOffset += verticalSpacing;
    float maxParticleSpeedTemp = *maxParticleSpeed;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Particle Speed");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &maxParticleSpeedTemp, *minParticleSpeed + 0.1f, 1000.0f);
    *maxParticleSpeed = maxParticleSpeedTemp;
    int maxParticleSpeedTempInt = (int)*maxParticleSpeed;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, &maxParticleSpeedTempInt, (int)(*minParticleSpeed + 0.1f), 1000, false);
    *maxParticleSpeed = (float)maxParticleSpeedTempInt;

    // Min Particle Mass
    yOffset += verticalSpacing;
    float minParticleMassTemp = *minParticleMass;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Min Particle Mass:");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &minParticleMassTemp, 0.1f, *maxParticleMass - 0.1f);
    *minParticleMass = minParticleMassTemp;
    int minParticleMassTempInt = (int)*minParticleMass;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, &minParticleMassTempInt, 1, (int)(*maxParticleMass - 0.1f), false);
    *minParticleMass = (float)minParticleMassTempInt;

    // Max Particle Mass
    yOffset += verticalSpacing;
    float maxParticleMassTemp = *maxParticleMass;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Particle Mass:");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &maxParticleMassTemp, *minParticleMass + 0.1f, 100.0f);
    *maxParticleMass = maxParticleMassTemp;
    int maxParticleMassTempInt = (int)*maxParticleMass;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, &maxParticleMassTempInt, (int)(*minParticleMass + 0.1f), 100, false);
    *maxParticleMass = (float)maxParticleMassTempInt;

    // Trail Length
    yOffset += verticalSpacing;
    float trailLengthTemp = *trailLength;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Trail Length");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &trailLengthTemp, 1.0f, 50.0f);
    *trailLength = trailLengthTemp;
    int trailLengthTempInt = (int)*trailLength;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, &trailLengthTempInt, 1, 50, false);
    *trailLength = (float)trailLengthTempInt;
}

void explosionTab(int *minExplosionParticles, int *maxExplosionParticles)
{
    float screenWidth = (float)GetScreenWidth();
    float screenHeight = (float)GetScreenHeight();

    float xOffset = screenWidth * 0.05f;
    float yOffset = screenHeight * 0.1f;
    float controlHeight = screenHeight * 0.03f;
    float labelWidth = screenWidth * 0.25f;
    float controlWidth = screenWidth * 0.375f;
    float verticalSpacing = screenHeight * 0.05f;

    // Min Explosion Particles
    yOffset += verticalSpacing;
    float minExplosionParticlesTemp = (float)(*minExplosionParticles);
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Min Explosion Particles");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &minExplosionParticlesTemp, 1.0f, (float)(*maxExplosionParticles - 1));
    *minExplosionParticles = (int)minExplosionParticlesTemp;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, minExplosionParticles, 1, (int)(*maxExplosionParticles - 1), false);

    // Max Explosion Particles
    yOffset += verticalSpacing;
    float maxExplosionParticlesTemp = (float)(*maxExplosionParticles);
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Explosion Particles");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &maxExplosionParticlesTemp, (float)(*minExplosionParticles + 0.1f), 100.0f);
    *maxExplosionParticles = (int)maxExplosionParticlesTemp;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, maxExplosionParticles, (int)(*minExplosionParticles + 0.1f), 100, false);
}

void virtualParticlesTab(int *minVirtualParticleSpeed, int *maxVirtualParticleSpeed, int *minVirtualParticleLifeTime, int *maxVirtualParticleLifeTime, float *minTimeBetweenVirtualPairs)
{
    float screenWidth = (float)GetScreenWidth();
    float screenHeight = (float)GetScreenHeight();

    float xOffset = screenWidth * 0.05f;
    float yOffset = screenHeight * 0.1f;
    float controlHeight = screenHeight * 0.03f;
    float labelWidth = screenWidth * 0.3f;
    float controlWidth = screenWidth * 0.375f;
    float verticalSpacing = screenHeight * 0.05f;

    // Min Virtual Particle Speed
    yOffset += verticalSpacing;
    float minVirtualSpeedTemp = (float)(*minVirtualParticleSpeed);
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Min Virtual Particle Speed");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &minVirtualSpeedTemp, 1.0f, (float)(*maxVirtualParticleSpeed - 1));
    *minVirtualParticleSpeed = (int)minVirtualSpeedTemp;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, minVirtualParticleSpeed, 1, (int)(*maxVirtualParticleSpeed - 1), false);

    // Max Virtual Particle Speed
    yOffset += verticalSpacing;
    float maxVirtualSpeedTemp = (float)(*maxVirtualParticleSpeed);
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Virtual Particle Speed");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &maxVirtualSpeedTemp, (float)(*minVirtualParticleSpeed + 1), 100.0f);
    *maxVirtualParticleSpeed = (int)maxVirtualSpeedTemp;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, maxVirtualParticleSpeed, (int)(*minVirtualParticleSpeed + 1), 100, false);

    // Min Virtual Particle Lifetime
    yOffset += verticalSpacing;
    float minLifetimeTemp = (float)(*minVirtualParticleLifeTime);
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Min Virtual Particle Lifetime");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &minLifetimeTemp, 1.0f, (float)(*maxVirtualParticleLifeTime - 1));
    *minVirtualParticleLifeTime = (int)minLifetimeTemp;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, minVirtualParticleLifeTime, 1, (int)(*maxVirtualParticleLifeTime - 1), false);

    // Max Virtual Particle Lifetime
    yOffset += verticalSpacing;
    float maxLifetimeTemp = (float)(*maxVirtualParticleLifeTime);
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Virtual Particle Lifetime");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &maxLifetimeTemp, (float)(*minVirtualParticleLifeTime + 1), 100.0f);
    *maxVirtualParticleLifeTime = (int)maxLifetimeTemp;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, maxVirtualParticleLifeTime, (int)(*minVirtualParticleLifeTime + 1), 100, false);

    // Min Time Between Virtual Pairs
    yOffset += verticalSpacing;
    float minTimeBetweenPairsTemp = *minTimeBetweenVirtualPairs;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Min Time Between Virtual Pairs");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &minTimeBetweenPairsTemp, 1.0f, 100.0f);
    *minTimeBetweenVirtualPairs = minTimeBetweenPairsTemp;
    int minTimeBetweenPairsTempInt = (int)*minTimeBetweenVirtualPairs;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, &minTimeBetweenPairsTempInt, 1, 100, false);
    *minTimeBetweenVirtualPairs = (float)minTimeBetweenPairsTempInt;
}

void physicsTab(float *g, float *gUniversal, float *maxGravityDistance, float *kElectric, float *maxForce, GravityType *gravityType, bool *electricForce, bool *dropDownOpen)
{
    float screenWidth = (float)GetScreenWidth();
    float screenHeight = (float)GetScreenHeight();

    float xOffset = screenWidth * 0.05f;
    float yOffset = screenHeight * 0.1f;
    float controlHeight = screenHeight * 0.03f;
    float labelWidth = screenWidth * 0.25f;
    float controlWidth = screenWidth * 0.375f;
    float verticalSpacing = screenHeight * 0.05f;

    // Gravitational Acceleration (g)
    yOffset += verticalSpacing;
    float gTemp = *g;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Gravitational Acceleration");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &gTemp, 1.0f, 500.0f);
    *g = gTemp;
    int gTempInt = (int)*g;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, &gTempInt, 1, 500, false);
    *g = (float)gTempInt;

    // Universal Gravitational Constant (gUniversal)
    yOffset += verticalSpacing;
    float gUniversalTemp = *gUniversal;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Universal Gravitational Constant");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &gUniversalTemp, -100.0f, 1000.0f);
    *gUniversal = gUniversalTemp;
    int gUniversalTempInt = (int)(*gUniversal);
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, &gUniversalTempInt, -100, 1000, false);
    *gUniversal = (float)gUniversalTempInt;

    // Max Gravity Distance
    yOffset += verticalSpacing;
    float maxGravityDistanceTemp = *maxGravityDistance;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Gravity Distance:");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &maxGravityDistanceTemp, 1.0f, 1000.0f);
    *maxGravityDistance = maxGravityDistanceTemp;
    int maxGravityDistanceTempInt = (int)(*maxGravityDistance);
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, &maxGravityDistanceTempInt, 1, 1000, false);
    *maxGravityDistance = (float)maxGravityDistanceTempInt;

    // Electrical Constant (kElectric)
    yOffset += verticalSpacing;
    float kElectricTemp = *kElectric;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Electrical Constant");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &kElectricTemp, 1.0f, 1000.0f);
    *kElectric = kElectricTemp;
    int kElectricTempInt = (int)(*kElectric);
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, &kElectricTempInt, 1, 1000, false);
    *kElectric = (float)kElectricTempInt;

    // Enable Electric Force
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Enable Electric Force:");
    GuiCheckBox((Rectangle){xOffset + labelWidth, yOffset, controlHeight, controlHeight}, "Electric Force", electricForce);

    // Max Force
    yOffset += verticalSpacing;
    float maxForceTemp = *maxForce;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Force Allowed");
    GuiSlider((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, NULL, &maxForceTemp, 1.0f, 1000.0f);
    *maxForce = maxForceTemp;
    int maxForceTempInt = (int)*maxForce;
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, &maxForceTempInt, 1, 1000, false);
    *maxForce = (float)maxForceTempInt;

    // Gravity Type
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Gravity Type:");
    if (GuiDropdownBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight},
                       "None;Center;Down;Up;Left;Right", (int *)gravityType, *dropDownOpen))
    {
        *dropDownOpen = !*dropDownOpen;
    }
}