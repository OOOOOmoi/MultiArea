#include <sys/stat.h>
#include <sys/types.h>
#include "MultiArea_CODE/definitions.h"
#include <map>
#include <cmath>
#include "time.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>
#include <memory>
#include "RecordFunc.h"
#include "DefaultParam.h"
using namespace std;
#define TOTAL_TIME 1000.0f
#define REPORT_TIME 100.0f

int main(){
    string outDirSpike="/home/yangjinhao/GeNN/userproject/MultiArea/output";
    allocateMem();
    initialize();
    initializeSparse();
    struct stat st;
    const string* AreaList=ParaMeters::areaNames;
    const string* PopList=ParaMeters::populationNames;
    map<string,map<string,ofstream>> outPutSpikeMap;
    for (int area=0;area<ParaMeters::AreaMax;area++){
        for(int pop=0;pop<ParaMeters::PopulationMax;pop++){
            string outDir=outDirSpike+"/"+AreaList[area];
            if (stat(outDir.c_str(), &st) != 0) { // 如果目录不存在
                mkdir(outDir.c_str(), 0777);      // 创建目录
            }
            outDir=outDir+"/"+PopList[pop]+".st";
            outPutSpikeMap[AreaList[area]][PopList[pop]].open(outDir);
        }
    }
    cout<<"start Simulation"<<endl;
    clock_t start=clock();
    while(t<TOTAL_TIME){
        if(fmod(t, 100.0) == 0.0) {
            cout << t / 10.0 << "%" << endl;
        }
        stepTime();
        SaveSpike(outPutSpikeMap);
    }
    clock_t end=clock();
    cout<<"Simulation Complete, Time Using: "<<static_cast<double>(end-start)/CLOCKS_PER_SEC<<endl;
    return EXIT_SUCCESS;
}