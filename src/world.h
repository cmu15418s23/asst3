#ifndef NBODY_WORLD_H
#define NBODY_WORLD_H

#include <math.h>
#include <memory>
#include <vector>

struct Vec2 {
  float x, y;
  Vec2(float vx = 0.0f, float vy = 0.0f) : x(vx), y(vy) {}
  static float dot(Vec2 v0, Vec2 v1) { return v0.x * v1.x + v0.y * v1.y; }
  float &operator[](int i) { return ((float *)this)[i]; }
  Vec2 operator*(float s) const { return Vec2(*this) *= s; }
  Vec2 operator*(Vec2 vin) const { return Vec2(*this) *= vin; }
  Vec2 operator+(Vec2 vin) const { return Vec2(*this) += vin; }
  Vec2 operator-(Vec2 vin) const { return Vec2(*this) -= vin; }
  Vec2 operator-() const { return Vec2(-x, -y); }
  Vec2 &operator+=(Vec2 vin) {
    x += vin.x;
    y += vin.y;
    return *this;
  }
  Vec2 &operator-=(Vec2 vin) {
    x -= vin.x;
    y -= vin.y;
    return *this;
  }
  Vec2 &operator=(float v) {
    x = y = v;
    return *this;
  }
  Vec2 &operator*=(float s) {
    x *= s;
    y *= s;
    return *this;
  }
  Vec2 &operator*=(Vec2 vin) {
    x *= vin.x;
    y *= vin.y;
    return *this;
  }
  float length2() const { return x * x + y * y; }
  float length() const { return sqrt(length2()); }
};

struct Particle {
  int id;
  float mass;
  Vec2 position;
  Vec2 velocity;
};

// Do not modify this function.
inline Vec2 computeForce(const Particle &target, const Particle &attractor,
                         float cullRadius) {
  auto dir = (attractor.position - target.position);
  auto dist = dir.length();
  if (dist < 1e-3f)
    return Vec2(0.0f, 0.0f);
  dir *= (1.0f / dist);
  if (dist > cullRadius)
    return Vec2(0.0f, 0.0f);
  if (dist < 1e-1f)
    dist = 1e-1f;
  const float G = 0.01f;
  Vec2 force = dir * target.mass * attractor.mass * (G / (dist * dist));
  if (dist > cullRadius * 0.75f) {
    float decay = 1.0f - (dist - cullRadius * 0.75f) / (cullRadius * 0.25f);
    force *= decay;
  }
  return force;
}

inline Particle updateParticle(const Particle &pi, Vec2 force,
                               float deltaTime) {
  Particle result = pi;
  result.velocity += force * (deltaTime / pi.mass);
  result.position += result.velocity * deltaTime;
  return result;
}

struct StepParameters {
  float deltaTime = 0.2f;
  float cullRadius = 1.0f;
};

struct Pixel {
  unsigned char r, g, b, a;
};

struct Image {
  int width = 0, height = 0;
  std::vector<Pixel> pixels;
  void setSize(int w, int h);
  void clear();
  void drawRectangle(Vec2 bmin, Vec2 bmax);
  void fillRectangle(int x, int y, int size);
  void saveToFile(std::string fileName);
};

class AccelerationStructure {
public:
  virtual void getParticles(std::vector<Particle> &particles, Vec2 position,
                            float radius) {}
  virtual void showStructure(Image &image, float viewportRadius){};
  virtual ~AccelerationStructure() {}
};

class INBodySimulator {
public:
  virtual std::unique_ptr<AccelerationStructure>
  buildAccelerationStructure(std::vector<Particle> &particles) = 0;
  virtual void simulateStep(AccelerationStructure *accel,
                            std::vector<Particle> &particles,
                            std::vector<Particle> &newParticles,
                            StepParameters params) = 0;
  virtual ~INBodySimulator() {}
};

std::unique_ptr<INBodySimulator> createSimpleNBodySimulator();
std::unique_ptr<INBodySimulator> createSequentialNBodySimulator();
std::unique_ptr<INBodySimulator> createParallelNBodySimulator();

struct TimeCost {
  double treeBuildingTime = 0, simulationTime = 0;
  double getTotal() { return treeBuildingTime + simulationTime; }
};

class World {
public:
  std::vector<Particle> particles;
  std::vector<Particle> newParticles;
  std::unique_ptr<INBodySimulator> nbodySimulator;

  void simulateStep(StepParameters params, TimeCost &times);
  bool loadFromFile(std::string fileName);
  void saveToFile(std::string fileName);
  void generateRandom(int numParticles, float spaceSize);
  void generateBigLittle(int numParticles, float spaceSize);
  void generateDiagonal(int numParticles, float spaceSize);
  void dumpView(std::string fileName, float viewportRadius);
};
#endif
