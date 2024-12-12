#pragma once
#include <map>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>
#include <memory>
#include "MultiArea_CODE/definitions.h"
using namespace std;

void IntoFile(unsigned int* Spk, unsigned int* SpkCnt, ofstream& file);
void SaveSpike(map<string,map<string,ofstream>>& outPuts);