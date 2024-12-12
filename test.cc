#include<ctime>
#include "DefaultParam.h"
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <limits>
#include <chrono>
#include <iostream>
#include "../include/normalDistribution.h"
using namespace std;
const bool PoissionInput=false;
const bool SpikeRecord=false;
const char *FileWeight="/home/yangjinhao/GeNN/userproject/MultiArea/SynapsesWeight.txt";
const char *FileSynapseNumber="/home/yangjinhao/GeNN/userproject/MultiArea/SynapsesNumber.txt";
const char *FileNeuronNumber="/home/yangjinhao/GeNN/userproject/MultiArea/NeuronNumber.txt";

map<string,map<string,map<string,map<string,int>>>> getSynNum(){
    ifstream file(FileSynapseNumber, ios::in);
    string srcArea, tarArea, srcPop, tarPop;
    float synNum;
    map<string,map<string,map<string,map<string,int>>>> SynNumber;
    while(file>>tarArea>>tarPop>>srcArea>>srcPop>>synNum){
        SynNumber[tarArea][tarPop][srcArea][srcPop]=static_cast<int>(synNum);
    }
    return SynNumber;
}

map<string,map<string,map<string,map<string,ParaMeters::weightInfo>>>> getSynWeight(){
    ifstream file(FileWeight,ios::in);
    string srcArea, tarArea, srcPop, tarPop;
    double wAve, wSd;
    map<string,map<string,map<string,map<string,ParaMeters::weightInfo>>>> SynWeight;
    while(file>>tarArea>>tarPop>>srcArea>>srcPop>>wAve>>wSd){
        SynWeight[tarArea][tarPop][srcArea][srcPop].wAve=wAve;
        SynWeight[tarArea][tarPop][srcArea][srcPop].wSd=wSd;
    }
    return SynWeight;
}


int main(){
    double wAve, wSd;
    const string* AreaList=ParaMeters::areaNames;
    const string* PopList=ParaMeters::populationNames;
    map<string,map<string,map<string,map<string,int>>>> SynapseNumber=getSynNum();
    map<string,map<string,map<string,map<string,ParaMeters::weightInfo>>>> SynapseWeight=getSynWeight();
    for(int tarArea=0;tarArea<ParaMeters::AreaMax;tarArea++){
        for(int tarPop=0;tarPop<ParaMeters::PopulationMax;tarPop++){
            string tarName=ParaMeters::getPopName(tarArea,tarPop);
            for(int srcArea=0;srcArea<ParaMeters::AreaMax;srcArea++){
                for(int srcPop=0;srcPop<ParaMeters::PopulationMax;srcPop++){
                    string srcName=ParaMeters::getPopName(srcArea,srcPop);
                    string synName=ParaMeters::getSynName(tarArea,tarPop,srcArea,srcPop);
                    int synNum=SynapseNumber[AreaList[tarArea]][PopList[tarPop]][AreaList[srcArea]][PopList[srcPop]];
                    // if(synNum==0){
                    //     continue;
                    // }
                    cout<<"Create Synapses of "<<synName<<", synNum="<<synNum<<endl;
                    wAve=SynapseWeight[AreaList[tarArea]][PopList[tarPop]][AreaList[tarArea]][PopList[tarPop]].wAve;
                    wSd=SynapseWeight[AreaList[tarArea]][PopList[tarPop]][AreaList[tarArea]][PopList[tarPop]].wSd;
                }
            }
        }
    }
}