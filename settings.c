#include "simulation.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define MAX_TABS 5
#define WIDTH 800
#define HEIGHT 800

void generalTab(int *maxParticles, int *initialCapacity, int *seed, bool *lifetime, bool *fragmentParticlesLive, bool *virtualParticles)
{
    int xOffset = 40;
    int yOffset = 80;

    yOffset += 40;
    float maxParticlesTemp = (float)(*maxParticles);
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Maximum particles");
    GuiSlider((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, NULL, &maxParticlesTemp, 1.0f, 5000.0f);
    *maxParticles = (int)maxParticlesTemp;
    GuiValueBox((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, maxParticles, 1, 5000, false);

    yOffset += 40;
    float initialCapacityTemp = (float)(*initialCapacity);
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Initial Particles");
    GuiSlider((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, NULL, &initialCapacityTemp, 1.0f, 500.0f);
    *initialCapacity = (int)initialCapacityTemp;
    GuiValueBox((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, initialCapacity, 1, 500, false);

    yOffset += 40;
    if (GuiButton((Rectangle){xOffset, yOffset, 150, 20}, "Set Seed"))
    {
        *seed = GetRandomValue(0, 10000);
        SetRandomSeed(*seed);
        TraceLog(LOG_INFO, "Random seed set");
    }
    GuiValueBox((Rectangle){xOffset + 200, yOffset, 120, 20}, NULL, seed, 0, 10000, false);

    yOffset += 40;
    if (GuiCheckBox((Rectangle){xOffset, yOffset, 20, 20}, "Lifetime", lifetime))
    {
        TraceLog(LOG_INFO, "Lifetime: %s", *lifetime ? "true" : "false");
    }

    yOffset += 40;
    if (GuiCheckBox((Rectangle){xOffset, yOffset, 20, 20}, "Fragment particles live", fragmentParticlesLive))
    {
        TraceLog(LOG_INFO, "Fragment particles live: %s", *fragmentParticlesLive ? "true" : "false");
    }

    yOffset += 40;
    if (GuiCheckBox((Rectangle){xOffset, yOffset, 20, 20}, "Virtual particles", virtualParticles))
    {
        TraceLog(LOG_INFO, "Virtual particles: %s", *virtualParticles ? "true" : "false");
    }
}

void particlesTab(float *minParticleLifeTime, float *maxParticleLifeTime, float *minParticleSpeed, float *maxParticleSpeed, float *minParticleMass, float *maxParticleMass, float *trailLength)
{
    int xOffset = 40;
    int yOffset = 80;

    yOffset += 40;

    float minParticleLifeTimeTemp = *minParticleLifeTime;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Min Particle Lifetime");
    GuiSlider((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, NULL, &minParticleLifeTimeTemp, 1.0f, *maxParticleLifeTime - 1.0f);
    *minParticleLifeTime = minParticleLifeTimeTemp;
    int minParticleLifeTimeTempInt = (int)*minParticleLifeTime;
    GuiValueBox((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, &minParticleLifeTimeTempInt, 1, *maxParticleLifeTime - 1, false);
    *minParticleLifeTime = (float)minParticleLifeTimeTempInt;

    yOffset += 40;
    float maxParticleLifeTimeTemp = *maxParticleLifeTime;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Max Particle Lifetime");
    GuiSlider((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, NULL, &maxParticleLifeTimeTemp, *minParticleLifeTime + 1.0f, 500.0f);
    *maxParticleLifeTime = maxParticleLifeTimeTemp;
    int maxParticleLifeTimeTempInt = (int)*maxParticleLifeTime;
    GuiValueBox((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, &maxParticleLifeTimeTempInt, *minParticleLifeTime + 1, 500, false);
    *maxParticleLifeTime = (float)maxParticleLifeTimeTempInt;

    yOffset += 40;
    float minParticleSpeedTemp = *minParticleSpeed;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Min Particle Speed");
    GuiSlider((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, NULL, &minParticleSpeedTemp, 0.1f, *maxParticleSpeed - 0.1f);
    *minParticleSpeed = minParticleSpeedTemp;
    int minParticleSpeedTempInt = (int)*minParticleSpeed;
    GuiValueBox((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, &minParticleSpeedTempInt, 1, (int)(*maxParticleSpeed - 0.1f), false);
    *minParticleSpeed = (float)minParticleSpeedTempInt;

    yOffset += 40;
    float maxParticleSpeedTemp = *maxParticleSpeed;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Max Particle Speed");
    GuiSlider((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, NULL, &maxParticleSpeedTemp, *minParticleSpeed + 0.1f, 1000.0f);
    *maxParticleSpeed = maxParticleSpeedTemp;
    int maxParticleSpeedTempInt = (int)*maxParticleSpeed;
    GuiValueBox((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, &maxParticleSpeedTempInt, (int)(*minParticleSpeed + 0.1f), 1000, false);
    *maxParticleSpeed = (float)maxParticleSpeedTempInt;

    yOffset += 40;
    float minParticleMassTemp = *minParticleMass;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Min Particle Mass:");
    GuiSlider((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, NULL, &minParticleMassTemp, 0.1f, *maxParticleMass - 0.1f);
    *minParticleMass = minParticleMassTemp;
    int minParticleMassTempInt = (int)*minParticleMass;
    GuiValueBox((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, &minParticleMassTempInt, 1, (int)(*maxParticleMass - 0.1f), false);
    *minParticleMass = (float)minParticleMassTempInt;

    yOffset += 40;
    float maxParticleMassTemp = *maxParticleMass;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Max Particle Mass:");
    GuiSlider((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, NULL, &maxParticleMassTemp, *minParticleMass + 0.1f, 100.0f);
    *maxParticleMass = maxParticleMassTemp;
    int maxParticleMassTempInt = (int)*maxParticleMass;
    GuiValueBox((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, &maxParticleMassTempInt, (int)(*minParticleMass + 0.1f), 100, false);
    *maxParticleMass = (float)maxParticleMassTempInt;

    yOffset += 40;
    float trailLengthTemp = *trailLength;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Trail Length");
    GuiSlider((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, NULL, &trailLengthTemp, 1.0f, 50.0f);
    *trailLength = trailLengthTemp;
    int trailLengthTempInt = (int)*trailLength;
    GuiValueBox((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, &trailLengthTempInt, 1, 50, false);
    *trailLength = (float)trailLengthTempInt;
}

void explosionTab(int *minExplosionParticles, int *maxExplosionParticles)
{
    int xOffset = 40;
    int yOffset = 80;

    // Minimum Explosion Particles
    yOffset += 40;
    float minExplosionParticlesTemp = (float)(*minExplosionParticles);
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Min Explosion Particles");
    GuiSlider((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, NULL, &minExplosionParticlesTemp, 1.0f, *maxExplosionParticles - 1.0f);
    *minExplosionParticles = (int)minExplosionParticlesTemp;
    GuiValueBox((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, minExplosionParticles, 1, (int)(*maxExplosionParticles - 1.0f), false);

    // Maximum Explosion Particles
    yOffset += 40;
    float maxExplosionParticlesTemp = (float)(*maxExplosionParticles);
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Max Explosion Particles");
    GuiSlider((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, NULL, &maxExplosionParticlesTemp, *minExplosionParticles + 0.1f, 100.0f);
    *maxExplosionParticles = (int)maxExplosionParticlesTemp;
    GuiValueBox((Rectangle){xOffset + 200, yOffset, 300, 20}, NULL, maxExplosionParticles, (int)(*minExplosionParticles + 0.1f), 100, false);
}

void virtualParticlesTab(int *minVirtualParticleSpeed, int *maxVirtualParticleSpeed, int *minVirtualParticleLifeTime, int *maxVirtualParticleLifeTime, float *minTimeBetweenVirtualPairs)
{
    int xOffset = 40;
    int yOffset = 80;

    // Minimum Virtual Particle Speed
    yOffset += 40;
    float minVirtualSpeedTemp = (float)(*minVirtualParticleSpeed);
    GuiLabel((Rectangle){xOffset, yOffset, 250, 20}, "Min Virtual Particle Speed");
    GuiSlider((Rectangle){xOffset + 300, yOffset, 300, 20}, NULL, NULL, &minVirtualSpeedTemp, 1.0f, (float)(*maxVirtualParticleSpeed - 1));
    *minVirtualParticleSpeed = (int)minVirtualSpeedTemp;
    GuiValueBox((Rectangle){xOffset + 300, yOffset, 300, 20}, NULL, minVirtualParticleSpeed, 1, (int)(*maxVirtualParticleSpeed - 1), false);

    // Maximum Virtual Particle Speed
    yOffset += 40;
    float maxVirtualSpeedTemp = (float)(*maxVirtualParticleSpeed);
    GuiLabel((Rectangle){xOffset, yOffset, 250, 20}, "Max Virtual Particle Speed");
    GuiSlider((Rectangle){xOffset + 300, yOffset, 300, 20}, NULL, NULL, &maxVirtualSpeedTemp, (float)(*minVirtualParticleSpeed + 1), 100.0f);
    *maxVirtualParticleSpeed = (int)maxVirtualSpeedTemp;
    GuiValueBox((Rectangle){xOffset + 300, yOffset, 300, 20}, NULL, maxVirtualParticleSpeed, (int)(*minVirtualParticleSpeed + 1), 100, false);

    // Minimum Virtual Particle Lifetime
    yOffset += 40;
    float minLifetimeTemp = (float)(*minVirtualParticleLifeTime);
    GuiLabel((Rectangle){xOffset, yOffset, 250, 20}, "Min Virtual Particle Lifetime");
    GuiSlider((Rectangle){xOffset + 300, yOffset, 300, 20}, NULL, NULL, &minLifetimeTemp, 1.0f, (float)(*maxVirtualParticleLifeTime - 1));
    *minVirtualParticleLifeTime = (int)minLifetimeTemp;
    GuiValueBox((Rectangle){xOffset + 300, yOffset, 300, 20}, NULL, minVirtualParticleLifeTime, 1, (int)(*maxVirtualParticleLifeTime - 1), false);

    // Maximum Virtual Particle Lifetime
    yOffset += 40;
    float maxLifetimeTemp = (float)(*maxVirtualParticleLifeTime);
    GuiLabel((Rectangle){xOffset, yOffset, 250, 20}, "Max Virtual Particle Lifetime");
    GuiSlider((Rectangle){xOffset + 300, yOffset, 300, 20}, NULL, NULL, &maxLifetimeTemp, (float)(*minVirtualParticleLifeTime + 1), 100.0f);
    *maxVirtualParticleLifeTime = (int)maxLifetimeTemp;
    GuiValueBox((Rectangle){xOffset + 300, yOffset, 300, 20}, NULL, maxVirtualParticleLifeTime, (int)(*minVirtualParticleLifeTime + 1), 100, false);

    // Minimum Time Between Virtual Pairs
    yOffset += 40;
    float minTimeBetweenPairsTemp = *minTimeBetweenVirtualPairs;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Min Particle Lifetime");
    GuiSlider((Rectangle){xOffset + 300, yOffset, 300, 20}, NULL, NULL, &minTimeBetweenPairsTemp, 1.0f, 100.0f);
    *minTimeBetweenVirtualPairs = minTimeBetweenPairsTemp;
    int minTimeBetweenPairsTempInt = (int)*minTimeBetweenVirtualPairs;
    GuiValueBox((Rectangle){xOffset + 300, yOffset, 300, 20}, NULL, &minTimeBetweenPairsTempInt, 1, 100, false);
    *minTimeBetweenVirtualPairs = (float)minTimeBetweenPairsTempInt;
}

void physicsTab(float *g, float *gUniversal, float *maxGravityDistance, float *kElectric, float *maxForce, GravityType *gravityType, bool *electricForce, bool *dropDownOpen)
{
    int xOffset = 40;
    int yOffset = 80;

    // Standard gravity (G)
    yOffset += 40;
    float gTemp = *g;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Gravitational\nAcceleration");
    GuiSlider((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, NULL, &gTemp, 1.0f, 500.0f);
    *g = gTemp;
    int gTempInt = (int)*g;
    GuiValueBox((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, &gTempInt, 1, 500, false);

    // Universal Gravitational Constant (G_UNIVERSAL)
    yOffset += 40;
    float gUniversalTemp = *gUniversal;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Universal\nGravitational Constant");
    GuiSlider((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, NULL, &gUniversalTemp, -100.0f, 1000.0f);
    *gUniversal = gUniversalTemp;
    int gUniversalTempInt = (int)(*gUniversal);
    GuiValueBox((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, &gUniversalTempInt, -100, 1000, false);

    // Maximum gravity distance
    yOffset += 40;
    float maxGravityDistanceTemp = *maxGravityDistance;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Max Gravity Distance:");
    GuiSlider((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, NULL, &maxGravityDistanceTemp, 1.0f, 1000.0f);
    *maxGravityDistance = maxGravityDistanceTemp;
    int maxGravityDistanceTempInt = (int)(*maxGravityDistance);
    GuiValueBox((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, &maxGravityDistanceTempInt, 1, 1000, false);

    // Eletric force constant (K_ELECTRIC)
    yOffset += 40;
    float kElectricTemp = *kElectric;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Electrical Constant");
    GuiSlider((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, NULL, &kElectricTemp, 1.0f, 1000.0f);
    *kElectric = kElectricTemp;
    int kElectricTempInt = (int)(*kElectric);
    GuiValueBox((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, &kElectricTempInt, 1, 1000, false);

    // Enable/disable electric force
    yOffset += 40;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Enable Electric Force:");
    GuiCheckBox((Rectangle){xOffset + 250, yOffset, 20, 20}, "Electric Force", electricForce);

    // Maximum force allowed (MAX_FORCE)
    yOffset += 40;
    float maxForceTemp = *maxForce;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Max Force Allowed");
    GuiSlider((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, NULL, &maxForceTemp, 1.0f, 1000.0f);
    *maxForce = maxForceTemp;
    int maxForceTempInt = (int)(*maxForce);
    GuiValueBox((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, &maxForceTempInt, 1, 1000, false);

    // gravity Type
    yOffset += 40;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Gravity Type:");
    if (GuiDropdownBox((Rectangle){xOffset + 250, yOffset, 300, 20},
                       "None;Center;Down;Up;Left;Right", (int *)gravityType, *dropDownOpen))
    {
        *dropDownOpen = !*dropDownOpen;
    }
}

int main(void)
{
    InitWindow(WIDTH, HEIGHT, "Particle Simulation");
    SetTargetFPS(60);

    bool showConfigPanel = false;
    bool paused = false;

    int screenWidth = GetScreenWidth();
    float scaleFactor = (float)screenWidth / WIDTH;
    int baseFontSize = 20;
    int scaledFontSize = (int)(baseFontSize * scaleFactor);

    int activeTab = 0;

    SimulationConfig config = {
        // GENERAL
        .maxParticles = 100,
        .initialCapacity = 10,
        .lifetime = false,
        .fragmentParticlesLive = false,
        .virtualParticles = false,

        // PARTICLES
        .minParticleLifeTime = 10,
        .maxParticleLifeTime = 50,
        .minParticleSpeed = 10,
        .maxParticleSpeed = 20,
        .minParticleMass = 5,
        .maxParticleMass = 10,
        .trailLength = 5,

        // EXPLOSION
        .minExplosionParticles = 0,
        .maxExplosionParticles = 5,

        // VIRTUAL PARTICLE
        .minVirtualParticleSpeed = 5,
        .maxVirtualParticleSpeed = 25,
        .minVirtualParticleLifeTime = 5,
        .maxVirtualParticleLifeTime = 25,
        .minTimeBetweenVirtualPairs = 5.0f,

        // PHYSICS
        .g = 9.81f,
        .gUniversal = 10,
        .maxGravityDistance = 100.0f,
        .kElectric = 50.0f,
        .maxForce = 100.0f,
        .gravityType = GRAVITY_NONE,
        .electricForce = false
    };

    Particles particles = {
        .items = (Particle *)malloc(config.maxParticles * sizeof(Particle)),
        .count = 0,
        .capacity = config.initialCapacity,
    };
    
    InitGrid(&config);
    InitParticles(&config, &particles);
    Shader glowShader = LoadShader(0, "glow.fs");
    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

    bool dropDownOpen = true;
    const char *tabTexts[MAX_TABS] = {"General", "Particles", "Explosions", "Virtual Particles", "Physics"};

    int xOffset = 20;
    int yOffset = 20;

    int seed = 0;

    RenderTexture2D simulationTexture = LoadRenderTexture(WIDTH, HEIGHT);
    Rectangle pauseBtnRect =   (Rectangle){10, HEIGHT - 40, 100, 30};
    Rectangle resumeBtnRect =  (Rectangle){120, HEIGHT - 40, 100, 30};
    Rectangle resetBtnRect =   (Rectangle){230, HEIGHT - 40, 100, 30};
    Rectangle settingsBtnRect = (Rectangle){WIDTH - 150, HEIGHT - 40, 130, 30};

    while (!WindowShouldClose())
    {
        float delta = GetFrameTime();
        GuiSetStyle(DEFAULT, TEXT_SIZE, scaledFontSize);
        Vector2 mousePos = GetMousePosition();
        Rectangle buttonRect = (Rectangle){WIDTH - 150, HEIGHT - 50, 130, 30};

        bool mouseOverButton = CheckCollisionPointRec(mousePos, buttonRect);
        bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        bool overPause   = CheckCollisionPointRec(mousePos, pauseBtnRect);
        bool overResume  = CheckCollisionPointRec(mousePos, resumeBtnRect);
        bool overReset   = CheckCollisionPointRec(mousePos, resetBtnRect);
        bool overSetting = CheckCollisionPointRec(mousePos, settingsBtnRect);

        if (mouseClicked)
        {
            if (overPause) {
                paused = true;
            }
            else if (overResume) {
                paused = false;
            }
            else if (overReset) {
                ResetParticles(&config, &particles);
            }
            else if (overSetting) {
                showConfigPanel = showConfigPanel ? false : true;
            }
        }

        if (!showConfigPanel && !paused)
        {
            HandleInput(&config, &particles);

            if (config.virtualParticles)
            {
                GenerateVirtualParticles(&config, &particles, delta);
            }

            UpdateSimulation(&config, &particles, &target);
        }

        BeginTextureMode(simulationTexture);
            ClearBackground(RAYWHITE);

            if (!showConfigPanel)
            {
                BeginScissorMode(0, 0, WIDTH, HEIGHT - 50);
                RenderSimulation(glowShader, target);
                EndScissorMode();
            }
            else
            {
                ClearBackground(RAYWHITE);
                Rectangle tabBarBounds = {xOffset, yOffset, 760, 40};
                int closedTab = GuiTabBar(tabBarBounds, tabTexts, MAX_TABS, &activeTab);

                if (closedTab >= 0)
                {
                    TraceLog(LOG_INFO, "Pestaña cerrada: %d", closedTab);
                }

                if (activeTab == 0)
                {
                    generalTab(&config.maxParticles, &config.initialCapacity, &seed, &config.lifetime, &config.fragmentParticlesLive, &config.virtualParticles);
                }
                else if (activeTab == 1)
                {
                    particlesTab(&config.minParticleLifeTime, &config.maxParticleLifeTime, &config.minParticleSpeed, &config.maxParticleSpeed, &config.minParticleMass, &config.maxParticleMass, &config.trailLength);
                }
                else if (activeTab == 2)
                {
                    explosionTab(&config.minExplosionParticles, &config.maxExplosionParticles);
                }
                else if (activeTab == 3)
                {
                    virtualParticlesTab(&config.minVirtualParticleSpeed, &config.maxVirtualParticleSpeed, &config.minVirtualParticleLifeTime, &config.maxVirtualParticleLifeTime, &config.minTimeBetweenVirtualPairs);
                }
                else if (activeTab == 4)
                {
                    physicsTab(&config.g, &config.gUniversal, &config.maxGravityDistance, &config.kElectric, &config.maxForce, &config.gravityType, &config.electricForce, &dropDownOpen);
                }
            }
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTextureRec(simulationTexture.texture, (Rectangle){0, 0, (float)simulationTexture.texture.width, (float)-simulationTexture.texture.height}, (Vector2){0,0}, WHITE);
            DrawRectangle(0, HEIGHT - 50, WIDTH, 50, (Color){200, 200, 200, 255});
            if (GuiButton(pauseBtnRect, "Pause")) {}
            if (GuiButton(resumeBtnRect, "Resume")) {}
            if (GuiButton(resetBtnRect, "Reset")) {}
            if (GuiButton(settingsBtnRect, "Settings")) {}
        EndDrawing();
    }

    CleanupSimulation(&particles, glowShader, target);
    UnloadRenderTexture(simulationTexture);
    CloseWindow();

    return 0;
}