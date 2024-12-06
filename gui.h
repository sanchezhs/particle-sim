#ifndef GUI_H
#define GUI_H

#include "simulation.h"

#define MAX_TABS 5


void generalTab(int *maxParticles, int *initialCapacity, int *seed, bool *lifetime, bool *fragmentParticlesLive, bool *virtualParticles);
void particlesTab(float *minParticleLifeTime, float *maxParticleLifeTime, float *minParticleSpeed, float *maxParticleSpeed, float *minParticleMass, float *maxParticleMass, float *trailLength);
void explosionTab(int *minExplosionParticles, int *maxExplosionParticles);
void virtualParticlesTab(int *minVirtualParticleSpeed, int *maxVirtualParticleSpeed, int *minVirtualParticleLifeTime, int *maxVirtualParticleLifeTime, float *minTimeBetweenVirtualPairs);
void physicsTab(float *g, float *gUniversal, float *maxGravityDistance, float *kElectric, float *maxForce, GravityType *gravityType, bool *electricForce, bool *dropDownOpen);


#endif // GUI_H
