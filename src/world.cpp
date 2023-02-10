#include "world.h"
#include "timing.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>

class Random {
private:
  unsigned int seed;

public:
  Random(int seed) : seed(seed) {}
  int Next() // random between 0 and RandMax (currently 0x7fff)
  {
    return (((seed = seed * 214013L + 2531011L) >> 16) & 0x7fff);
  }
  int Next(int min, int max) // inclusive min, exclusive max
  {
    unsigned int a = ((seed = seed * 214013L + 2531011L) & 0xFFFF0000);
    unsigned int b = ((seed = seed * 214013L + 2531011L) >> 16);
    unsigned int r = a + b;
    return min + r % (max - min);
  }
  float NextFloat() { return ((Next() << 15) + Next()) / ((float)(1 << 30)); }
  float NextFloat(float valMin, float valMax) {
    return valMin + (valMax - valMin) * NextFloat();
  }
  static int RandMax() { return 0x7fff; }
};

inline int clamp(int val, int lbound, int ubound) {
  return val < lbound ? lbound : val > ubound ? ubound : val;
}

void Image::setSize(int w, int h) {
  width = w;
  height = h;
  pixels.resize(w * h);
}

void Image::clear() {
  for (auto &p : pixels) {
    p.r = p.g = p.b = 0;
    p.a = 255;
  }
}

void Image::drawRectangle(Vec2 bmin, Vec2 bmax) {
  int minX = clamp(bmin.x, 0, width - 1);
  int minY = clamp(bmin.y, 0, height - 1);
  int maxX = clamp(bmax.x, 0, width - 1);
  int maxY = clamp(bmax.y, 0, height - 1);
  Pixel highlight = {255, 0, 0, 255};
  for (int x = minX; x <= maxX; x++) {
    pixels[minY * width + x] = highlight;
    pixels[maxY * width + x] = highlight;
  }

  for (int y = minY; y < maxY; y++) {
    pixels[y * width + minX] = highlight;
    pixels[y * width + maxX] = highlight;
  }
}

void Image::fillRectangle(int x, int y, int size) {
  int minX = clamp(x - size, 0, width - 1);
  int minY = clamp(y - size, 0, height - 1);
  int maxX = clamp(x + size, 0, width - 1);
  int maxY = clamp(y + size, 0, height - 1);
  Pixel highlight = {255, 255, 255, 255};
  for (int y = minY; y <= maxY; y++)
    for (int x = minX; x <= maxX; x++) {
      pixels[y * width + x] = highlight;
    }
}

void Image::saveToFile(std::string fileName) {
  int filesize = 54 + 3 * width * height;
  std::vector<unsigned char> img;
  img.resize(3 * width * height);
  for (int j = 0; j < height; j++) {
    Pixel *scanLine = &pixels[0] + j * width;
    for (int i = 0; i < width; i++) {
      int x = i;
      int y = height - 1 - j;

      int r = scanLine[i].r;
      int g = scanLine[i].g;
      int b = scanLine[i].b;
      img[(x + y * width) * 3 + 2] = (unsigned char)(r);
      img[(x + y * width) * 3 + 1] = (unsigned char)(g);
      img[(x + y * width) * 3 + 0] = (unsigned char)(b);
    }
  }

  unsigned char bmpfileheader[14] = {'B', 'M', 0, 0,  0, 0, 0,
                                     0,   0,   0, 54, 0, 0, 0};
  unsigned char bmpinfoheader[40] = {40, 0, 0, 0, 0, 0, 0,  0,
                                     0,  0, 0, 0, 1, 0, 24, 0};
  unsigned char bmppad[3] = {0, 0, 0};

  bmpfileheader[2] = (unsigned char)(filesize);
  bmpfileheader[3] = (unsigned char)(filesize >> 8);
  bmpfileheader[4] = (unsigned char)(filesize >> 16);
  bmpfileheader[5] = (unsigned char)(filesize >> 24);

  bmpinfoheader[4] = (unsigned char)(width);
  bmpinfoheader[5] = (unsigned char)(width >> 8);
  bmpinfoheader[6] = (unsigned char)(width >> 16);
  bmpinfoheader[7] = (unsigned char)(width >> 24);
  bmpinfoheader[8] = (unsigned char)(height);
  bmpinfoheader[9] = (unsigned char)(height >> 8);
  bmpinfoheader[10] = (unsigned char)(height >> 16);
  bmpinfoheader[11] = (unsigned char)(height >> 24);

  std::ofstream file;
  file.open(fileName, std::ios::out | std::ios::binary);
  if (!file) {
    std::cout << "error writing file \"" << fileName << "\"" << std::endl;
    return;
  }
  file.write((const char *)bmpfileheader, 14);
  file.write((const char *)bmpinfoheader, 40);
  for (int i = 0; i < height; i++) {
    file.write((const char *)(&img[0] + (width * i * 3)), 3 * width);
    file.write((const char *)bmppad, (4 - (width * 3) % 4) % 4);
  }
  file.close();
}

// Do not modify this function. This simulates a single 'step' of nbody
// computation
void World::simulateStep(StepParameters params, TimeCost &times) {
  newParticles.resize(particles.size());
  Timer t;
  t.reset();
  auto tree = nbodySimulator->buildAccelerationStructure(particles);
  times.treeBuildingTime += t.elapsed();
  t.reset();
  nbodySimulator->simulateStep(tree.get(), particles, newParticles, params);
  times.simulationTime += t.elapsed();
  particles.swap(newParticles);
}

bool World::loadFromFile(std::string fileName) {
  std::ifstream inFile;
  inFile.open(fileName);
  if (!inFile) {
    return false;
  }

  std::string line;
  while (std::getline(inFile, line)) {
    Particle particle;
    std::stringstream sstream(line);
    std::string str;
    std::getline(sstream, str, ' ');
    particle.mass = (float)atof(str.c_str());
    std::getline(sstream, str, ' ');
    particle.position.x = (float)atof(str.c_str());
    std::getline(sstream, str, ' ');
    particle.position.y = (float)atof(str.c_str());
    std::getline(sstream, str, ' ');
    particle.velocity.x = (float)atof(str.c_str());
    std::getline(sstream, str, '\n');
    particle.velocity.y = (float)atof(str.c_str());
    particle.id = (int)particles.size();
    particles.push_back(particle);
  }
  inFile.close();
  newParticles.resize(particles.size());
  return true;
}

void World::saveToFile(std::string fileName) {
  std::ofstream file(fileName);
  if (!file) {
    std::cout << "error writing file \"" << fileName << "\"" << std::endl;
    return;
  }
  file << std::setprecision(9);
  for (auto p : particles) {
    file << p.mass << " " << p.position.x << " " << p.position.y << " "
         << p.velocity.x << " " << p.velocity.y << std::endl;
  }
  file.close();
  if (!file)
    std::cout << "error writing file \"" << fileName << "\"" << std::endl;
}

void World::generateRandom(int numParticles, float spaceSize) {
  float maxVelocity = spaceSize * 0.5f;
  Random random(2713);
  particles.resize(numParticles);
  newParticles.clear();
  newParticles.resize(numParticles);

  for (int i = 0; i < numParticles; i++) {
    particles[i].mass = random.NextFloat(1.0f, 10.0f);
    particles[i].velocity.x = random.NextFloat(-maxVelocity, maxVelocity);
    particles[i].velocity.y = random.NextFloat(-maxVelocity, maxVelocity);
    particles[i].position.x = random.NextFloat(-spaceSize, spaceSize);
    particles[i].position.y = random.NextFloat(-spaceSize, spaceSize);
    particles[i].id = i;
  }
}

void World::generateBigLittle(int numParticles, float spaceSize) {
  int largeNumParticles = numParticles / 3 * 2;
  int smallNumParticles = (numParticles - largeNumParticles) / 3;
  float maxVelocity = spaceSize * 0.5f;
  Random random(2713);
  particles.resize(numParticles);
  newParticles.clear();
  newParticles.resize(numParticles);

  // upper right = large cluster
  int i = 0;
  for (; i < largeNumParticles; i++) {
    particles[i].mass = random.NextFloat(1.0f, 10.0f);
    particles[i].velocity.x = random.NextFloat(-maxVelocity, maxVelocity);
    particles[i].velocity.y = random.NextFloat(-maxVelocity, maxVelocity);
    particles[i].position.x = random.NextFloat(0.0f, spaceSize);
    particles[i].position.y = random.NextFloat(0.0f, spaceSize);
    particles[i].id = i;
  }

  // upper left = small cluster
  for (; i < largeNumParticles + smallNumParticles; i++) {
    particles[i].mass = random.NextFloat(1.0f, 10.0f);
    particles[i].velocity.x = random.NextFloat(-maxVelocity, maxVelocity);
    particles[i].velocity.y = random.NextFloat(-maxVelocity, maxVelocity);
    particles[i].position.x = random.NextFloat(-spaceSize, 0.0f);
    particles[i].position.y = random.NextFloat(0.0f, spaceSize);
    particles[i].id = i;
  }

  // lower left = small cluster
  for (; i < largeNumParticles + smallNumParticles * 2; i++) {
    particles[i].mass = random.NextFloat(1.0f, 10.0f);
    particles[i].velocity.x = random.NextFloat(-maxVelocity, maxVelocity);
    particles[i].velocity.y = random.NextFloat(-maxVelocity, maxVelocity);
    particles[i].position.x = random.NextFloat(-spaceSize, 0.0f);
    particles[i].position.y = random.NextFloat(-spaceSize, 0.0f);
    particles[i].id = i;
  }

  // lower right = small cluster
  for (; i < numParticles; i++) {
    particles[i].mass = random.NextFloat(1.0f, 10.0f);
    particles[i].velocity.x = random.NextFloat(-maxVelocity, maxVelocity);
    particles[i].velocity.y = random.NextFloat(-maxVelocity, maxVelocity);
    particles[i].position.x = random.NextFloat(0.0f, spaceSize);
    particles[i].position.y = random.NextFloat(-spaceSize, 0.0f);
    particles[i].id = i;
  }
}

void World::generateDiagonal(int numParticles, float spaceSize) {
  float maxVelocity = spaceSize * 0.5f;
  Random random(2713);
  particles.resize(numParticles);
  newParticles.clear();
  newParticles.resize(numParticles);

  // control the center of point generation
  float diagonalCenter = -spaceSize;
  float range = spaceSize / 10;

  for (int i = 0; i < numParticles; i++) {
    // compute left and right bounds for point to be generated in
    float leftBound = diagonalCenter - range;
    float rightBound = diagonalCenter + range;

    // clamp values
    if (leftBound < -spaceSize)
      leftBound = -spaceSize;
    if (rightBound > spaceSize)
      rightBound = spaceSize;

    particles[i].mass = random.NextFloat(1.0f, 10.0f);
    particles[i].velocity.x = random.NextFloat(-maxVelocity, maxVelocity);
    particles[i].velocity.y = random.NextFloat(-maxVelocity, maxVelocity);
    particles[i].position.x = random.NextFloat(leftBound, rightBound);
    particles[i].position.y = random.NextFloat(leftBound, rightBound);
    particles[i].id = i;

    // wrap diagonal center
    diagonalCenter++;
    if (diagonalCenter > spaceSize) {
      diagonalCenter = -spaceSize;
    }
  }
}

void World::dumpView(std::string fileName, float viewportRadius) {
  Image image;
  const int imageSize = 512;
  image.setSize(imageSize, imageSize);
  image.clear();
  float invViewportSize = 0.5f / viewportRadius;
  for (auto &p : particles) {
    int x =
        (int)((p.position.x + viewportRadius) * invViewportSize * imageSize);
    int y =
        (int)((p.position.y + viewportRadius) * invViewportSize * imageSize);
    image.fillRectangle(x, y, 1);
  }

  // overlay visualization on to particle views
  if (nbodySimulator) {
    auto tree = nbodySimulator->buildAccelerationStructure(particles);
    if (tree) {
      tree->showStructure(image, viewportRadius);
    }
  }
  image.saveToFile(fileName);
}
