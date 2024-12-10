#define RAYGUI_IMPLEMENTATION

#if defined(PLATFORM_WEB)
#include <emscripten.h>
#endif

#include "raygui.h"
#include "external/raylib/src/raymath.h"
#include "simulation.h"
#include "cJSON.h"
#include "gui.h"

#define SIMULATION_WIDTH 10000
#define SIMULATION_HEIGHT 10000

bool paused = false;
int activeTab = 0;

// Hash table for spatial partitioning
HashTable *table = NULL;

// Camera
Camera2D camera;
bool showConfigPanel = false;
static bool rightMousePressed = false;
static Vector2 previousMousePosition = {0};
const float ZOOM_INCREMENT = 0.1f;
const float ZOOM_MIN = 0.1f;
const float ZOOM_MAX = 10.0f;

float gridWidth = 0.0f;
float gridHeight = 0.0f;
float gridSpacing = 250.f;
Color gridColor = (Color) {.r = 248, .g = 249, .b = 249, .a = 25};
Color originColor = GRAY;

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

// Simulation parameters
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
    ResetSimulation(&config, &particles, table, SIMULATION_WIDTH, SIMULATION_HEIGHT - footerHeight);
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

    // particlesDie (corresponding to lifetime)
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
        {
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
        {
            groupCount = 100;
        }
        config.numGroups = groupCount;
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

void drawInfiniteGrid(Camera2D camera, int screenWidth, int screenHeight, float gridSpacing, Color gridColor, Color originColor)
{
    Vector2 topLeft = GetScreenToWorld2D((Vector2){0, 0}, camera);
    Vector2 bottomRight = GetScreenToWorld2D((Vector2){(float)screenWidth, (float)screenHeight}, camera);

    float firstVertical = floor(topLeft.x / gridSpacing) * gridSpacing;
    float lastVertical = ceil(bottomRight.x / gridSpacing) * gridSpacing;

    for (float x = firstVertical; x <= lastVertical; x += gridSpacing)
    {
        Color lineColor = gridColor;
        if (fabsf(x) < 0.01f)
            lineColor = originColor;
        DrawLineV((Vector2){x, topLeft.y}, (Vector2){x, bottomRight.y}, lineColor);
    }

    float firstHorizontal = floor(topLeft.y / gridSpacing) * gridSpacing;
    float lastHorizontal = ceil(bottomRight.y / gridSpacing) * gridSpacing;

    for (float y = firstHorizontal; y <= lastHorizontal; y += gridSpacing)
    {
        Color lineColor = gridColor;
        if (fabsf(y) < 0.01f)
            lineColor = originColor;
        DrawLineV((Vector2){topLeft.x, y}, (Vector2){bottomRight.x, y}, lineColor);
    }
}


void updateCamera(float deltaTime) {
    int cameraSpeed = 750;
    if (IsKeyDown(KEY_W)) camera.target.y -= cameraSpeed * deltaTime;
    if (IsKeyDown(KEY_S)) camera.target.y += cameraSpeed * deltaTime;
    if (IsKeyDown(KEY_A)) camera.target.x -= cameraSpeed * deltaTime;
    if (IsKeyDown(KEY_D)) camera.target.x += cameraSpeed * deltaTime;

    if (IsKeyDown(KEY_Q)) camera.zoom += 0.5f * deltaTime;
    if (IsKeyDown(KEY_E)) camera.zoom -= 0.5f * deltaTime;

    if (camera.zoom < 0.1f) camera.zoom = 0.1f;
    if (camera.zoom > 3.0f) camera.zoom = 3.0f;
}

void main_loop_web(void)
{
    float delta = GetFrameTime();
    updateCamera(delta);
    GuiSetStyle(DEFAULT, TEXT_SIZE, (int)(8 * ((float)GetScreenWidth() / 800.0f)));

    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0)
    {
         float newZoom = camera.zoom + wheelMove * 0.1f;
        newZoom = fmax(newZoom, 0.1f);
        newZoom = fmin(newZoom, 10.0f);

        Vector2 mouseWorldBeforeZoom = GetScreenToWorld2D(GetMousePosition(), camera);
        camera.zoom = newZoom;
        Vector2 mouseWorldAfterZoom = GetScreenToWorld2D(GetMousePosition(), camera);
        Vector2 zoomOffset = Vector2Subtract(mouseWorldBeforeZoom, mouseWorldAfterZoom);
        camera.target = Vector2Add(camera.target, zoomOffset);
    }

    if (IsWindowResized())
    {
        UnloadRenderTexture(simulationTexture);
        UnloadRenderTexture(target);
        simulationTexture = LoadRenderTexture(SIMULATION_WIDTH, SIMULATION_HEIGHT);
        target = LoadRenderTexture(SIMULATION_WIDTH, SIMULATION_HEIGHT);
        camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
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

    if (!paused)
    {
        HandleInput(&config, &particles, GetScreenWidth(), GetScreenHeight());

        if (config.virtualParticles)
        {
            GenerateVirtualParticles(&config, &particles, delta, SIMULATION_WIDTH, SIMULATION_HEIGHT);
        }
        UpdateSimulation(&config, &particles, table, SIMULATION_WIDTH, SIMULATION_HEIGHT);
    }
    BeginDrawing();
        ClearBackground(BLACK);
            BeginMode2D(camera);
                drawInfiniteGrid(camera, GetScreenWidth(), GetScreenHeight(), gridSpacing, gridColor, originColor);
                RenderParticles(table, camera, CELL_SIZE);
        EndMode2D();

        char zoomText[50];
        snprintf(zoomText, sizeof(zoomText), "%.2fx", camera.zoom);
        GuiLabel((Rectangle){GetScreenWidth() - 90, 10, 140, 30}, zoomText);

        char particleText[50];
        snprintf(particleText, sizeof(particleText), "Particles: %d", particles.count);
        GuiLabel((Rectangle){(int)(GetScreenWidth() / 2 - 50), 10, 200, buttonHeight}, particleText);
    EndDrawing();
}

void main_loop_desktop(void)
{
    screenWidth = GetScreenWidth();
    scaleFactor = (float)screenWidth / 800;
    baseFontSize = 18;
    scaledFontSize = (int)(baseFontSize * scaleFactor);

    while (!WindowShouldClose())
    {
        float delta = GetFrameTime();
        updateCamera(delta);
        GuiSetStyle(DEFAULT, TEXT_SIZE, scaledFontSize);
        Vector2 mousePos = GetMousePosition();

        bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        float wheelMove = GetMouseWheelMove();
        if (wheelMove != 0)
        {
            float newZoom = camera.zoom + wheelMove * 0.1f;
            newZoom = fmax(newZoom, 0.1f);
            newZoom = fmin(newZoom, 10.0f);

            Vector2 mouseWorldBeforeZoom = GetScreenToWorld2D(GetMousePosition(), camera);
            camera.zoom = newZoom;
            Vector2 mouseWorldAfterZoom = GetScreenToWorld2D(GetMousePosition(), camera);
            Vector2 zoomOffset = Vector2Subtract(mouseWorldBeforeZoom, mouseWorldAfterZoom);
            camera.target = Vector2Add(camera.target, zoomOffset);
        }

        footerHeight = buttonHeight + (int)(GetScreenHeight() * 0.03f);
        int numButtons = 4;
        int spacing = 10;
        float totalWidth = numButtons * buttonWidth + (numButtons - 1) * spacing;
        float startX = (GetScreenWidth() - totalWidth) / 2.0f;
        float centerY = GetScreenHeight() - footerHeight / 2.0f - buttonHeight / 2.0f;

        pauseBtnRect = (Rectangle){startX, centerY, buttonWidth, buttonHeight};
        resumeBtnRect = (Rectangle){startX + (buttonWidth + spacing), centerY, buttonWidth, buttonHeight};
        resetBtnRect = (Rectangle){startX + 2 * (buttonWidth + spacing), centerY, buttonWidth, buttonHeight};
        settingsBtnRect = (Rectangle){startX + 3 * (buttonWidth + spacing), centerY, buttonWidth, buttonHeight};

        if (IsWindowResized())
        {
            UnloadRenderTexture(simulationTexture);
            UnloadRenderTexture(target);
            simulationTexture = LoadRenderTexture(SIMULATION_WIDTH, SIMULATION_HEIGHT);
            target = LoadRenderTexture(SIMULATION_WIDTH, SIMULATION_HEIGHT);
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
                ResetSimulation(&config, &particles, table, SIMULATION_WIDTH, SIMULATION_HEIGHT);
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

        // Pause simulation
        if (!showConfigPanel && !paused)
        {
            HandleInput(&config, &particles, GetScreenWidth(), GetScreenHeight() - footerHeight);

            if (config.virtualParticles)
            {
                GenerateVirtualParticles(&config, &particles, delta, SIMULATION_WIDTH, SIMULATION_HEIGHT - footerHeight);
            }
            UpdateSimulation(&config, &particles, table, SIMULATION_WIDTH, SIMULATION_HEIGHT);
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
        }
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode2D(camera);
                drawInfiniteGrid(camera, GetScreenWidth(), GetScreenHeight(), gridSpacing, gridColor, originColor);
                RenderParticles(table, camera, CELL_SIZE);
            EndMode2D();

        GuiButton(pauseBtnRect, "Pause");
        GuiButton(resumeBtnRect, "Resume");
        GuiButton(resetBtnRect, "Reset");
        GuiButton(settingsBtnRect, "Settings");

        char zoomText[50];
        snprintf(zoomText, sizeof(zoomText), "%.2fx", camera.zoom);
        GuiLabel((Rectangle){GetScreenWidth() - 90, 10, 140, 30}, zoomText);

        char particleText[50];
        snprintf(particleText, sizeof(particleText), "Particles: %d", particles.count);
        GuiLabel((Rectangle){GetScreenWidth() / 2, 10, 200, buttonHeight}, particleText);

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
    InitWindow(800, 800, "Particle Simulation");
    SetTargetFPS(60);
    TraceLog(LOG_INFO, "Window initialized.");

    table = InitHashTable();

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
    InitParticles(&config, &particles, SIMULATION_WIDTH, SIMULATION_HEIGHT);

    buttonWidth = GetScreenWidth() * 0.15;
    buttonHeight = GetScreenHeight() * 0.05;
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
    FreeHashTable(table);
    UnloadRenderTexture(simulationTexture);
    CloseWindow();

    return 0;
}
