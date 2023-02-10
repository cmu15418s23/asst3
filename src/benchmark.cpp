#include "benchmark.h"

template <typename T> std::string toString(T val) {
  std::stringstream ss("");
  ss << val;
  return ss.str();
}

void displayIterationPerformance(int step, TimeCost timeCost) {
  printf("iteration %d, tree construction: %.6fs, simulation: %.6fs\n", step,
         timeCost.treeBuildingTime, timeCost.simulationTime);
}

// The formatting of this helps the perl script parse times; do not modify
void displayTotalPerformance(int steps, TimeCost timeCost) {
  printf("TOTAL TIME: %.6fs\ntotal tree construction time: %.6fs\ntotal "
         "simulation time: %.6fs\n",
         timeCost.treeBuildingTime + timeCost.simulationTime,
         timeCost.treeBuildingTime, timeCost.simulationTime);
}

// checks the world w against the reference world provided. must have completed
// the same number of steps.
bool checkForCorrectness(std::string implementation, const World &refW,
                         const World &w, std::string referenceAnswerDir,
                         int numParticles, StepParameters stepParams) {
  if (w.particles.size() != refW.particles.size()) {
    std::cout << implementation << " -- Mismatch: number of particles "
              << w.particles.size()
              << " does not match reference particle number "
              << refW.particles.size() << "\n";
    return false;
  }

  for (size_t i = 0; i < w.particles.size(); i++) {
    auto errorX = abs(w.particles[i].position.x - refW.particles[i].position.x);
    auto errorY = abs(w.particles[i].position.y - refW.particles[i].position.y);
    if (errorX > 1e-2f || errorY > 1e-2f) {
      std::cout << implementation
                << " -- Mismatch: found correctness error at index " << i
                << ", result " << w.particles[i].position.x << ", "
                << w.particles[i].position.y << ", should be "
                << refW.particles[i].position.x << ", "
                << refW.particles[i].position.y << "\n";
      return false;
    }
  }

  return true;
}
