#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define BASE_SCREEN_WIDTH 800
#define BASE_SCREEN_HEIGHT 800

#define MAX_TABS 5

#define WIDTH 800
#define HEIGHT 800

typedef enum GravityType
{
    GRAVITY_CENTER,
    GRAVITY_DOWN,
    GRAVITY_UP,
    GRAVITY_LEFT,
    GRAVITY_RIGHT,
    GRAVITY_NONE
} GravityType;

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

    // Gravedad estándar (G)
    yOffset += 40;
    float gTemp = *g;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Gravitational\nAcceleration");
    GuiSlider((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, NULL, &gTemp, 1.0f, 500.0f);
    *g = gTemp;
    int gTempInt = (int)*g;
    GuiValueBox((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, &gTempInt, 1, 500, false);

    // Constante gravitacional universal (G_UNIVERSAL)
    yOffset += 40;
    float gUniversalTemp = *gUniversal;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Universal\nGravitational Constant");
    GuiSlider((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, NULL, &gUniversalTemp, -100.0f, 1000.0f);
    *gUniversal = gUniversalTemp;
    int gUniversalTempInt = (int)(*gUniversal);
    GuiValueBox((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, &gUniversalTempInt, -100, 1000, false);

    // Distancia máxima de gravedad
    yOffset += 40;
    float maxGravityDistanceTemp = *maxGravityDistance;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Max Gravity Distance:");
    GuiSlider((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, NULL, &maxGravityDistanceTemp, 1.0f, 1000.0f);
    *maxGravityDistance = maxGravityDistanceTemp;
    int maxGravityDistanceTempInt = (int)(*maxGravityDistance);
    GuiValueBox((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, &maxGravityDistanceTempInt, 1, 1000, false);

    // Constante de fuerza eléctrica (K_ELECTRIC)
    yOffset += 40;
    float kElectricTemp = *kElectric;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Electrical Constant");
    GuiSlider((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, NULL, &kElectricTemp, 1.0f, 1000.0f);
    *kElectric = kElectricTemp;
    int kElectricTempInt = (int)(*kElectric);
    GuiValueBox((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, &kElectricTempInt, 1, 1000, false);

    // Activar/desactivar fuerza eléctrica
    yOffset += 40;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Enable Electric Force:");
    GuiCheckBox((Rectangle){xOffset + 250, yOffset, 20, 20}, "Electric Force", electricForce);

    // Fuerza máxima permitida (MAX_FORCE)
    yOffset += 40;
    float maxForceTemp = *maxForce;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Max Force Allowed");
    GuiSlider((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, NULL, &maxForceTemp, 1.0f, 1000.0f);
    *maxForce = maxForceTemp;
    int maxForceTempInt = (int)(*maxForce);
    GuiValueBox((Rectangle){xOffset + 250, yOffset, 300, 20}, NULL, &maxForceTempInt, 1, 1000, false);

    // Tipo de gravedad
    yOffset += 40;
    GuiLabel((Rectangle){xOffset, yOffset, 200, 20}, "Gravity Type:");
    const char *gravityOptions[] = {"Center", "Down", "Up", "Left", "Right", "None"};
    if (GuiDropdownBox((Rectangle){xOffset + 250, yOffset, 300, 20},
                       "Center;Down;Up;Left;Right;None", (int *)gravityType, *dropDownOpen))
    {
        *dropDownOpen = !*dropDownOpen;
    }
}

int main(void)
{
    // Inicializar ventana
    InitWindow(BASE_SCREEN_WIDTH, BASE_SCREEN_HEIGHT, "Panel de Control");
    SetTargetFPS(60);

    bool showConfigPanel = false;

    // Obtener resolución actual del monitor
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Calcular factor de escala basado en la resolución
    float scaleFactor = (float)screenWidth / BASE_SCREEN_WIDTH;

    // Escalar tamaño de fuente
    int baseFontSize = 20;                                  // Tamaño base de la fuente
    int scaledFontSize = (int)(baseFontSize * scaleFactor); // Tamaño escalado de la fuente

    float scaleFactorX = (float)screenWidth / BASE_SCREEN_WIDTH;
    float scaleFactorY = (float)screenHeight / BASE_SCREEN_HEIGHT;

    // GENERAL TAB
    int activeTab = 0;         // Pestaña activa por defecto
    int maxParticles = 1000;   // Número máximo de partículas
    int initialCapacity = 500; // Capacidad inicial
    bool lifetime = false;     // Opciones booleanas
    bool fragmentParticlesLive = false;
    bool virtualParticles = false;

    // PARTICLES TAB
    float minParticleLifeTime = 10;
    float maxParticleLifeTime = 50;
    float minParticleSpeed = 10;
    float maxParticleSpeed = 20;
    float minParticleMass = 5;
    float maxParticleMass = 10;
    float trailLength = 5;

    // EXPLOSION TAB
    int minExplosionParticles = 0;
    int maxExplosionParticles = 5;

    // VIRTUAL PARTICLE TAB
    int minVirtualParticleSpeed = 5;
    int maxVirtualParticleSpeed = 25;
    int minVirtualParticleLifeTime = 5;
    int maxVirtualParticleLifeTime = 25;
    float minTimeBetweenVirtualPairs = 5.0f;

    // PHYSICS TAB
    float g = 9.81f;
    float gUniversal = 10;
    float maxGravityDistance = 100.0f;
    float kElectric = 50.0f;
    float maxForce = 100.0f;
    GravityType gravityType = GRAVITY_NONE;
    bool electricForce = false;

    bool dropDownOpen = true;

    Rectangle panelRec = {20, 40, 200, 150};
    Rectangle panelContentRec = {0, 0, 340, 340};
    Rectangle panelView = {0};
    Vector2 panelScroll = {99, -20};

    const char *tabTexts[MAX_TABS] = {"General", "Particles", "Explosions", "Virtual Particles", "Physics"};

    bool showContentArea = true;

    bool editMode = true;
    int value = 0;
    int active = 0;

    int xOffset = 20;
    int yOffset = 20;

    int seed = 0;

    while (!WindowShouldClose())
    {
        GuiSetStyle(DEFAULT, TEXT_SIZE, scaledFontSize);
        if (showConfigPanel)
        {
            ClearBackground(RAYWHITE);


            Rectangle tabBarBounds = {xOffset, yOffset, 760, 40};
            int closedTab = GuiTabBar(tabBarBounds, tabTexts, MAX_TABS, &activeTab);

            if (closedTab >= 0)
            {
                TraceLog(LOG_INFO, "Pestaña cerrada: %d", closedTab);
            }

            // General
            if (activeTab == 0)
            {
                generalTab(&maxParticles, &initialCapacity, &seed, &lifetime, &fragmentParticlesLive, &virtualParticles);
            }
            else if (activeTab == 1)
            {
                particlesTab(&minParticleLifeTime, &maxParticleLifeTime, &minParticleSpeed, &maxParticleSpeed, &minParticleMass, &maxParticleMass, &trailLength);
            }
            else if (activeTab == 2)
            {
                explosionTab(&minExplosionParticles, &maxExplosionParticles);
            }
            else if (activeTab == 3)
            {
                virtualParticlesTab(&minVirtualParticleSpeed, &maxVirtualParticleSpeed, &minVirtualParticleLifeTime, &maxVirtualParticleLifeTime, &minTimeBetweenVirtualPairs);
            }
            else if (activeTab == 4)
            {
                physicsTab(&g, &gUniversal, &maxGravityDistance, &kElectric, &maxForce, &gravityType, &electricForce, &dropDownOpen);
            }

            if (GuiButton((Rectangle){BASE_SCREEN_WIDTH - 150, BASE_SCREEN_HEIGHT - 50, 130, 30}, "Simulation"))
            {
                showConfigPanel = false;
            }
        }
        else
        {
            ClearBackground(BLACK);

            // TODO: Draw simulation here

            if (GuiButton((Rectangle){BASE_SCREEN_WIDTH - 150, BASE_SCREEN_HEIGHT - 50, 130, 30}, "Settings"))
            {
                showConfigPanel = true;
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
