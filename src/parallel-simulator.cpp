#include "quad-tree.h"
#include "world.h"
#include <algorithm>
#include <iostream>

// TASK 2

// NOTE: You may modify this class definition as you see fit, as long as the
// class name, and type of simulateStep and buildAccelerationStructure remain
// the same. You may modify any code outside this class unless otherwise
// specified.

const int QuadTreeLeafSize = 8;
class ParallelNBodySimulator : public INBodySimulator {
public:
  // TODO: implement a function that builds and returns a quadtree containing
  // particles. You do not have to preserve this function type.
  std::unique_ptr<QuadTreeNode> buildQuadTree(std::vector<Particle> &particles,
                                              Vec2 bmin, Vec2 bmax) {
    return nullptr;
  }

  // Do not modify this function type.
  virtual std::unique_ptr<AccelerationStructure>
  buildAccelerationStructure(std::vector<Particle> &particles) {
    // build quad-tree
    auto quadTree = std::make_unique<QuadTree>();

    // find bounds
    Vec2 bmin(1e30f, 1e30f);
    Vec2 bmax(-1e30f, -1e30f);

    for (auto &p : particles) {
      bmin.x = fminf(bmin.x, p.position.x);
      bmin.y = fminf(bmin.y, p.position.y);
      bmax.x = fmaxf(bmax.x, p.position.x);
      bmax.y = fmaxf(bmax.y, p.position.y);
    }

    quadTree->bmin = bmin;
    quadTree->bmax = bmax;

    // build nodes
    quadTree->root = buildQuadTree(particles, bmin, bmax);
    if (!quadTree->checkTree()) {
      std::cout << "Your Tree has Error!" << std::endl;
    }

    return quadTree;
  }

  // Do not modify this function type.
  virtual void simulateStep(AccelerationStructure *accel,
                            std::vector<Particle> &particles,
                            std::vector<Particle> &newParticles,
                            StepParameters params) override {
    // TODO: implement parallel version of quad-tree accelerated n-body
    // simulation here, using quadTree as acceleration structure
  }
};

// Do not modify this function type.
std::unique_ptr<INBodySimulator> createParallelNBodySimulator() {
  return std::make_unique<ParallelNBodySimulator>();
}
