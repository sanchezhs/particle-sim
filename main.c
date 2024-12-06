#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "simulation.h"
#include "gui.h"

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GetScreenWidth(), GetScreenHeight(), "Particle Simulation");
    SetTargetFPS(60);

    bool showConfigPanel = false;
    bool paused = false;

    int screenWidth = GetScreenWidth();
    float scaleFactor = (float)screenWidth / 800;
    int baseFontSize = 10;
    int scaledFontSize = (int)(baseFontSize * scaleFactor);
    float gridWidth = ((GetScreenWidth() + CELL_SIZE - 1) / CELL_SIZE);
    float gridHeight = ((GetScreenHeight() + CELL_SIZE - 1) / CELL_SIZE);

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
        .electricForce = false};

    Particles particles = {
        .items = (Particle *)malloc(config.maxParticles * sizeof(Particle)),
        .count = 0,
        .capacity = config.initialCapacity,
    };

    InitGrid(&config, gridWidth, gridHeight);
    InitParticles(&config, &particles, GetScreenWidth(), GetScreenHeight());
    Shader glowShader = LoadShader(0, "glow.fs");
    RenderTexture2D target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    bool dropDownOpen = true;
    const char *tabTexts[MAX_TABS] = {"General", "Particles", "Explosions", "Virtual Particles", "Physics"};

    int xOffset = GetScreenWidth() * 0.02;
    int yOffset = GetScreenHeight() * 0.02;
    int seed = 0;

    RenderTexture2D simulationTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    int buttonWidth = GetScreenWidth() * 0.05;
    int buttonHeight = GetScreenHeight() * 0.025;

    Rectangle pauseBtnRect = (Rectangle){GetScreenWidth() * 0.01, GetScreenHeight() * 0.95 - buttonHeight, buttonWidth, buttonHeight};
    Rectangle resumeBtnRect = (Rectangle){GetScreenWidth() * 0.01 + buttonWidth + 10, GetScreenHeight() * 0.95 - buttonHeight, buttonWidth, buttonHeight};
    Rectangle resetBtnRect = (Rectangle){GetScreenWidth() * 0.01 + 2 * (buttonWidth + 10), GetScreenHeight() * 0.95 - buttonHeight, buttonWidth, buttonHeight};
    Rectangle settingsBtnRect = (Rectangle){
        GetScreenWidth() - buttonWidth - (GetScreenWidth() * 0.01),
        GetScreenHeight() - buttonHeight - (GetScreenHeight() * 0.05),
        buttonWidth,
        buttonHeight
    };

    while (!WindowShouldClose())
    {
        float delta = GetFrameTime();
        GuiSetStyle(DEFAULT, TEXT_SIZE, scaledFontSize);
        Vector2 mousePos = GetMousePosition();
        Rectangle buttonRect = (Rectangle){GetScreenWidth() - 150, GetScreenHeight() - 50, 130, 30};

        bool mouseOverButton = CheckCollisionPointRec(mousePos, buttonRect);
        bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        bool overPause = CheckCollisionPointRec(mousePos, pauseBtnRect);
        bool overResume = CheckCollisionPointRec(mousePos, resumeBtnRect);
        bool overReset = CheckCollisionPointRec(mousePos, resetBtnRect);
        bool overSetting = CheckCollisionPointRec(mousePos, settingsBtnRect);

        if (IsWindowResized()) {
            UnloadRenderTexture(simulationTexture);
            UnloadRenderTexture(target);
            simulationTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
            target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
            gridWidth = ((GetScreenWidth() + CELL_SIZE - 1) / CELL_SIZE);
            gridHeight = ((GetScreenHeight() + CELL_SIZE - 1) / CELL_SIZE);
        }

        if (mouseClicked)
        {
            if (overPause)
            {
                paused = true;
            }
            else if (overResume)
            {
                paused = false;
            }
            else if (overReset)
            {
                ResetParticles(&config, &particles, GetScreenWidth(), GetScreenHeight());
            }
            else if (overSetting)
            {
                showConfigPanel = showConfigPanel ? false : true;
            }
        }

        if (!showConfigPanel && !paused)
        {
            HandleInput(&config, &particles);

            if (config.virtualParticles)
            {
                GenerateVirtualParticles(&config, &particles, delta, GetScreenWidth(), GetScreenHeight());
            }

            UpdateSimulation(&config, &particles, &target, GetScreenWidth(), GetScreenHeight(), gridWidth, gridHeight);
        }

        BeginTextureMode(simulationTexture);
        ClearBackground(RAYWHITE);

        if (!showConfigPanel)
        {
            BeginScissorMode(0, 0, GetScreenWidth(), GetScreenHeight() - (int)(GetScreenHeight() * 0.05));
            RenderSimulation(glowShader, target);
            EndScissorMode();
        }
        else
        {
            ClearBackground(RAYWHITE);
            Rectangle tabBarBounds = {GetScreenWidth() * 0.02, GetScreenHeight() * 0.02, GetScreenWidth() * 0.95, GetScreenHeight() * 0.05};
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
        DrawTextureRec(simulationTexture.texture, (Rectangle){0, 0, (float)simulationTexture.texture.width, (float)-simulationTexture.texture.height}, (Vector2){0, 0}, WHITE);
        int footerHeight = buttonHeight + GetScreenHeight() * 0.03;
        DrawRectangle(0, GetScreenHeight() - footerHeight, GetScreenWidth(), footerHeight, (Color){200, 200, 200, 255});

        int numButtons = 4;
        int spacing = 10;
        float totalWidth = numButtons * buttonWidth + (numButtons - 1) * spacing;
        float startX = (GetScreenWidth() - totalWidth) / 2.0f;
        float centerY = GetScreenHeight() - footerHeight/2.0f - buttonHeight/2.0f;

        pauseBtnRect = (Rectangle){startX, centerY, buttonWidth, buttonHeight};
        resumeBtnRect = (Rectangle){startX + (buttonWidth + spacing), centerY, buttonWidth, buttonHeight};
        resetBtnRect = (Rectangle){startX + 2*(buttonWidth + spacing), centerY, buttonWidth, buttonHeight};
        settingsBtnRect = (Rectangle){startX + 3*(buttonWidth + spacing), centerY, buttonWidth, buttonHeight};

        if (GuiButton(pauseBtnRect, "Pause"))
        {
        }
        if (GuiButton(resumeBtnRect, "Resume"))
        {
        }
        if (GuiButton(resetBtnRect, "Reset"))
        {
        }
        if (GuiButton(settingsBtnRect, "Settings"))
        {
        }
        EndDrawing();
    }

    CleanupSimulation(&particles, glowShader, target, gridWidth, gridHeight);
    FreeGrid(gridWidth, gridHeight);
    UnloadRenderTexture(simulationTexture);
    CloseWindow();

    return 0;
}