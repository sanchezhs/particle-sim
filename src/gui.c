#include "../external/raylib/src/raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raygui.h"
#include "gui.h"

void DrawTooltip(const char *tooltipText)
{
    float padding = 8.0f;
    Vector2 mousePosition = GetMousePosition();
    float tooltipWidth = (float)strlen(tooltipText) * 2 * padding;
    float tooltipHeight = GuiGetStyle(DEFAULT, TEXT_SIZE) + 2 * padding;
    float tooltipX = mousePosition.x;
    float tooltipY = mousePosition.y - tooltipHeight - 5.0f;

    if (tooltipX + tooltipWidth > GetScreenWidth()) tooltipX -= (tooltipX + tooltipWidth) - GetScreenWidth();
    if (tooltipY < 0) tooltipY = mousePosition.y + 5.0f;

    DrawRectangleRounded((Rectangle){tooltipX, tooltipY, tooltipWidth, tooltipHeight}, 0.25f, 1, LIGHTGRAY);

    GuiLabel((Rectangle){tooltipX + padding, tooltipY + padding, tooltipWidth - 2 * padding, tooltipHeight - 2 * padding},
                tooltipText);
}

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
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){xOffset, yOffset, labelWidth, controlHeight}))
    {
        DrawTooltip("The maximum number of particles that can be simulated");
    }

    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Initial Particles");
    GuiValueBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, gtp->initialCapacity.value, 0, 10000, true);

    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Friction");
    GuiValueBoxFloat((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, NULL, gtp->friction.textValue, gtp->friction.value);
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){xOffset, yOffset, labelWidth, controlHeight}))
    {
        DrawTooltip("Particle's speed can increase, decrease or stay the same");
    }

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
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){xOffset, yOffset, controlHeight, controlHeight}))
    {
        DrawTooltip("Enable if particles should have a limited lifetime");
    }

    yOffset += verticalSpacing;
    if (GuiCheckBox((Rectangle){xOffset, yOffset, controlHeight, controlHeight}, "Fragment particles live", gtp->fragmentParticlesLive.value))
    {
        TraceLog(LOG_INFO, "Fragment particles live: %s", *gtp->fragmentParticlesLive.value ? "true" : "false");
    }
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){xOffset, yOffset, controlHeight, controlHeight}))
    {
        DrawTooltip("Enable if fragments should have their own lifetime");
    }

    yOffset += verticalSpacing;
    if (GuiCheckBox((Rectangle){xOffset, yOffset, controlHeight, controlHeight}, "Virtual particles", gtp->virtualParticles.value))
    {
        TraceLog(LOG_INFO, "Virtual particles: %s", *gtp->virtualParticles.value ? "true" : "false");
    }
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){xOffset, yOffset, controlHeight, controlHeight}))
    {
        DrawTooltip("Enable if pairs of virtual particles can be generated");
    }
}

void patternsTab(PatternsTabParameters *pstp)
{
    float screenWidth = (float)GetScreenWidth();
    float screenHeight = (float)GetScreenHeight();

    float xOffset = screenWidth * 0.05f;
    float yOffset = screenHeight * 0.1f;
    float controlHeight = screenHeight * 0.03f;
    float labelWidth = screenWidth * 0.25f;
    float controlWidth = screenWidth * 0.375f;
    float verticalSpacing = screenHeight * 0.05f;

    // Description
    GuiLabel((Rectangle){xOffset, yOffset, screenWidth * 0.5f, controlHeight}, "Particle pattern parameters");
    GuiLabel((Rectangle){xOffset, yOffset += (verticalSpacing*0.5f), screenWidth * 0.85f, controlHeight}, "Controls how particles are initially placed and how they move afterwards");

    // Initial Pattern
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Pattern");
    if (GuiDropdownBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight},
                       "Random;Vortex;Group", (int *)pstp->initialPattern, pstp->dropDownOpen))
    {
        pstp->dropDownOpen = !pstp->dropDownOpen;
    }

    // Vortex Center
    // TODO

    // Vortex Strength
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Vortex Strength");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight},
        NULL,
        pstp->vortexStrength.textValue,
        pstp->vortexStrength.value
    );

    // Radial Strength
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Vortex Radial Strength");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight},
        NULL,
        pstp->radialStrength.textValue,
        pstp->radialStrength.value
    );

    // Vortex radius
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Vortex Radius");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight},
        NULL,
        pstp->vortexRadious.textValue,
        pstp->vortexRadious.value
    );
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

    // Description
    GuiLabel((Rectangle){xOffset, yOffset, screenWidth * 0.5f, controlHeight}, "Particle parameters");

    // Min Particle Lifetime
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Mininum Lifetime");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        ptp->minParticleLifeTime.textValue,
        ptp->minParticleLifeTime.value
    );

    // Max Particle Lifetime
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Maximum Lifetime");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        ptp->maxParticleLifeTime.textValue,
        ptp->maxParticleLifeTime.value
    );

    // Min Particle Speed
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Mininum Speed");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        ptp->minParticleSpeed.textValue,
        ptp->minParticleSpeed.value
    );

    // Max Particle Speed
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Maximum Speed");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        ptp->maxParticleSpeed.textValue,
        ptp->maxParticleSpeed.value
    );

    // Min Particle Mass
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Mininum Mass");
    GuiValueBoxFloat(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        ptp->minParticleMass.textValue,
        ptp->minParticleMass.value
    );

    // Max Particle Mass
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Maximum Mass");
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

    // Description
    GuiLabel((Rectangle){xOffset, yOffset, screenWidth * 0.5f, controlHeight}, "Particle explosion parameters");
    yOffset += verticalSpacing*0.5f;
    GuiLabel((Rectangle){xOffset, yOffset, screenWidth * 0.5f, controlHeight}, "Requires Particles Die to be enabled");
    
    // Min Explosion Particles
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Mininum Particles");
    GuiValueBox(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        etp->minExplosionParticles.value,
        0,
        5,
        true
    );
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){xOffset, yOffset, labelWidth, controlHeight}))
    {
        DrawTooltip("The minimum number of particles created when a particle dies");
    }

    // Max Explosion Particles
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Maximum Particles");
    GuiValueBox(
        (Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight}, 
        NULL, 
        etp->maxExplosionParticles.value,
        0,
        5,
        true
    );
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){xOffset, yOffset, labelWidth, controlHeight}))
    {
        DrawTooltip("The maximum number of particles created when a particle dies");
    }

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

    GuiLabel((Rectangle){xOffset, yOffset, screenWidth * 0.5f, controlHeight}, "Virtual particles parameters");
    yOffset += verticalSpacing*0.5f;
    GuiLabel((Rectangle){xOffset, yOffset, screenWidth * 0.5f, controlHeight}, "Requires Virtual Particles to be enabled");

    // Min Virtual Particle Speed
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Minimum Speed");
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
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Maximum Speed");
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
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Mininum Lifetime");
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
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Maximum Lifetime");
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
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Time between virtual particles");
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

    GuiLabel((Rectangle){xOffset, yOffset, screenWidth * 0.5f, controlHeight}, "Simulation physics parameters");
    yOffset += verticalSpacing*0.5f;
    GuiLabel((Rectangle){xOffset, yOffset, screenWidth * 0.8f, controlHeight}, "relevant forces and constants affecting the particles");

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
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Max Gravity Distance");
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
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Gravity Type");
    if (GuiDropdownBox((Rectangle){xOffset + labelWidth, yOffset, controlWidth, controlHeight},
                       "None;Center;Down;Up;Left;Right", (int *)phtp->gravityType, phtp->dropDownOpen))
    {
        phtp->dropDownOpen = !phtp->dropDownOpen;
    }

    // Enable Electric Force
    yOffset += verticalSpacing;
    GuiLabel((Rectangle){xOffset, yOffset, labelWidth, controlHeight}, "Electric Force");
    GuiCheckBox((Rectangle){xOffset + labelWidth, yOffset, controlHeight, controlHeight}, NULL, phtp->electricForce.value);
}
