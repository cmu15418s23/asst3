#include "benchmark.h"
#include "timing.h"
#include "world.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string.h>

#define NUM_ITERATIONS 5

enum class FrameOutputStyle { None, FinalFrameOnly, AllFrames };

enum class SimulatorType { Simple, Sequential, Parallel };

struct StartupOptions {
  int numIterations = 1;
  int numParticles = 5;
  float viewportRadius = 10.0f;
  float spaceSize = 10.0f;
  FrameOutputStyle frameOutputStyle = FrameOutputStyle::FinalFrameOnly;
  std::string outputFile = "out.txt";
  std::string bitmapOutputDir;
  std::string inputFile;
  SimulatorType simulatorType = SimulatorType::Simple;
  bool checkCorrectness = false;
  std::string referenceAnswerDir = "";
};

std::string removeQuote(std::string input) {
  if (input.length() > 0 && input.front() == '\"')
    return input.substr(1, input.length() - 2);
  return input;
}

StepParameters getBenchmarkStepParams(float spaceSize) {
  StepParameters result;
  result.cullRadius = spaceSize / 4.0f;
  result.deltaTime = 0.2f;
  return result;
}

StartupOptions parseOptions(int argc, const char **argv) {
  StartupOptions rs;
  for (int i = 1; i < argc; i++) {
    if (i < argc - 1) {
      if (strcmp(argv[i], "-i") == 0)
        rs.numIterations = atoi(argv[i + 1]);
      else if (strcmp(argv[i], "-s") == 0)
        rs.spaceSize = (float)atof(argv[i + 1]);
      else if (strcmp(argv[i], "-c") == 0)
        rs.checkCorrectness = true;
      else if (strcmp(argv[i], "-in") == 0)
        rs.inputFile = removeQuote(argv[i + 1]);
      else if (strcmp(argv[i], "-n") == 0)
        rs.numParticles = atoi(argv[i + 1]);
      else if (strcmp(argv[i], "-v") == 0)
        rs.viewportRadius = (float)atof(argv[i + 1]);
      else if (strcmp(argv[i], "-o") == 0)
        rs.outputFile = argv[i + 1];
      else if (strcmp(argv[i], "-fo") == 0) {
        rs.bitmapOutputDir = removeQuote(argv[i + 1]);
        rs.frameOutputStyle = FrameOutputStyle::AllFrames;
      } else if (strcmp(argv[i], "-ref") == 0)
        rs.referenceAnswerDir = removeQuote(argv[i + 1]);
    }
    if (strcmp(argv[i], "-par") == 0) {
      rs.simulatorType = SimulatorType::Parallel;
    } else if (strcmp(argv[i], "-simple") == 0) {
      rs.simulatorType = SimulatorType::Simple;
    } else if (strcmp(argv[i], "-seq") == 0) {
      rs.simulatorType = SimulatorType::Sequential;
    }
  }
  return rs;
}

int main(int argc, const char **argv) {
  StartupOptions options = parseOptions(argc, argv);

  World w;
  World refW;
  if (options.inputFile.length())
    w.loadFromFile(options.inputFile);
  else
    w.generateRandom(options.numParticles, options.spaceSize);
  w.saveToFile("reference-init.txt");
  // w.generateBigLittle(options.numParticles, options.spaceSize);

  if (options.checkCorrectness) {
    std::cout << "Correctness Checking Enabled";
    refW.nbodySimulator = createSimpleNBodySimulator();
    if (options.inputFile.length())
      refW.loadFromFile(options.inputFile);
    else
      refW.loadFromFile("reference-init.txt");
  }

  std::string simulatorName;
  switch (options.simulatorType) {
  case SimulatorType::Simple:
    w.nbodySimulator = createSimpleNBodySimulator();
    simulatorName = "Simple";
    break;
  case SimulatorType::Sequential:
    w.nbodySimulator = createSequentialNBodySimulator();
    simulatorName = "Sequential";
    break;
  case SimulatorType::Parallel:
    w.nbodySimulator = createParallelNBodySimulator();
    simulatorName = "Parallel";
    break;
  }
  std::cout << simulatorName << "\n";
  StepParameters stepParams;
  stepParams = getBenchmarkStepParams(options.spaceSize);

  // run the implementation
  bool fullCorrectness = true;
  TimeCost totalTimeCost;
  for (int i = 0; i < options.numIterations; i++) {
    TimeCost timeCost;
    TimeCost timeCostRef;
    w.simulateStep(stepParams, timeCost);
    totalTimeCost.treeBuildingTime += timeCost.treeBuildingTime;
    totalTimeCost.simulationTime += timeCost.simulationTime;
    if (options.checkCorrectness) {
      refW.simulateStep(stepParams, timeCostRef);
      bool correct = checkForCorrectness(simulatorName, refW, w, "",
                                         options.numParticles, stepParams);
      if (correct != true)
        fullCorrectness = false;
    }
    displayIterationPerformance(i, timeCost);

    // generate simulation image
    if (options.frameOutputStyle == FrameOutputStyle::AllFrames) {
      std::stringstream sstream;
      sstream << options.bitmapOutputDir;
      if (!options.bitmapOutputDir.size() ||
          (options.bitmapOutputDir.back() != '\\' &&
           options.bitmapOutputDir.back() != '/'))
        sstream << "/";
      sstream << i << ".bmp";
      w.dumpView(sstream.str(), options.viewportRadius);
    }
  }
  displayTotalPerformance(options.numIterations, totalTimeCost);

  if (options.outputFile.length()) {
    w.saveToFile(options.outputFile);
  }
  return !fullCorrectness;
}
