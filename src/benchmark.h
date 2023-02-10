#include "timing.h"
#include "world.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

/*              OUTPUT FUNCTIONS               */
void displayIterationPerformance(int step, TimeCost timeCost);
void displayTotalPerformance(int step, TimeCost timeCost);

/*            CORRECTNESS FUNCTIONS            */
bool checkForCorrectness(std::string implementation, const World &refW,
                         const World &w, std::string referenceAnswerDir,
                         int numParticles, StepParameters stepParams);
