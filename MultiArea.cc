#include "modelSpec.h"
#include "neuronModels.h"
#include<ctime>
#include "currentSourceModels.h"
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
const char *FileDist="/home/yangjinhao/GeNN/userproject/MultiArea/AreaDist.txt";

map<string,map<string,map<string,map<string,int>>>> getSynNum(){
    ifstream file(FileSynapseNumber, ios::in);
    string srcArea, tarArea, srcPop, tarPop;
    float synNum;
    map<string,map<string,map<string,map<string,int>>>> SynNumber;
    while(file>>tarArea>>tarPop>>srcArea>>srcPop>>synNum){
        SynNumber[tarArea][tarPop][srcArea][srcPop]=static_cast<int>(synNum);
    }
    return SynNumber;
    file.close();
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
    file.close();
}

map<string, map<string, int>> getNeuNum(){
    ifstream file(FileNeuronNumber, ios::in);
    string tarArea, tarPop;
    int neuNum;
    map<string, map<string, int>> NeuNumber;
    while(file>>tarArea>>tarPop>>neuNum){
        NeuNumber[tarArea][tarPop]=neuNum;
    }
    return NeuNumber;
    file.close();
}

map<string, map<string, float>> getDist(){
    ifstream file(FileDist,ios::in);
    string tarArea, srcArea;
    float d;
    map<string, map<string, float>> AreaDist;
    while(file>>tarArea>>srcArea>>d){
        AreaDist[tarArea][srcArea]=d;
    }
    return AreaDist;
    file.close();
}

void modelDefinition(ModelSpec &model){
    model.setDT(ParaMeters::dtMs);
    model.setName("MultiArea");
    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
    model.setSeed(seed);
    model.setTiming(ParaMeters::measureTiming);
    map<string, map<string, int>> NeuronNumberMap;
    const string* AreaList=ParaMeters::areaNames;
    const string* PopList=ParaMeters::populationNames;
    NeuronNumberMap=getNeuNum();
    InitVarSnippet::Normal::ParamValues vDist(
        ParaMeters::Vmean, //mean
        ParaMeters::Vsd);  //sd
    NeuronModels::LIF::VarValues lifInit(
        initVar<InitVarSnippet::Normal>(vDist), //V
        0.0);//refractor
    for(int area=0;area<ParaMeters::AreaMax;area++){
        for(int pop=0;pop<ParaMeters::PopulationMax;pop++){
            string popName=ParaMeters::getPopName(area,pop);
            cout<<"Create Group:"<<popName<<", Neuron Number="<<NeuronNumberMap[AreaList[area]][PopList[pop]]<<endl;
            float ioffset=ParaMeters::input[ParaMeters::populationNames[pop]];
            NeuronModels::LIF::ParamValues lifParams(
                ParaMeters::lifParam.Cm/1000.0,//C
                ParaMeters::lifParam.taum,//TauM
                ParaMeters::lifParam.EL,//Vrest
                ParaMeters::lifParam.Vreset,//Vreset
                ParaMeters::lifParam.Vth,//Vthresh
                ioffset,//Ioffset
                ParaMeters::lifParam.t_ref);//refractor
            auto *Neu=model.addNeuronPopulation<NeuronModels::LIF>(popName,NeuronNumberMap[AreaList[area]][PopList[pop]],lifParams,lifInit);
            if(SpikeRecord==true){
                Neu->setSpikeRecordingEnabled(true);
            }
        }
    }
    PostsynapticModels::ExpCurr::ParamValues excitatoryExpCurrParams(
        0.5);  // 0 - TauSyn (ms)
    PostsynapticModels::ExpCurr::ParamValues inhibitoryExpCurrParams(
        0.5);  // 0 - TauSyn (ms)
    const double quantile = 0.9999;
    const double maxDelayMs[ParaMeters::typeMax] = {
        ParaMeters::meanDelay[ParaMeters::typeE] + (ParaMeters::delaySD[ParaMeters::typeE] * normalCDFInverse(quantile)),
        ParaMeters::meanDelay[ParaMeters::typeE] + (ParaMeters::delaySD[ParaMeters::typeI] * normalCDFInverse(quantile))};
    const unsigned int maxDendriticDelaySlots = (unsigned int)rint(max(maxDelayMs[ParaMeters::typeE], maxDelayMs[ParaMeters::typeI])  / ParaMeters::dtMs);
    
    double wAve, wSd;
    map<string,map<string,map<string,map<string,int>>>> SynapseNumberMap=getSynNum();
    map<string,map<string,map<string,map<string,ParaMeters::weightInfo>>>> SynapseWeightMap=getSynWeight();
    map<string,map<string,float>> AreaDistMap=getDist();
    float meanDelay, sdDelay, maxDelay;
    for(int tarArea=0;tarArea<ParaMeters::AreaMax;tarArea++){
        for(int tarPop=0;tarPop<ParaMeters::PopulationMax;tarPop++){
            string tarName=ParaMeters::getPopName(tarArea,tarPop);
            for(int srcArea=0;srcArea<ParaMeters::AreaMax;srcArea++){
                for(int srcPop=0;srcPop<ParaMeters::PopulationMax;srcPop++){
                    string srcName=ParaMeters::getPopName(srcArea,srcPop);
                    string synName=ParaMeters::getSynName(tarArea,tarPop,srcArea,srcPop);
                    int synNum=SynapseNumberMap[AreaList[tarArea]][PopList[tarPop]][AreaList[srcArea]][PopList[srcPop]];
                    if(synNum==0){
                        continue;
                    }
                    cout<<"Create Synapses of "<<synName<<", synNum="<<synNum<<endl;
                    wAve=SynapseWeightMap[AreaList[tarArea]][PopList[tarPop]][AreaList[tarArea]][PopList[tarPop]].wAve;
                    wSd=SynapseWeightMap[AreaList[tarArea]][PopList[tarPop]][AreaList[tarArea]][PopList[tarPop]].wSd;
                    InitSparseConnectivitySnippet::FixedNumberTotalWithReplacement::ParamValues synapseNum(
                        synNum);
                    if (PopList[srcPop].find("E") != string::npos){
                        InitVarSnippet::NormalClipped::ParamValues wDist(
                            wAve,
                            wSd,
                            0,
                            numeric_limits<float>::max());
                        if(srcArea==tarArea){
                            meanDelay=ParaMeters::meanDelay[0];
                            sdDelay=ParaMeters::delaySD[0];
                            maxDelay=maxDelayMs[0];
                        }else{
                            meanDelay=AreaDistMap[AreaList[tarArea]][AreaList[srcArea]]/ParaMeters::speed;
                            sdDelay=meanDelay/2;
                            maxDelay=meanDelay+sdDelay*normalCDFInverse(quantile);
                        }
                        InitVarSnippet::NormalClippedDelay::ParamValues dDist(
                            meanDelay,
                            sdDelay,
                            0.0,
                            maxDelay);
                        WeightUpdateModels::StaticPulseDendriticDelay::VarValues staticSynapsesInit(
                            initVar<InitVarSnippet::NormalClipped>(wDist),
                            initVar<InitVarSnippet::NormalClippedDelay>(dDist));
                        auto* syn=model.addSynapsePopulation<WeightUpdateModels::StaticPulseDendriticDelay,PostsynapticModels::ExpCurr>(
                            synName,SynapseMatrixType::SPARSE_INDIVIDUALG,NO_DELAY,srcName,tarName,
                            {},staticSynapsesInit,excitatoryExpCurrParams,{},
                            initConnectivity<InitSparseConnectivitySnippet::FixedNumberTotalWithReplacement>(synapseNum));
                        // syn->setMaxDendriticDelayTimesteps(maxDendriticDelaySlots);
                    }else{
                        InitVarSnippet::NormalClipped::ParamValues wDist(
                            wAve,
                            wSd,
                            0,
                            numeric_limits<float>::max());
                        if(srcArea==tarArea){
                            meanDelay=ParaMeters::meanDelay[1];
                            sdDelay=ParaMeters::delaySD[1];
                            maxDelay=maxDelayMs[1];
                        }else{
                            meanDelay=AreaDistMap[AreaList[tarArea]][AreaList[srcArea]]/ParaMeters::speed;
                            sdDelay=meanDelay/2;
                            maxDelay=meanDelay+sdDelay*normalCDFInverse(quantile);
                        }
                        InitVarSnippet::NormalClippedDelay::ParamValues dDist(
                            meanDelay,
                            sdDelay,
                            0.0,
                            maxDelay);
                        WeightUpdateModels::StaticPulseDendriticDelay::VarValues staticSynapsesInit(
                            initVar<InitVarSnippet::NormalClipped>(wDist),
                            initVar<InitVarSnippet::NormalClippedDelay>(dDist));
                        auto* syn=model.addSynapsePopulation<WeightUpdateModels::StaticPulseDendriticDelay,PostsynapticModels::ExpCurr>(
                            synName,SynapseMatrixType::SPARSE_INDIVIDUALG,NO_DELAY,srcName,tarName,
                            {},staticSynapsesInit,inhibitoryExpCurrParams,{},
                            initConnectivity<InitSparseConnectivitySnippet::FixedNumberTotalWithReplacement>(synapseNum));
                        // syn->setMaxDendriticDelayTimesteps(maxDendriticDelaySlots);
                    }
                }
            }
        }
    }
}