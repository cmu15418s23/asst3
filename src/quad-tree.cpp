#include "quad-tree.h"
#include <algorithm>
#include <iostream>

// NOTE: You do not need to modify this function but you are welcome to optomize
// it if you wish. Do not change the function defintions.

void getParticlesImpl(std::vector<Particle> &particles, QuadTreeNode *node,
                      Vec2 bmin, Vec2 bmax, Vec2 position, float radius) {
  if (node->isLeaf) {
    for (auto &p : node->particles)
      if ((position - p.position).length() < radius)
        particles.push_back(p);
    return;
  }
  Vec2 pivot = (bmin + bmax) * 0.5f;
  Vec2 size = (bmax - bmin) * 0.5f;
  int containingChild =
      (position.x < pivot.x ? 0 : 1) + ((position.y < pivot.y ? 1 : 0) << 1);
  for (int i = 0; i < 4; i++) {
    Vec2 childBMin;
    childBMin.x = (i & 1) ? pivot.x : bmin.x;
    childBMin.y = ((i >> 1) & 1) ? pivot.y : bmin.y;
    Vec2 childBMax = childBMin + size;
    if (boxPointDistance(childBMin, childBMax, position) <= radius)
      getParticlesImpl(particles, node->children[i].get(), childBMin, childBMax,
                       position, radius);
  }
}

// NOTE: Do not modify any of this functions.

void QuadTree::getParticles(std::vector<Particle> &particles, Vec2 position,
                            float radius) {
  getParticlesImpl(particles, root.get(), bmin, bmax, position, radius);
}

bool checkNode(QuadTreeNode *node, Vec2 bmin, Vec2 bmax) {
  if (!node) {
    std::cout << "a null node" << std::endl;
    return false;
  }

  const float delta = 1e-4f;
  if (node->isLeaf) {
    for (auto p : node->particles) {
      if (p.position.x > bmax.x + delta || p.position.y > bmax.y + delta ||
          p.position.x < bmin.x - delta || p.position.y < bmin.y - delta) {
        std::cout << "particle: " << p.id << "(" << p.position.x << ", "
                  << p.position.y << ")"
                  << " outside of "
                  << "min: (" << bmin.x << ", " << bmin.y << ")"
                  << " "
                  << "max: (" << bmax.x << ", " << bmax.y << ")" << std::endl;
        return false;
      }
    }
  } else {
    Vec2 pivot = (bmin + bmax) * 0.5f;
    Vec2 size = (bmax - bmin) * 0.5f;
    for (int i = 0; i < 4; i++) {
      Vec2 childBMin;
      childBMin.x = (i & 1) ? pivot.x : bmin.x;
      childBMin.y = ((i >> 1) & 1) ? pivot.y : bmin.y;
      if (!checkNode(node->children[i].get(), childBMin, childBMin + size)) {
        return false;
      }
    }
  }
  return true;
}

bool QuadTree::checkTree() { return checkNode(root.get(), bmin, bmax); }

void showNode(QuadTreeNode *node, Image &image, float viewportRadius, Vec2 bmin,
              Vec2 bmax) {
  float invViewportSize = 0.5f / viewportRadius;
  Vec2 boxMin, boxMax;
  boxMin.x = (int)((bmin.x + viewportRadius) * invViewportSize * image.width);
  boxMax.x = (int)((bmax.x + viewportRadius) * invViewportSize * image.width);
  boxMin.y = (int)((bmin.y + viewportRadius) * invViewportSize * image.height);
  boxMax.y = (int)((bmax.y + viewportRadius) * invViewportSize * image.height);
  image.drawRectangle(boxMin, boxMax);
  // draw children
  if (!node->isLeaf) {
    Vec2 pivot = (bmin + bmax) * 0.5f;
    Vec2 size = (bmax - bmin) * 0.5f;
    for (int i = 0; i < 4; i++) {
      Vec2 childBMin;
      childBMin.x = (i & 1) ? pivot.x : bmin.x;
      childBMin.y = ((i >> 1) & 1) ? pivot.y : bmin.y;
      showNode(node->children[i].get(), image, viewportRadius, childBMin,
               childBMin + size);
    }
  }
}

void QuadTree::showStructure(Image &image, float viewportRadius) {
  showNode(root.get(), image, viewportRadius, bmin, bmax);
}
