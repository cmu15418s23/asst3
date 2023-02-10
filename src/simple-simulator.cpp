#include "world.h"

class SimpleNBodySimulator : public INBodySimulator {
public:
  std::vector<Particle> nearbyParticles;

  virtual std::unique_ptr<AccelerationStructure>
  buildAccelerationStructure(std::vector<Particle> & /*particles*/) override {
    // don't build any acceleration structures
    return nullptr;
  }

  virtual void simulateStep(AccelerationStructure *accel,
                            std::vector<Particle> &particles,
                            std::vector<Particle> &newParticles,
                            StepParameters params) override {
#pragma omp parallel for
    for (int i = 0; i < (int)particles.size(); i++) {
      auto pi = particles[i];
      Vec2 force = Vec2(0.0f, 0.0f);
      // accumulate attractive forces to apply to particle i
      for (size_t j = 0; j < particles.size(); j++) {
        if (j == i)
          continue;
        if ((pi.position - particles[j].position).length() < params.cullRadius)
          force += computeForce(pi, particles[j], params.cullRadius);
      }
      // update particle state using the computed force
      newParticles[i] = updateParticle(pi, force, params.deltaTime);
    }
  }
};

std::unique_ptr<INBodySimulator> createSimpleNBodySimulator() {
  return std::make_unique<SimpleNBodySimulator>();
}
