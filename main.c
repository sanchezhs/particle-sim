#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "simulation.h"
#include "gui.h"

#define SIMULATION_WIDTH  2500
#define SIMULATION_HEIGHT 2500

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GetScreenWidth(), GetScreenHeight(), "Particle Simulation");
    SetTargetFPS(60);

    Camera2D camera = {0};
    camera.target = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;


    bool showConfigPanel = false;
    bool paused = false;
    bool boot = true;

    int screenWidth = GetScreenWidth();
    float scaleFactor = (float)screenWidth / 800;
    int baseFontSize = 10;
    int scaledFontSize = (int)(baseFontSize * scaleFactor);
    float gridWidth = ((GetScreenWidth() + CELL_SIZE - 1) / CELL_SIZE);
    float gridHeight = ((GetScreenHeight() + CELL_SIZE - 1) / CELL_SIZE);

    int activeTab = 0;
    Color boundaryColor = RED;

    // Simulation configuration
    SimulationConfig config = {
        // GENERAL
        .maxParticles = 1000,
        .initialCapacity = 500,
        .lifetime = false,
        .fragmentParticlesLive = false,
        .virtualParticles = false,

        // PARTICLES
        .minParticleLifeTime = 50,
        .maxParticleLifeTime = 250,
        .minParticleSpeed = 5,
        .maxParticleSpeed = 25,
        .minParticleMass = 1,
        .maxParticleMass = 100,
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
        .gUniversal = 6.67430e-11f,
        .maxGravityDistance = 50.0f,
        .kElectric = 5.0f,
        .maxForce = 10.0f,
        .gravityType = GRAVITY_NONE,
        .electricForce = false};

    Particles particles = {
        .items = (Particle *)malloc(config.maxParticles * sizeof(Particle)),
        .count = 0,
        .capacity = config.initialCapacity,
    };

    Shader glowShader = LoadShader(0, "glow.fs");
    // RenderTexture2D target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    RenderTexture2D target = LoadRenderTexture(SIMULATION_WIDTH, SIMULATION_HEIGHT);
    RenderTexture2D simulationTexture = LoadRenderTexture(SIMULATION_WIDTH, SIMULATION_HEIGHT);

    bool dropDownOpen = true;
    const char *tabTexts[MAX_TABS] = {"General", "Particles", "Explosions", "Virtual Particles", "Physics"};

    int xOffset = GetScreenWidth() * 0.02;
    int yOffset = GetScreenHeight() * 0.02;
    int seed = 0;

    int buttonWidth = GetScreenWidth() * 0.05;
    int buttonHeight = GetScreenHeight() * 0.025;
    int footerHeight = buttonHeight + GetScreenHeight() * 0.03;

    Rectangle pauseBtnRect = (Rectangle){GetScreenWidth() * 0.01, GetScreenHeight() * 0.95 - buttonHeight, buttonWidth, buttonHeight};
    Rectangle resumeBtnRect = (Rectangle){GetScreenWidth() * 0.01 + buttonWidth + 10, GetScreenHeight() * 0.95 - buttonHeight, buttonWidth, buttonHeight};
    Rectangle resetBtnRect = (Rectangle){GetScreenWidth() * 0.01 + 2 * (buttonWidth + 10), GetScreenHeight() * 0.95 - buttonHeight, buttonWidth, buttonHeight};
    Rectangle settingsBtnRect = (Rectangle){
        GetScreenWidth() - buttonWidth - (GetScreenWidth() * 0.01),
        GetScreenHeight() - buttonHeight - (GetScreenHeight() * 0.05),
        buttonWidth,
        buttonHeight};

    InitGrid(&config, gridWidth, gridHeight);
    InitParticles(&config, &particles, SIMULATION_WIDTH, SIMULATION_HEIGHT - footerHeight);

    static bool rightMousePressed = false;
    static Vector2 previousMousePosition = {0};
    const float ZOOM_INCREMENT = 0.1f;  // Increment per scroll
    const float ZOOM_MIN = 0.1f;        // Minimum zoom
    const float ZOOM_MAX = 10.0f;       // Maximum zoom


   while (!WindowShouldClose())
    {
        float delta = GetFrameTime();
        GuiSetStyle(DEFAULT, TEXT_SIZE, scaledFontSize);
        Vector2 mousePos = GetMousePosition();

        bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        float wheelMove = GetMouseWheelMove();
        if (wheelMove != 0)
        {
            camera.zoom += wheelMove * ZOOM_INCREMENT * camera.zoom;
            if (camera.zoom < ZOOM_MIN) camera.zoom = ZOOM_MIN;
            if (camera.zoom > ZOOM_MAX) camera.zoom = ZOOM_MAX;
        }

        // Update footerHeight if window is resized
        footerHeight = buttonHeight + (int)(GetScreenHeight() * 0.03f);

        // Recalculate button positions
        int numButtons = 4;
        int spacing = 10;
        float totalWidth = numButtons * buttonWidth + (numButtons - 1) * spacing;
        float startX = (GetScreenWidth() - totalWidth) / 2.0f;
        float centerY = GetScreenHeight() - footerHeight / 2.0f - buttonHeight / 2.0f;

        pauseBtnRect = (Rectangle){startX, centerY, buttonWidth, buttonHeight};
        resumeBtnRect = (Rectangle){startX + (buttonWidth + spacing), centerY, buttonWidth, buttonHeight};
        resetBtnRect = (Rectangle){startX + 2 * (buttonWidth + spacing), centerY, buttonWidth, buttonHeight};
        settingsBtnRect = (Rectangle){startX + 3 * (buttonWidth + spacing), centerY, buttonWidth, buttonHeight};

        // Handle window resize events
        if (IsWindowResized())
        {
            // Update RenderTextures based on simulation size
            UnloadRenderTexture(simulationTexture);
            UnloadRenderTexture(target);
            simulationTexture = LoadRenderTexture(SIMULATION_WIDTH, SIMULATION_HEIGHT);
            target = LoadRenderTexture(SIMULATION_WIDTH, SIMULATION_HEIGHT);

            // Update grid
            gridWidth = ((SIMULATION_WIDTH + CELL_SIZE - 1) / CELL_SIZE);
            gridHeight = ((SIMULATION_HEIGHT + CELL_SIZE - 1) / CELL_SIZE);
            InitGrid(&config, gridWidth, gridHeight);
            ResetSimulation(&config, &particles, SIMULATION_WIDTH, SIMULATION_HEIGHT - footerHeight);

            // Update camera
            camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
        }

        // Handle button clicks
        if (mouseClicked)
        {
            if (CheckCollisionPointRec(mousePos, pauseBtnRect))
            {
                paused = true;
            }
            else if (CheckCollisionPointRec(mousePos, resumeBtnRect))
            {
                paused = false;
            }
            else if (CheckCollisionPointRec(mousePos, resetBtnRect))
            {
                ResetSimulation(&config, &particles, SIMULATION_WIDTH, SIMULATION_HEIGHT - footerHeight);
            }
            else if (CheckCollisionPointRec(mousePos, settingsBtnRect))
            {
                showConfigPanel = !showConfigPanel;
            }
        }

        // Handle right mouse button drag to move the camera
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
            Vector2 currentMousePosition = GetMousePosition();

            if (!rightMousePressed)
            {
                rightMousePressed = true;
                previousMousePosition = currentMousePosition;
            }
            else
            {
                Vector2 deltaMouse = (Vector2){currentMousePosition.x - previousMousePosition.x, currentMousePosition.y - previousMousePosition.y};
                camera.target = (Vector2){camera.target.x - deltaMouse.x / camera.zoom, camera.target.y - deltaMouse.y / camera.zoom};
                previousMousePosition = currentMousePosition;
            }
        }
        else
        {
            rightMousePressed = false;
        }

        if (!showConfigPanel && !paused)
        {
            HandleInput(&config, &particles, GetScreenWidth(), GetScreenHeight() - footerHeight);

            if (config.virtualParticles)
            {
                GenerateVirtualParticles(&config, &particles, delta, SIMULATION_WIDTH, SIMULATION_HEIGHT - footerHeight);
            }
            UpdateSimulation(&config, &particles, &target, SIMULATION_WIDTH, SIMULATION_HEIGHT, gridWidth, gridHeight);
        }

        BeginTextureMode(simulationTexture);
            ClearBackground(RAYWHITE);

            if (!showConfigPanel)
            {
                BeginShaderMode(glowShader);
                    DrawTextureRec(target.texture, (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height}, (Vector2){0, 0}, WHITE);
                EndShaderMode();

                Rectangle simBounds = {0, 0, SIMULATION_WIDTH, SIMULATION_HEIGHT};
                DrawRectangleLines(0, 0, SIMULATION_WIDTH, SIMULATION_HEIGHT, boundaryColor);
                int lineThickness = 3;
                DrawRectangleLinesEx(simBounds, lineThickness, boundaryColor);

            }
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK); // Main background

            BeginMode2D(camera);
            DrawTextureRec(simulationTexture.texture, (Rectangle){0, 0, (float)simulationTexture.texture.width, (float)-simulationTexture.texture.height}, (Vector2){0, 0}, WHITE);
            EndMode2D();

            DrawRectangle(0, GetScreenHeight() - footerHeight, GetScreenWidth(), footerHeight, (Color){200, 200, 200, 255});

            GuiButton(pauseBtnRect, "Pause");
            GuiButton(resumeBtnRect, "Resume");
            GuiButton(resetBtnRect, "Reset");
            GuiButton(settingsBtnRect, "Settings");
        
            // Show current zoom level
            char zoomText[50];
            snprintf(zoomText, sizeof(zoomText), "Zoom: %.2fx", camera.zoom);
            GuiLabel((Rectangle){GetScreenWidth() - 150, 10, 140, 30}, zoomText);

            // Show particle count
            char particleText[50];
            snprintf(particleText, sizeof(particleText), "Particles: %d", particles.count);
            GuiLabel((Rectangle){(int)(GetScreenWidth() / 2 - 50), 10, 200, buttonHeight}, particleText);


            // Render configuration menu if active
            if (showConfigPanel)
            {
            // Add a semi-transparent overlay for the menu background
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 128});

            // Define bounds for the configuration menu to cover the entire screen
            Rectangle configBounds = {0, 0, GetScreenWidth(), GetScreenHeight()};

            // Draw a background rectangle for the menu (you can customize the color)
            DrawRectangleRec(configBounds, RAYWHITE);

            // Define the area for the tab bar
            Rectangle tabBarBounds = {0, 0, GetScreenWidth(), GetScreenHeight() * 0.05f};

            // Draw the tab bar
            int closedTab = GuiTabBar(tabBarBounds, tabTexts, MAX_TABS, &activeTab);

            if (closedTab >= 0)
            {
                TraceLog(LOG_INFO, "Closed tab: %d", closedTab);
            }

            // Render the active tab in the screen space
            switch (activeTab)
            {
                case 0:
                generalTab(&config.maxParticles, &config.initialCapacity, &seed, &config.lifetime, &config.fragmentParticlesLive, &config.virtualParticles);
                break;
                case 1:
                particlesTab(&config.minParticleLifeTime, &config.maxParticleLifeTime, &config.minParticleSpeed, &config.maxParticleSpeed, &config.minParticleMass, &config.maxParticleMass, &config.trailLength);
                break;
                case 2:
                explosionTab(&config.minExplosionParticles, &config.maxExplosionParticles);
                break;
                case 3:
                virtualParticlesTab(&config.minVirtualParticleSpeed, &config.maxVirtualParticleSpeed, &config.minVirtualParticleLifeTime, &config.maxVirtualParticleLifeTime, &config.minTimeBetweenVirtualPairs);
                break;
                case 4:
                physicsTab(&config.g, &config.gUniversal, &config.maxGravityDistance, &config.kElectric, &config.maxForce, &config.gravityType, &config.electricForce, &dropDownOpen);
                break;
                default:
                break;
            }

            // Add a button to close the configuration menu
            Rectangle closeBtnRect = {GetScreenWidth() - 120, GetScreenHeight() * 0.05f + 10, 100, 50};
            if (GuiButton(closeBtnRect, "Close"))
            {
                showConfigPanel = false;
            }
            }

        EndDrawing();
    }

    CleanupSimulation(&particles, glowShader, target, gridWidth, gridHeight);
    FreeGrid(gridWidth, gridHeight);
    UnloadRenderTexture(simulationTexture);
    CloseWindow();

    return 0;
}