#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "raygui.h"
#include "gui.h"

void generalTab(GeneralTabParameters *gtp)
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
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Maximum particles");
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, gtp->maxParticles.value, 0, 10000, true);

    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Initial Particles");
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, gtp->initialCapacity.value, 0, 10000, true);

    yOffset += verticalSpacing;
    if (GuiButton((Rectangle){xOffset, yOffset, labelWidth * 0.75f, controlHeight}, "Set Seed"))
    {
        gtp->seed = GetRandomValue(0, 10000);
        SetRandomSeed(gtp->seed);
        TraceLog(LOG_INFO, "Random seed set");
    }
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth * 0.4f, controlHeight}, NULL, &gtp->seed, 0, 10000, false);

    yOffset += verticalSpacing;
    if (GuiCheckBox((Rectangle){xOffset, yOffset, controlHeight, controlHeight}, "Particles Die", gtp->lifetime.value))
    {
        TraceLog(LOG_INFO, "Lifetime: %s", *gtp->lifetime.value ? "true" : "false");
    }

    yOffset += verticalSpacing;
    if (GuiCheckBox((Rectangle){xOffset, yOffset, controlHeight, controlHeight}, "Fragment particles live", gtp->fragmentParticlesLive.value))
    {
        TraceLog(LOG_INFO, "Fragment particles live: %s", *gtp->fragmentParticlesLive.value ? "true" : "false");
    }

    yOffset += verticalSpacing;
    if (GuiCheckBox((Rectangle){xOffset, yOffset, controlHeight, controlHeight}, "Virtual particles", gtp->virtualParticles.value))
    {
        TraceLog(LOG_INFO, "Virtual particles: %s", *gtp->virtualParticles.value ? "true" : "false");
    }
}

void particlesTab(ParticlesTabParameters *ptp)
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
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Min Particle Lifetime");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        ptp->minParticleLifeTime.textValue,
        ptp->minParticleLifeTime.value
    );

    // Max Particle Lifetime
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Particle Lifetime");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        ptp->maxParticleLifeTime.textValue,
        ptp->maxParticleLifeTime.value
    );

    // Min Particle Speed
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Min Particle Speed");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        ptp->minParticleSpeed.textValue,
        ptp->minParticleSpeed.value
    );

    // Max Particle Speed
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Particle Speed");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        ptp->maxParticleSpeed.textValue,
        ptp->maxParticleSpeed.value
    );

    // Min Particle Mass
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Min Particle Mass");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        ptp->minParticleMass.textValue,
        ptp->minParticleMass.value
    );

    // Max Particle Mass
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Particle Mass");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        ptp->maxParticleMass.textValue,
        ptp->maxParticleMass.value
    );

    // Trail Length (IntValue)
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Trail Length");
    GuiValueBox(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        ptp->trailLength.value,
        0,
        5,
        true
    );
}

void explosionTab(ExplosionTabParameters *etp)
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
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Min Explosion Particles");
    GuiValueBox(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        etp->minExplosionParticles.value,
        0,
        5,
        true
    );

    // Max Explosion Particles
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Explosion Particles");
    GuiValueBox(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        etp->maxExplosionParticles.value,
        0,
        5,
        true
    );
}

void virtualParticlesTab(VirtualParticlesTabParameters *vtp)
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
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Min Virtual Particle Speed");
    GuiValueBox(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        vtp->minVirtualParticleSpeed.value,
        1,
        100,
        true
    );

    // Max Virtual Particle Speed
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Virtual Particle Speed");
    GuiValueBox(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        vtp->maxVirtualParticleSpeed.value,
        1,
        100,
        true
    );

    // Min Virtual Particle Lifetime
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Min Virtual Particle Lifetime");
    GuiValueBox(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        vtp->minVirtualParticleLifeTime.value,
        1,
        100,
        true
    );

    // Max Virtual Particle Lifetime
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Virtual Particle Lifetime");
    GuiValueBox(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        vtp->maxVirtualParticleLifeTime.value,
        1,
        500,
        true
    );

    // Min Time Between Virtual Pairs
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Min Time Between Virtual Pairs");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        vtp->minTimeBetweenVirtualPairs.textValue,
        vtp->minTimeBetweenVirtualPairs.value
    );
}

void physicsTab(PhysicsTabParameters *phtp)
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
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Gravitational Acceleration");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        phtp->g.textValue,
        phtp->g.value
    );

    // Universal Gravitational Constant (gUniversal)
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Universal Gravitational Constant");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        phtp->gUniversal.textValue,
        phtp->gUniversal.value
    );

    // Max Gravity Distance
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Gravity Distance:");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        phtp->maxGravityDistance.textValue,
        phtp->maxGravityDistance.value
    );

    // Electrical Constant (kElectric)
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Electrical Constant");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        phtp->kElectric.textValue,
        phtp->kElectric.value
    );

    // Max Force
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Force Allowed");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        phtp->maxForce.textValue,
        phtp->maxForce.value
    );

    // Gravity Type
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Gravity Type:");
    if (GuiDropdownBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight},
                       "None;Center;Down;Up;Left;Right", (int *)phtp->gravityType, phtp->dropDownOpen))
    {
        phtp->dropDownOpen = !phtp->dropDownOpen;
    }

    // Enable Electric Force
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Electric Force:");
    GuiCheckBox((Rectangle){xOffset + labelWidth, yOffset, controlHeight, controlHeight}, NULL, phtp->electricForce.value);
}