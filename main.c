#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "simulation.h"
#include "gui.h"
#include "gui.h"

#define WIDTH 800
#define HEIGHT 800

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