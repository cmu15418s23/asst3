#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include "world.h"

// NOTE: Do not remove or edit funcations and variables in this class definition
class QuadTreeNode {
public:
  bool isLeaf = 0;

  // four child nodes are stored in following order:
  //  x0, y0 --------------- x1, y0
  //    |           |           |
  //    |children[0]|children[1]|
  //    | ----------+---------  |
  //    |children[2]|children[3]|
  //    |           |           |
  //  x0, y1 ----------------- x1, y1
  // where x0 < x1 and y0 < y1.

  std::unique_ptr<QuadTreeNode> children[4];

  std::vector<Particle> particles;
};

// NOTE: Do not remove or edit funcations and variables in this class definition
// but you may add more for optimization/debugging/measuring
class QuadTree : public AccelerationStructure {
public:
  std::unique_ptr<QuadTreeNode> root = nullptr;
  // the bounds of all particles
  Vec2 bmin, bmax;
  virtual void getParticles(std::vector<Particle> &particles, Vec2 position,
                            float radius) override;
  virtual void showStructure(Image &image, float viewportRadius) override;
  bool checkTree();
};

inline float boxPointDistance(Vec2 bmin, Vec2 bmax, Vec2 p) {
  float dx = fmaxf(fmaxf(bmin.x - p.x, p.x - bmax.x), 0.0f);
  float dy = fmaxf(fmaxf(bmin.y - p.y, p.y - bmax.y), 0.0f);
  return sqrt(dx * dx + dy * dy);
}

#endif
