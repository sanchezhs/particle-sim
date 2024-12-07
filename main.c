#define RAYGUI_IMPLEMENTATION

#if defined(PLATFORM_WEB)
#include <emscripten.h>
#endif

#include "raygui.h"
#include "simulation.h"
#include "cJSON.h"
#include "gui.h"

#define SIMULATION_WIDTH 2500
#define SIMULATION_HEIGHT 2500

Camera2D camera;
bool showConfigPanel = false;
bool paused = false;
int activeTab = 0;

float gridWidth = 0.0f;
float gridHeight = 0.0f;

int buttonWidth = 0;
int buttonHeight = 0;
int footerHeight = 0;

int screenWidth;
float scaleFactor;
int baseFontSize;
int scaledFontSize;

Rectangle pauseBtnRect;
Rectangle resumeBtnRect;
Rectangle resetBtnRect;
Rectangle settingsBtnRect;

SimulationConfig config;
Particles particles;
GeneralTabParameters gtp;
PatternsTabParameters pstp;
ParticlesTabParameters ptp;
ExplosionTabParameters etp;
VirtualParticlesTabParameters vtp;
PhysicsTabParameters phtp;

RenderTexture2D target;
RenderTexture2D simulationTexture;

static bool rightMousePressed = false;
static Vector2 previousMousePosition = {0};
const float ZOOM_INCREMENT = 0.1f;
const float ZOOM_MIN = 0.1f;
const float ZOOM_MAX = 10.0f;

// Tab texts
const char *tabTexts[MAX_TABS] = {"General", "Patterns", "Particles", "Explosions", "Virtual Particles", "Physics"};

#if defined(PLATFORM_WEB)

GravityType getGravityType(const char *gravityTypeStr) {
    if (strcmp(gravityTypeStr, "none") == 0) {
        return GRAVITY_NONE;
    } else if (strcmp(gravityTypeStr, "center") == 0) {
        return GRAVITY_CENTER;
    } else if (strcmp(gravityTypeStr, "down") == 0) {
        return GRAVITY_DOWN;
    } else if (strcmp(gravityTypeStr, "up") == 0) {
        return GRAVITY_UP;
    } else if (strcmp(gravityTypeStr, "left") == 0) {
        return GRAVITY_LEFT;
    } else if (strcmp(gravityTypeStr, "right") == 0) {
        return GRAVITY_RIGHT;
    } else {
        return GRAVITY_NONE;
    }
}

ParticlePattern getPatternType(const char *patternStr) {
    if (strcmp(patternStr, "random") == 0) {
        return PATTERN_RANDOM;
    } else if (strcmp(patternStr, "vortex") == 0) {
        return PATTERN_VORTEX;
    } else if (strcmp(patternStr, "group") == 0) {
        return PATTERN_GROUP;
    } else {
        return PATTERN_RANDOM;
    }
}

EMSCRIPTEN_KEEPALIVE
void on_pause_button_click(void)
{
    paused = true;
}

EMSCRIPTEN_KEEPALIVE
void on_continue_button_click(void)
{
    paused = false;
}

EMSCRIPTEN_KEEPALIVE
void on_reset_button_click(void)
{
    ResetSimulation(&config, &particles, SIMULATION_WIDTH, SIMULATION_HEIGHT - footerHeight);
}

EMSCRIPTEN_KEEPALIVE
void on_settings_button_click(void)
{
    showConfigPanel = !showConfigPanel;
}

EMSCRIPTEN_KEEPALIVE
void apply_config(const char *config_json)
{
    printf("Applying config: %s\n", config_json);

    // Parsear el JSON
    cJSON *json = cJSON_Parse(config_json);
    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf("Error antes de: %s\n", error_ptr);
        }
        return;
    }

    // *** General ***

    // maxParticles
    cJSON *maxParticles = cJSON_GetObjectItemCaseSensitive(json, "maxParticles");
    if (cJSON_IsNumber(maxParticles))
    {
        config.maxParticles = maxParticles->valueint;
        printf("Updated maxParticles: %d\n", config.maxParticles);
    }

    // initialParticles
    cJSON *initialParticles = cJSON_GetObjectItemCaseSensitive(json, "initialParticles");
    if (cJSON_IsNumber(initialParticles))
    {
        config.initialCapacity = initialParticles->valueint;
        printf("Updated initialCapacity: %d\n", config.initialCapacity);
    }

    // friction
    cJSON *friction = cJSON_GetObjectItemCaseSensitive(json, "friction");
    if (cJSON_IsNumber(friction))
    {
        config.friction = friction->valuedouble;
        printf("Updated friction: %.2f\n", config.friction);
    }

    // setSeed
    // cJSON *setSeed = cJSON_GetObjectItemCaseSensitive(json, "setSeed");
    // if (cJSON_IsNumber(setSeed))
    // {
    //     config.setSeed = setSeed->valueint;
    //     printf("Updated setSeed: %d\n", config.setSeed);
    // }

    // particlesDie (correspondiente a lifetime)
    cJSON *particlesDie = cJSON_GetObjectItemCaseSensitive(json, "particlesDie");
    if (cJSON_IsBool(particlesDie))
    {
        config.lifetime = cJSON_IsTrue(particlesDie);
        printf("Updated lifetime: %s\n", config.lifetime ? "true" : "false");
    }

    // *** Patterns ***

    // initialPattern
    cJSON *initialPattern = cJSON_GetObjectItemCaseSensitive(json, "initialPattern");
    if (cJSON_IsString(initialPattern) && (initialPattern->valuestring != NULL))
    {
        config.initialPattern = getPatternType(initialPattern->valuestring);
        printf("Updated initialPattern: %d\n", config.initialPattern);
    }

    // vortexCenter
    cJSON *vortexCenter = cJSON_GetObjectItemCaseSensitive(json, "vortexCenter");
    if (cJSON_IsObject(vortexCenter))
    {
        cJSON *x = cJSON_GetObjectItemCaseSensitive(vortexCenter, "x");
        cJSON *y = cJSON_GetObjectItemCaseSensitive(vortexCenter, "y");
        if (cJSON_IsNumber(x) && cJSON_IsNumber(y))
        {
            config.vortexCenter.x = (float)x->valuedouble;
            config.vortexCenter.y = (float)y->valuedouble;
            printf("Updated vortexCenter: x=%.2f, y=%.2f\n", config.vortexCenter.x, config.vortexCenter.y);
        }
    }

    // vortexStrength
    cJSON *vortexStrength = cJSON_GetObjectItemCaseSensitive(json, "vortexStrength");
    if (cJSON_IsNumber(vortexStrength))
    {
        config.vortexStrength = (float)vortexStrength->valuedouble;
        printf("Updated vortexStrength: %.2f\n", config.vortexStrength);
    }

    // radialStrength
    cJSON *radialStrength = cJSON_GetObjectItemCaseSensitive(json, "radialStrength");
    if (cJSON_IsNumber(radialStrength))
    {
        config.radialStrength = (float)radialStrength->valuedouble;
        printf("Updated radialStrength: %.2f\n", config.radialStrength);
    }

    // vortexRadius
    cJSON *vortexRadius = cJSON_GetObjectItemCaseSensitive(json, "vortexRadius");
    if (cJSON_IsNumber(vortexRadius))
    {
        config.vortexRadius = (float)vortexRadius->valuedouble;
        printf("Updated vortexRadius: %.2f\n", config.vortexRadius);
    }

    // numGroups
    cJSON *numGroups = cJSON_GetObjectItemCaseSensitive(json, "numGroups");
    if (cJSON_IsNumber(numGroups))
    {
        config.numGroups = numGroups->valueint;
        if (config.numGroups > 100)
        { // Asegurarse de no exceder el máximo
            config.numGroups = 100;
        }
        printf("Updated numGroups: %d\n", config.numGroups);
    }

    // groupCenters
    cJSON *groupCenters = cJSON_GetObjectItemCaseSensitive(json, "groupCenters");
    if (cJSON_IsArray(groupCenters))
    {
        int groupCount = cJSON_GetArraySize(groupCenters);
        if (groupCount > 100)
        { // Limitar al máximo permitido
            groupCount = 100;
        }
        config.numGroups = groupCount; // Actualizar numGroups basado en el array
        printf("Updated numGroups based on groupCenters array: %d\n", config.numGroups);
        for (int i = 0; i < groupCount; i++)
        {
            cJSON *group = cJSON_GetArrayItem(groupCenters, i);
            if (cJSON_IsObject(group))
            {
                cJSON *x = cJSON_GetObjectItemCaseSensitive(group, "x");
                cJSON *y = cJSON_GetObjectItemCaseSensitive(group, "y");
                if (cJSON_IsNumber(x) && cJSON_IsNumber(y))
                {
                    config.groupCenters[i].x = (float)x->valuedouble;
                    config.groupCenters[i].y = (float)y->valuedouble;
                    printf("Updated groupCenters[%d]: x=%.2f, y=%.2f\n", i, config.groupCenters[i].x, config.groupCenters[i].y);
                }
            }
        }
    }

    // particlesPerGroup
    cJSON *particlesPerGroup = cJSON_GetObjectItemCaseSensitive(json, "particlesPerGroup");
    if (cJSON_IsNumber(particlesPerGroup))
    {
        config.particlesPerGroup = particlesPerGroup->valueint;
        printf("Updated particlesPerGroup: %d\n", config.particlesPerGroup);
    }

    // *** Particles ***

    // minParticleLifeTime
    cJSON *minParticleLifeTime = cJSON_GetObjectItemCaseSensitive(json, "minParticleLifeTime");
    if (cJSON_IsNumber(minParticleLifeTime))
    {
        config.minParticleLifeTime = (float)minParticleLifeTime->valuedouble;
        printf("Updated minParticleLifeTime: %.2f\n", config.minParticleLifeTime);
    }

    // maxParticleLifeTime
    cJSON *maxParticleLifeTime = cJSON_GetObjectItemCaseSensitive(json, "maxParticleLifeTime");
    if (cJSON_IsNumber(maxParticleLifeTime))
    {
        config.maxParticleLifeTime = (float)maxParticleLifeTime->valuedouble;
        printf("Updated maxParticleLifeTime: %.2f\n", config.maxParticleLifeTime);
    }

    // minParticleSpeed
    cJSON *minParticleSpeed = cJSON_GetObjectItemCaseSensitive(json, "minParticleSpeed");
    if (cJSON_IsNumber(minParticleSpeed))
    {
        config.minParticleSpeed = (float)minParticleSpeed->valuedouble;
        printf("Updated minParticleSpeed: %.2f\n", config.minParticleSpeed);
    }

    // maxParticleSpeed
    cJSON *maxParticleSpeed = cJSON_GetObjectItemCaseSensitive(json, "maxParticleSpeed");
    if (cJSON_IsNumber(maxParticleSpeed))
    {
        config.maxParticleSpeed = (float)maxParticleSpeed->valuedouble;
        printf("Updated maxParticleSpeed: %.2f\n", config.maxParticleSpeed);
    }

    // minParticleMass
    cJSON *minParticleMass = cJSON_GetObjectItemCaseSensitive(json, "minParticleMass");
    if (cJSON_IsNumber(minParticleMass))
    {
        config.minParticleMass = (float)minParticleMass->valuedouble;
        printf("Updated minParticleMass: %.2f\n", config.minParticleMass);
    }

    // maxParticleMass
    cJSON *maxParticleMass = cJSON_GetObjectItemCaseSensitive(json, "maxParticleMass");
    if (cJSON_IsNumber(maxParticleMass))
    {
        config.maxParticleMass = (float)maxParticleMass->valuedouble;
        printf("Updated maxParticleMass: %.2f\n", config.maxParticleMass);
    }

    // trailLength
    cJSON *trailLength = cJSON_GetObjectItemCaseSensitive(json, "trailLength");
    if (cJSON_IsNumber(trailLength))
    {
        config.trailLength = trailLength->valueint;
        printf("Updated trailLength: %d\n", config.trailLength);
    }

    // *** Explosion ***

    // minExplosionParticles
    cJSON *minExplosionParticles = cJSON_GetObjectItemCaseSensitive(json, "minExplosionParticles");
    if (cJSON_IsNumber(minExplosionParticles))
    {
        config.minExplosionParticles = minExplosionParticles->valueint;
        printf("Updated minExplosionParticles: %d\n", config.minExplosionParticles);
    }

    // maxExplosionParticles
    cJSON *maxExplosionParticles = cJSON_GetObjectItemCaseSensitive(json, "maxExplosionParticles");
    if (cJSON_IsNumber(maxExplosionParticles))
    {
        config.maxExplosionParticles = maxExplosionParticles->valueint;
        printf("Updated maxExplosionParticles: %d\n", config.maxExplosionParticles);
    }

    // *** Virtual Particles ***

    // minVirtualParticleSpeed
    cJSON *minVirtualParticleSpeed = cJSON_GetObjectItemCaseSensitive(json, "minVirtualParticleSpeed");
    if (cJSON_IsNumber(minVirtualParticleSpeed))
    {
        config.minVirtualParticleSpeed = minVirtualParticleSpeed->valueint;
        printf("Updated minVirtualParticleSpeed: %d\n", config.minVirtualParticleSpeed);
    }

    // maxVirtualParticleSpeed
    cJSON *maxVirtualParticleSpeed = cJSON_GetObjectItemCaseSensitive(json, "maxVirtualParticleSpeed");
    if (cJSON_IsNumber(maxVirtualParticleSpeed))
    {
        config.maxVirtualParticleSpeed = maxVirtualParticleSpeed->valueint;
        printf("Updated maxVirtualParticleSpeed: %d\n", config.maxVirtualParticleSpeed);
    }

    // minVirtualParticleLifeTime
    cJSON *minVirtualParticleLifeTime = cJSON_GetObjectItemCaseSensitive(json, "minVirtualParticleLifeTime");
    if (cJSON_IsNumber(minVirtualParticleLifeTime))
    {
        config.minVirtualParticleLifeTime = minVirtualParticleLifeTime->valueint;
        printf("Updated minVirtualParticleLifeTime: %d\n", config.minVirtualParticleLifeTime);
    }

    // maxVirtualParticleLifeTime
    cJSON *maxVirtualParticleLifeTime = cJSON_GetObjectItemCaseSensitive(json, "maxVirtualParticleLifeTime");
    if (cJSON_IsNumber(maxVirtualParticleLifeTime))
    {
        config.maxVirtualParticleLifeTime = maxVirtualParticleLifeTime->valueint;
        printf("Updated maxVirtualParticleLifeTime: %d\n", config.maxVirtualParticleLifeTime);
    }

    // minTimeBetweenVirtualPairs
    cJSON *minTimeBetweenVirtualPairs = cJSON_GetObjectItemCaseSensitive(json, "minTimeBetweenVirtualPairs");
    if (cJSON_IsNumber(minTimeBetweenVirtualPairs))
    {
        config.minTimeBetweenVirtualPairs = (float)minTimeBetweenVirtualPairs->valuedouble;
        printf("Updated minTimeBetweenVirtualPairs: %.2f\n", config.minTimeBetweenVirtualPairs);
    }

    // *** Physics ***

    // g
    cJSON *g = cJSON_GetObjectItemCaseSensitive(json, "g");
    if (cJSON_IsNumber(g))
    {
        config.g = (float)g->valuedouble;
        printf("Updated g: %.2f\n", config.g);
    }

    // gUniversal
    cJSON *gUniversal = cJSON_GetObjectItemCaseSensitive(json, "gUniversal");
    if (cJSON_IsNumber(gUniversal))
    {
        config.gUniversal = gUniversal->valuedouble;
        printf("Updated gUniversal: %.5e\n", config.gUniversal);
    }

    // maxGravityDistance
    cJSON *maxGravityDistance = cJSON_GetObjectItemCaseSensitive(json, "maxGravityDistance");
    if (cJSON_IsNumber(maxGravityDistance))
    {
        config.maxGravityDistance = (float)maxGravityDistance->valuedouble;
        printf("Updated maxGravityDistance: %.2f\n", config.maxGravityDistance);
    }

    // kElectric
    cJSON *kElectric = cJSON_GetObjectItemCaseSensitive(json, "kElectric");
    if (cJSON_IsNumber(kElectric))
    {
        config.kElectric = (float)kElectric->valuedouble;
        printf("Updated kElectric: %.2f\n", config.kElectric);
    }

    // maxForce
    cJSON *maxForce = cJSON_GetObjectItemCaseSensitive(json, "maxForce");
    if (cJSON_IsNumber(maxForce))
    {
        config.maxForce = (float)maxForce->valuedouble;
        printf("Updated maxForce: %.2f\n", config.maxForce);
    }

    // gravityType
    cJSON *gravityType = cJSON_GetObjectItemCaseSensitive(json, "gravityType");
    if (cJSON_IsString(gravityType) && (gravityType->valuestring != NULL))
    {
        config.gravityType = getGravityType(gravityType->valuestring);
        printf("Updated gravityType: %d\n", config.gravityType);
    }

    // electricForce
    cJSON *electricForce = cJSON_GetObjectItemCaseSensitive(json, "electricForce");
    if (cJSON_IsBool(electricForce))
    {
        config.electricForce = cJSON_IsTrue(electricForce);
        printf("Updated electricForce: %s\n", config.electricForce ? "true" : "false");
    }

    // Liberar el objeto JSON
    cJSON_Delete(json);

    printf("Configuration applied successfully.\n");
}

#endif

void initGuiParamaters(SimulationConfig *config, GeneralTabParameters *gpt, PatternsTabParameters *pstb, ParticlesTabParameters *ptp, ExplosionTabParameters *etp, VirtualParticlesTabParameters *vtp, PhysicsTabParameters *phtp)
{
    // General Tab parameters
    gpt->maxParticles.value = &config->maxParticles;
    gpt->initialCapacity.value = &config->initialCapacity;
    gpt->seed = 0;
    gpt->lifetime.value = &config->lifetime;
    gpt->fragmentParticlesLive.value = &config->fragmentParticlesLive;
    gpt->virtualParticles.value = &config->virtualParticles;
    gpt->friction.value = &config->friction;

    snprintf(gpt->friction.textValue, sizeof(gpt->friction.textValue), "%.2f", *(gpt->friction.value));

    // Patterns Tab parameters
    pstb->dropDownOpen = false;
    pstb->initialPattern = &config->initialPattern;
    pstb->vortexCenter = &config->vortexCenter;
    pstb->vortexStrength.value = &config->vortexStrength;
    pstb->radialStrength.value = &config->radialStrength;
    pstb->vortexRadious.value = &config->vortexRadius;
    pstb->numGroups.value = &config->numGroups;
    pstb->groupCenters = config->groupCenters;
    pstb->particlesPerGroup.value = &config->particlesPerGroup;

    snprintf(pstb->vortexStrength.textValue, sizeof(pstb->vortexStrength.textValue), "%.2f", *(pstb->vortexStrength.value));
    snprintf(pstb->vortexRadious.textValue, sizeof(pstb->vortexRadious.textValue), "%.2f", *(pstb->vortexRadious.value));
    snprintf(pstb->radialStrength.textValue, sizeof(pstb->radialStrength.textValue), "%.2f", *(pstb->radialStrength.value));

    // Particle Tab parameters
    ptp->maxParticleLifeTime.value = &config->maxParticleLifeTime;
    ptp->minParticleLifeTime.value = &config->minParticleLifeTime;
    ptp->maxParticleSpeed.value = &config->maxParticleSpeed;
    ptp->minParticleSpeed.value = &config->minParticleSpeed;
    ptp->maxParticleMass.value = &config->maxParticleMass;
    ptp->minParticleMass.value = &config->minParticleMass;
    ptp->trailLength.value = &config->trailLength;

    snprintf(ptp->maxParticleLifeTime.textValue, sizeof(ptp->maxParticleLifeTime.textValue), "%.2f", *(ptp->maxParticleLifeTime.value));
    snprintf(ptp->minParticleLifeTime.textValue, sizeof(ptp->minParticleLifeTime.textValue), "%.2f", *(ptp->minParticleLifeTime.value));
    snprintf(ptp->maxParticleSpeed.textValue, sizeof(ptp->maxParticleSpeed.textValue), "%.2f", *(ptp->maxParticleSpeed.value));
    snprintf(ptp->minParticleSpeed.textValue, sizeof(ptp->minParticleSpeed.textValue), "%.2f", *(ptp->minParticleSpeed.value));
    snprintf(ptp->maxParticleMass.textValue, sizeof(ptp->maxParticleMass.textValue), "%.2f", *(ptp->maxParticleMass.value));
    snprintf(ptp->minParticleMass.textValue, sizeof(ptp->minParticleMass.textValue), "%.2f", *(ptp->minParticleMass.value));
    snprintf(ptp->trailLength.textValue, sizeof(ptp->trailLength.textValue), "%d", *(ptp->trailLength.value));

    // Explosion Tab parameters
    etp->minExplosionParticles.value = &config->minExplosionParticles;
    etp->maxExplosionParticles.value = &config->maxExplosionParticles;

    // Virtual Particles Tab parameters
    vtp->minVirtualParticleSpeed.value = &config->minVirtualParticleSpeed;
    vtp->maxVirtualParticleSpeed.value = &config->maxVirtualParticleSpeed;
    vtp->minVirtualParticleLifeTime.value = &config->minVirtualParticleLifeTime;
    vtp->maxVirtualParticleLifeTime.value = &config->maxVirtualParticleLifeTime;
    vtp->minTimeBetweenVirtualPairs.value = &config->minTimeBetweenVirtualPairs;

    snprintf(vtp->minTimeBetweenVirtualPairs.textValue, sizeof(vtp->minTimeBetweenVirtualPairs.textValue), "%.2f", *(vtp->minTimeBetweenVirtualPairs.value));

    // Physics Tab parameters
    phtp->g.value = &config->g;
    phtp->gUniversal.value = &config->gUniversal;
    phtp->maxGravityDistance.value = &config->maxGravityDistance;
    phtp->kElectric.value = &config->kElectric;
    phtp->maxForce.value = &config->maxForce;
    phtp->electricForce.value = &config->electricForce;
    phtp->gravityType = &config->gravityType;
    phtp->dropDownOpen = false;

    snprintf(phtp->g.textValue, sizeof(phtp->g.textValue), "%.2f", *(phtp->g.value));
    snprintf(phtp->gUniversal.textValue, sizeof(phtp->gUniversal.textValue), "%.20f", *(phtp->gUniversal.value));
    snprintf(phtp->maxGravityDistance.textValue, sizeof(phtp->maxGravityDistance.textValue), "%.2f", *(phtp->maxGravityDistance.value));
    snprintf(phtp->kElectric.textValue, sizeof(phtp->kElectric.textValue), "%.2f", *(phtp->kElectric.value));
    snprintf(phtp->maxForce.textValue, sizeof(phtp->maxForce.textValue), "%.2f", *(phtp->maxForce.value));
}

void main_loop_web(void)
{
    float delta = GetFrameTime();
    GuiSetStyle(DEFAULT, TEXT_SIZE, (int)(8 * ((float)GetScreenWidth() / 800.0f)));

    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0)
    {
        camera.zoom += wheelMove * ZOOM_INCREMENT * camera.zoom;
        if (camera.zoom < ZOOM_MIN)
            camera.zoom = ZOOM_MIN;
        if (camera.zoom > ZOOM_MAX)
            camera.zoom = ZOOM_MAX;
    }

    if (IsWindowResized())
    {
        UnloadRenderTexture(simulationTexture);
        UnloadRenderTexture(target);
        simulationTexture = LoadRenderTexture(SIMULATION_WIDTH, SIMULATION_HEIGHT);
        target = LoadRenderTexture(SIMULATION_WIDTH, SIMULATION_HEIGHT);

        gridWidth = ((SIMULATION_WIDTH + CELL_SIZE - 1) / CELL_SIZE);
        gridHeight = ((SIMULATION_HEIGHT + CELL_SIZE - 1) / CELL_SIZE);
        InitGrid(&config, gridWidth, gridHeight);
        ResetSimulation(&config, &particles, SIMULATION_WIDTH, SIMULATION_HEIGHT);

        camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    }

    if (IsKeyPressed(KEY_P)) // Pause
    {
        paused = true;
        TraceLog(LOG_INFO, "Paused simulation");
    }
    else if (IsKeyPressed(KEY_C)) // Continue (Resume)
    {
        paused = false;
        TraceLog(LOG_INFO, "Resumed simulation");
    }
    else if (IsKeyPressed(KEY_R)) // Reset
    {
        ResetSimulation(&config, &particles, SIMULATION_WIDTH, SIMULATION_HEIGHT);
        TraceLog(LOG_INFO, "Reset simulation");
    }
    else if (IsKeyPressed(KEY_S)) // Settings
    {
        showConfigPanel = !showConfigPanel;
        TraceLog(LOG_INFO, "Toggled settings panel");
    }

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
        HandleInput(&config, &particles, GetScreenWidth(), GetScreenHeight());

        if (config.virtualParticles)
        {
            GenerateVirtualParticles(&config, &particles, delta, SIMULATION_WIDTH, SIMULATION_HEIGHT);
        }
        UpdateSimulation(&config, &particles, &target, SIMULATION_WIDTH, SIMULATION_HEIGHT, gridWidth, gridHeight);
    }

    BeginTextureMode(simulationTexture);
    ClearBackground(RAYWHITE);

    if (!showConfigPanel)
    {
        DrawTextureRec(target.texture, (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height}, (Vector2){0, 0}, WHITE);

        if (config.initialPattern == PATTERN_BLACKHOLE)
        {
            DrawCircleV(config.blackHoleCenter, config.blackHoleRadius, BLACK);
            DrawCircleLines(config.blackHoleCenter.x, config.blackHoleCenter.y, config.blackHoleRadius, GRAY);

            float glowRadius = config.blackHoleRadius * 1.5f;
            float glowThickness = 2.0f;
            for (float r = config.blackHoleRadius; r <= glowRadius; r += glowThickness)
            {
                float alpha = 0.5f * (1.0f - (r - config.blackHoleRadius) / (glowRadius - config.blackHoleRadius));
                Color glowColor = (Color){128, 128, 128, (unsigned char)(alpha * 255)};
                DrawCircleLines(config.blackHoleCenter.x, config.blackHoleCenter.y, r, glowColor);
            }

            float accretionDiskInnerRadius = config.blackHoleRadius * 1.1f;
            float accretionDiskOuterRadius = config.blackHoleRadius * 2.0f;
            for (float r = accretionDiskInnerRadius; r <= accretionDiskOuterRadius; r += 1.0f)
            {
                float alpha = 0.3f * (1.0f - (r - accretionDiskInnerRadius) / (accretionDiskOuterRadius - accretionDiskInnerRadius));
                Color diskColor = (Color){255, 165, 0, (unsigned char)(alpha * 255)}; // Resplandor naranja
                DrawCircleLines(config.blackHoleCenter.x, config.blackHoleCenter.y, r, diskColor);
            }
        }
        Rectangle simBounds = {0, 0, SIMULATION_WIDTH, SIMULATION_HEIGHT};
        int lineThickness = 1;
        Color fillColor = Fade(BLACK, 0.1f);
        DrawRectangleRec(simBounds, fillColor);
        DrawRectangleLinesEx(simBounds, lineThickness, BLACK);
    }
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);

    BeginMode2D(camera);
    DrawTextureRec(simulationTexture.texture, (Rectangle){0, 0, (float)simulationTexture.texture.width, (float)-simulationTexture.texture.height}, (Vector2){0, 0}, WHITE);
    EndMode2D();

    char zoomText[50];
    snprintf(zoomText, sizeof(zoomText), "%.2fx", camera.zoom);
    GuiLabel((Rectangle){GetScreenWidth() - 90, 10, 140, 30}, zoomText);

    char particleText[50];
    snprintf(particleText, sizeof(particleText), "Particles: %d", particles.count);
    GuiLabel((Rectangle){(int)(GetScreenWidth() / 2 - 50), 10, 200, buttonHeight}, particleText);

    if (showConfigPanel)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 128});
        Rectangle configBounds = {0, 0, GetScreenWidth(), GetScreenHeight()};
        DrawRectangleRec(configBounds, RAYWHITE);
        Rectangle tabBarBounds = {0, 0, GetScreenWidth(), GetScreenHeight() * 0.05f};
        int closedTab = GuiTabBar(tabBarBounds, tabTexts, MAX_TABS, &activeTab);

        if (closedTab >= 0)
        {
            TraceLog(LOG_INFO, "Closed tab: %d", closedTab);
        }

        switch (activeTab)
        {
        case 0:
            generalTab(&gtp);
            break;
        case 1:
            patternsTab(&pstp);
            break;
        case 2:
            particlesTab(&ptp);
            break;
        case 3:
            explosionTab(&etp);
            break;
        case 4:
            virtualParticlesTab(&vtp);
            break;
        case 5:
            physicsTab(&phtp);
            break;
        default:
            break;
        }
    }
    EndDrawing();
}

void main_loop_desktop(void)
{
    screenWidth = GetScreenWidth();
    scaleFactor = (float)screenWidth / 800;
    baseFontSize = 8;
    scaledFontSize = (int)(baseFontSize * scaleFactor);

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
            if (camera.zoom < ZOOM_MIN)
                camera.zoom = ZOOM_MIN;
            if (camera.zoom > ZOOM_MAX)
                camera.zoom = ZOOM_MAX;
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
            DrawTextureRec(target.texture, (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height}, (Vector2){0, 0}, WHITE);

            if (config.initialPattern == PATTERN_BLACKHOLE)
            {
                // Draw the event horizon
                DrawCircleV(config.blackHoleCenter, config.blackHoleRadius, BLACK);
                DrawCircleLines(config.blackHoleCenter.x, config.blackHoleCenter.y, config.blackHoleRadius, GRAY);

                // Glow effect around the black hole
                float glowRadius = config.blackHoleRadius * 1.5f;
                float glowThickness = 2.0f;
                for (float r = config.blackHoleRadius; r <= glowRadius; r += glowThickness)
                {
                    float alpha = 0.5f * (1.0f - (r - config.blackHoleRadius) / (glowRadius - config.blackHoleRadius));
                    Color glowColor = (Color){128, 128, 128, (unsigned char)(alpha * 255)};
                    DrawCircleLines(config.blackHoleCenter.x, config.blackHoleCenter.y, r, glowColor);
                }

                // Accretion disk effect
                float accretionDiskInnerRadius = config.blackHoleRadius * 1.1f;
                float accretionDiskOuterRadius = config.blackHoleRadius * 2.0f;
                for (float r = accretionDiskInnerRadius; r <= accretionDiskOuterRadius; r += 1.0f)
                {
                    float alpha = 0.3f * (1.0f - (r - accretionDiskInnerRadius) / (accretionDiskOuterRadius - accretionDiskInnerRadius));
                    Color diskColor = (Color){255, 165, 0, (unsigned char)(alpha * 255)}; // Orange glow
                    DrawCircleLines(config.blackHoleCenter.x, config.blackHoleCenter.y, r, diskColor);
                }
            }
            Rectangle simBounds = {0, 0, SIMULATION_WIDTH, SIMULATION_HEIGHT};
            int lineThickness = 1;
            Color fillColor = Fade(BLACK, 0.1f);
            DrawRectangleRec(simBounds, fillColor);
            DrawRectangleLinesEx(simBounds, lineThickness, BLACK);
        }
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);
        DrawTextureRec(simulationTexture.texture, (Rectangle){0, 0, (float)simulationTexture.texture.width, (float)-simulationTexture.texture.height}, (Vector2){0, 0}, WHITE);
        EndMode2D();

        DrawRectangle(0, GetScreenHeight() - footerHeight, GetScreenWidth(), footerHeight, (Color){200, 200, 200, 255});

        GuiButton(pauseBtnRect, "Pause");
        GuiButton(resumeBtnRect, "Resume");
        GuiButton(resetBtnRect, "Reset");
        GuiButton(settingsBtnRect, "Settings");

        char zoomText[50];
        snprintf(zoomText, sizeof(zoomText), "%.2fx", camera.zoom);
        GuiLabel((Rectangle){GetScreenWidth() - 90, 10, 140, 30}, zoomText);

        char particleText[50];
        snprintf(particleText, sizeof(particleText), "Particles: %d", particles.count);
        GuiLabel((Rectangle){(int)(GetScreenWidth() / 2 - 50), 10, 200, buttonHeight}, particleText);

        if (showConfigPanel)
        {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 128});
            Rectangle configBounds = {0, 0, GetScreenWidth(), GetScreenHeight()};
            DrawRectangleRec(configBounds, RAYWHITE);
            Rectangle tabBarBounds = {0, 0, GetScreenWidth(), GetScreenHeight() * 0.05f};

            int closedTab = GuiTabBar(tabBarBounds, tabTexts, MAX_TABS, &activeTab);

            if (closedTab >= 0)
            {
                TraceLog(LOG_INFO, "Closed tab: %d", closedTab);
            }

            switch (activeTab)
            {
            case 0:
                generalTab(&gtp);
                break;
            case 1:
                patternsTab(&pstp);
                break;
            case 2:
                particlesTab(&ptp);
                break;
            case 3:
                explosionTab(&etp);
                break;
            case 4:
                virtualParticlesTab(&vtp);
                break;
            case 5:
                physicsTab(&phtp);
                break;
            default:
                break;
            }
            Rectangle closeBtnRect = {GetScreenWidth() / 2, GetScreenHeight() - 100, 100, 50};
            if (GuiButton(closeBtnRect, "Close"))
            {
                showConfigPanel = false;
            }
        }
        EndDrawing();
    }
}

int main(void)
{
    TraceLog(LOG_INFO, "Initializing window...");
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1500, 1500, "Particle Simulation");
    SetTargetFPS(60);
    TraceLog(LOG_INFO, "Window initialized.");

    TraceLog(LOG_INFO, "Loading fonts...");
    Font customFont = LoadFont("resources/UbuntuMonoNerdFont-Regular.ttf");
    if (customFont.texture.id == 0)
    {
        TraceLog(LOG_WARNING, "Failed to load custom font, using default font.");
        customFont = GetFontDefault();
    }
    GuiSetFont(customFont);
    TraceLog(LOG_INFO, "Fonts loaded.");

    camera = (Camera2D){0};
    camera.target = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    TraceLog(LOG_INFO, "Initializing simulation configurations...");
    Vector2 groupCenters[3] = {
        (Vector2){SIMULATION_WIDTH * 0.25f, SIMULATION_HEIGHT * 0.25f},
        (Vector2){SIMULATION_WIDTH * 0.75f, SIMULATION_HEIGHT * 0.25f},
        (Vector2){SIMULATION_WIDTH * 0.5f, SIMULATION_HEIGHT * 0.75f}};

    Vector2 blackHoleCenter = (Vector2){SIMULATION_WIDTH / 2.0f, SIMULATION_HEIGHT / 2.0f};

    config = (SimulationConfig){
        .initialPattern = PATTERN_RANDOM,
        .vortexCenter = (Vector2){SIMULATION_WIDTH / 2.0f, SIMULATION_HEIGHT / 2.0f},
        .vortexStrength = 5.0f,
        .radialStrength = 50.0f,
        .vortexRadius = 150.0f,
        .numGroups = 3,
        .groupCenters = groupCenters,
        .particlesPerGroup = 150,
        .blackHoleCenter = blackHoleCenter,
        .blackHoleMass = 1000.0f,
        .blackHoleRadius = 50.0f,
        .maxParticles = 1000,
        .initialCapacity = 500,
        .lifetime = false,
        .fragmentParticlesLive = false,
        .virtualParticles = false,
        .friction = 1,
        .minParticleLifeTime = 50,
        .maxParticleLifeTime = 250,
        .minParticleSpeed = 5,
        .maxParticleSpeed = 25,
        .minParticleMass = 1,
        .maxParticleMass = 100,
        .trailLength = 5,
        .minExplosionParticles = 0,
        .maxExplosionParticles = 5,
        .minVirtualParticleSpeed = 5,
        .maxVirtualParticleSpeed = 25,
        .minVirtualParticleLifeTime = 5,
        .maxVirtualParticleLifeTime = 25,
        .minTimeBetweenVirtualPairs = 5.0f,
        .g = 9.81f,
        .gUniversal = 6.67430e-11f,
        .maxGravityDistance = 50.0f,
        .kElectric = 5.0f,
        .maxForce = 10.0f,
        .gravityType = GRAVITY_NONE,
        .electricForce = false};

    particles = (Particles){
        .items = (Particle *)malloc(config.maxParticles * sizeof(Particle)),
        .count = 0,
        .capacity = config.initialCapacity,
    };

    gtp = (GeneralTabParameters){0};
    pstp = (PatternsTabParameters){0};
    ptp = (ParticlesTabParameters){0};
    etp = (ExplosionTabParameters){0};
    vtp = (VirtualParticlesTabParameters){0};
    phtp = (PhysicsTabParameters){0};
    initGuiParamaters(&config, &gtp, &pstp, &ptp, &etp, &vtp, &phtp);

    target = LoadRenderTexture(800, 800);
    simulationTexture = LoadRenderTexture(SIMULATION_WIDTH, SIMULATION_HEIGHT);

    gridWidth = ((SIMULATION_WIDTH + CELL_SIZE - 1) / CELL_SIZE);
    gridHeight = ((SIMULATION_HEIGHT + CELL_SIZE - 1) / CELL_SIZE);
    InitGrid(&config, gridWidth, gridHeight);
    InitParticles(&config, &particles, SIMULATION_WIDTH, SIMULATION_HEIGHT - footerHeight);

    buttonWidth = GetScreenWidth() * 0.05;
    buttonHeight = GetScreenHeight() * 0.025;
    footerHeight = buttonHeight + (int)(GetScreenHeight() * 0.03f);

    float totalWidth = 4 * buttonWidth + 3 * 10;
    float startX = (GetScreenWidth() - totalWidth) / 2.0f;
    float centerY = GetScreenHeight() - footerHeight / 2.0f - buttonHeight / 2.0f;

    pauseBtnRect = (Rectangle){startX, centerY, buttonWidth, buttonHeight};
    resumeBtnRect = (Rectangle){startX + (buttonWidth + 10), centerY, buttonWidth, buttonHeight};
    resetBtnRect = (Rectangle){startX + 2 * (buttonWidth + 10), centerY, buttonWidth, buttonHeight};
    settingsBtnRect = (Rectangle){startX + 3 * (buttonWidth + 10), centerY, buttonWidth, buttonHeight};

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(main_loop_web, 0, 1);
#else
    while (!WindowShouldClose())
    {
        main_loop_desktop();
    }
#endif
    CleanupSimulation(&particles, gridWidth, gridHeight);
    FreeGrid(gridWidth, gridHeight);
    UnloadRenderTexture(simulationTexture);
    CloseWindow();

    return 0;
}

/*-----------------------------------------------------------------------*/
int main2(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GetScreenWidth(), GetScreenHeight(), "Particle Simulation");
    SetTargetFPS(60);
    Font customFont = LoadFont("/usr/share/fonts/TTF/UbuntuMonoNerdFont-Regular.ttf");
    if (customFont.texture.id == 0)
    {
        customFont = GetFontDefault();
    }
    GuiSetFont(customFont);

    Camera2D camera = {0};
    camera.target = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    bool showConfigPanel = false;
    bool paused = false;

    int screenWidth = GetScreenWidth();
    float scaleFactor = (float)screenWidth / 800;
    int baseFontSize = 8;
    int scaledFontSize = (int)(baseFontSize * scaleFactor);
    float gridWidth = ((GetScreenWidth() + CELL_SIZE - 1) / CELL_SIZE);
    float gridHeight = ((GetScreenHeight() + CELL_SIZE - 1) / CELL_SIZE);

    int activeTab = 0;

    // Simulation configuration
    Vector2 groupCenters[3] = {
        (Vector2){SIMULATION_WIDTH * 0.25f, SIMULATION_HEIGHT * 0.25f},
        (Vector2){SIMULATION_WIDTH * 0.75f, SIMULATION_HEIGHT * 0.25f},
        (Vector2){SIMULATION_WIDTH * 0.5f, SIMULATION_HEIGHT * 0.75f}};

    Vector2 blackHoleCenter = (Vector2){SIMULATION_WIDTH / 2.0f, SIMULATION_HEIGHT / 2.0f};

    SimulationConfig config = {
        // Particle initialization pattern
        .initialPattern = PATTERN_RANDOM,
        .vortexCenter = (Vector2){SIMULATION_WIDTH / 2.0f, SIMULATION_HEIGHT / 2.0f},
        .vortexStrength = 5.0f,
        .radialStrength = 50.0f,
        .vortexRadius = 150.0f,

        .numGroups = 3,
        .groupCenters = groupCenters,
        .particlesPerGroup = 150,

        .blackHoleCenter = blackHoleCenter,
        .blackHoleMass = 1000.0f,
        .blackHoleRadius = 50.0f,

        // GENERAL
        .maxParticles = 1000,
        .initialCapacity = 500,
        .lifetime = false,
        .fragmentParticlesLive = false,
        .virtualParticles = false,
        .friction = 1,

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

    GeneralTabParameters gtp = {0};
    PatternsTabParameters pstp = {0};
    ParticlesTabParameters ptp = {0};
    ExplosionTabParameters etp = {0};
    VirtualParticlesTabParameters vtp = {0};
    PhysicsTabParameters phtp = {0};
    initGuiParamaters(&config, &gtp, &pstp, &ptp, &etp, &vtp, &phtp);

    // Shader glowShader = LoadShader(0, "resources/glow.fs");

    RenderTexture2D target = LoadRenderTexture(SIMULATION_WIDTH, SIMULATION_HEIGHT);
    RenderTexture2D simulationTexture = LoadRenderTexture(SIMULATION_WIDTH, SIMULATION_HEIGHT);

    const char *tabTexts[MAX_TABS] = {"General", "Patterns", "Particles", "Explosions", "Virtual Particles", "Physics"};

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
    const float ZOOM_INCREMENT = 0.1f; // Increment per scroll
    const float ZOOM_MIN = 0.1f;       // Minimum zoom
    const float ZOOM_MAX = 10.0f;      // Maximum zoom

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
            if (camera.zoom < ZOOM_MIN)
                camera.zoom = ZOOM_MIN;
            if (camera.zoom > ZOOM_MAX)
                camera.zoom = ZOOM_MAX;
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
            // BeginShaderMode(glowShader);
            DrawTextureRec(target.texture, (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height}, (Vector2){0, 0}, WHITE);
            // EndShaderMode();

            if (config.initialPattern == PATTERN_BLACKHOLE)
            {
                // Draw the event horizon
                DrawCircleV(config.blackHoleCenter, config.blackHoleRadius, BLACK);
                DrawCircleLines(config.blackHoleCenter.x, config.blackHoleCenter.y, config.blackHoleRadius, GRAY);

                // Glow effect around the black hole
                float glowRadius = config.blackHoleRadius * 1.5f;
                float glowThickness = 2.0f;
                for (float r = config.blackHoleRadius; r <= glowRadius; r += glowThickness)
                {
                    float alpha = 0.5f * (1.0f - (r - config.blackHoleRadius) / (glowRadius - config.blackHoleRadius));
                    Color glowColor = (Color){128, 128, 128, (unsigned char)(alpha * 255)};
                    DrawCircleLines(config.blackHoleCenter.x, config.blackHoleCenter.y, r, glowColor);
                }

                // Accretion disk effect
                float accretionDiskInnerRadius = config.blackHoleRadius * 1.1f;
                float accretionDiskOuterRadius = config.blackHoleRadius * 2.0f;
                for (float r = accretionDiskInnerRadius; r <= accretionDiskOuterRadius; r += 1.0f)
                {
                    float alpha = 0.3f * (1.0f - (r - accretionDiskInnerRadius) / (accretionDiskOuterRadius - accretionDiskInnerRadius));
                    Color diskColor = (Color){255, 165, 0, (unsigned char)(alpha * 255)}; // Orange glow
                    DrawCircleLines(config.blackHoleCenter.x, config.blackHoleCenter.y, r, diskColor);
                }
            }
            Rectangle simBounds = {0, 0, SIMULATION_WIDTH, SIMULATION_HEIGHT};
            int lineThickness = 1;
            Color fillColor = Fade(BLACK, 0.1f);
            DrawRectangleRec(simBounds, fillColor);
            DrawRectangleLinesEx(simBounds, lineThickness, BLACK);
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
        snprintf(zoomText, sizeof(zoomText), "%.2fx", camera.zoom);
        GuiLabel((Rectangle){GetScreenWidth() - 90, 10, 140, 30}, zoomText);

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
                generalTab(&gtp);
                break;
            case 1:
                patternsTab(&pstp);
                break;
            case 2:
                particlesTab(&ptp);
                break;
            case 3:
                explosionTab(&etp);
                break;
            case 4:
                virtualParticlesTab(&vtp);
                break;
            case 5:
                physicsTab(&phtp);
                break;
            default:
                break;
            }

            // Add a button to close the configuration menu
            Rectangle closeBtnRect = {GetScreenWidth() / 2, GetScreenHeight() - 100, 100, 50};
            if (GuiButton(closeBtnRect, "Close"))
            {
                showConfigPanel = false;
            }
        }
        EndDrawing();
    }

    CleanupSimulation(&particles, gridWidth, gridHeight);
    FreeGrid(gridWidth, gridHeight);
    UnloadRenderTexture(simulationTexture);
    CloseWindow();

    return 0;
}
