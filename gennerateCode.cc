#include <iostream>
#include <fstream>
#include "DefaultParam.h"
#include <string>
#include <map>
using namespace std;

int main(){
    string outDir="/home/yangjinhao/GeNN/userproject/MultiArea/generateCoda.txt";
    ofstream file(outDir);
    const string* AreaList=ParaMeters::areaNames;
    const string* PopList=ParaMeters::populationNames;
    for(int area=0;area<ParaMeters::AreaMax;area++){
        for(int pop=0;pop<ParaMeters::PopulationMax;pop++){
            string popName=ParaMeters::getPopName(area,pop);
            file<<"pull"<<popName<<"CurrentSpikesFromDevice();\n";
            file<<"IntoFile(glbSpk"<<popName<<",glbSpkCnt"<<popName<<",output[\""<<AreaList[area]<<"\"][\""<<PopList[pop]<<"\"]);\n";
        }
    }
    file.close();
    return 0;
}