#ifndef binLayout_h
#define binLayout_h

#include <vector>
#include <deque>
#include <map>
#include <assert.h>
#include <time.h>
#include "MemoryMapped.h"
#include "../baseFunctions/fpBaseNode.h"
#include "../forestTypes/binnedTree/binnedBase.h"
#include "../forestTypes/binnedTree/binStruct.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include <assert.h>
#include <cstdio>
#include <string>
#include <functional>
#include <cstring>
#include <iostream>
#include <fstream>
#define NUM_FILES 2 
#define BLOCK_SIZE 128
using namespace std::placeholders;
using std::placeholders::_1;
using std::placeholders::_2;

std::map<int, int> map_subtree_to_class;
std::map<int, int> map_subtree_to_size;
int class_size_in_st[102];

namespace fp{
    template<typename T, typename Q>
        class BinLayout
        {
            binStruct<T, Q> binstr;
            std::deque<fpBaseNodeStat<T, Q>> binQ;
            std::deque<fpBaseNodeStat<T, Q>> binQTemp;
            std::deque<fpBaseNodeStat<T, Q>> binQLeft;
            std::deque<fpBaseNodeStat<T, Q>> binQRight;
            std::map<int, int> nodeNewIdx;
            std::string filename;
            std::vector<std::string> filename_vec;
            public:
            std::vector<fpBaseNodeStat<T, Q>> finalbin;
            std::vector<int> treeRootPos;
            BinLayout(binStruct<T, Q> tempbins): binstr(tempbins){

            }	

            BinLayout(binStruct<T, Q> tempbins, std::string fname): binstr(tempbins){
                filename = fname;
            }

            inline std::string returnFilename(){
                /* Return the file in which the forest was written to */
                return filename;
            }
            inline void push_insert(std::deque<fpBaseNodeStat<T, Q>> &binST, fpBaseNodeStat<T, Q> ele){
                //Find position to insert element so that it remains sorted
                int count = 0, pos=0;
                int flag = 0;
                for(auto i: binST){
                    //if ( ((float)(ele.getID())/(float)(ele.returnDepth())) < ((float)(i.getID())/(float)(i.returnDepth())))
                    if ( ((float)(ele.getID())/(float)(ele.returnDepth())) < ((float)(i.getID())/(float)(i.returnDepth())))
                        //if ( ele.getID()  <  i.getID() )
                    {
                        //flag = 1;
                        pos = count;
                        break;
                    }

                    count++;
                }
                if(pos == 0)
                    pos = count;
                binST.insert(binST.begin() + pos, ele);
            }

            inline void BINBFSLayout(int depthIntertwined){
                std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
                std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();

                int numNodesToProc = std::pow(2, depthIntertwined) -1 ; 
                auto numClasses = fpSingleton::getSingleton().returnNumClasses();

                /*if(depthIntertwined == 1){
                  for(int i=0; i<binstr.returnNumTrees(); ++i)
                  bin[i+numClasses].setSTNum(-1*depthIntertwined);
                  for(auto i: bin)
                  finalbin.push_back(i);
                  return;
                  }*/

                for(auto i = 0; i < numClasses; ++i){
                    finalbin.push_back(bin[i]);
                    nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
                }

                for(auto i = 0; i < binstr.returnNumTrees(); ++i) {
                    binQTemp.push_back(bin[i+numClasses]);
                }

                // Intertwined levels
                int currLevel = 0; 
                int posInLevel = 0;
                //if(nodeTreeMap[binQ.front().getID()] != i)
                //  continue;

                while(currLevel < numNodesToProc*binstr.returnNumTrees()) {
                    auto ele = binQTemp.front();
                    ele.printID();
                    ele.printNode();
                    binQTemp.pop_front();
                    if(ele.getID()>= numClasses) {
                        ele.setSTNum(currLevel);
                        finalbin.push_back(ele);
                        nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));
                        bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum());
                        bin[ele.returnRightNodeID()].setSTNum(ele.getSTNum());
                        binQLeft.push_back(bin[ele.returnLeftNodeID()]); 
                        binQRight.push_back(bin[ele.returnRightNodeID()]); 
                    }
                    else {
                        binQLeft.push_back(bin[ele.returnRightNodeID()]); 
                        binQRight.push_back(bin[ele.returnRightNodeID()]); 
                    }

                    if(posInLevel == binstr.returnNumTrees() - 1)
                    {
                        while(!binQLeft.empty()){
                            auto ele = binQLeft.front();
                            binQTemp.push_back(ele);
                            binQLeft.pop_front();
                        }
                        while(!binQRight.empty()){
                            auto ele = binQRight.front();
                            binQTemp.push_back(ele);
                            binQRight.pop_front();
                        }
                        posInLevel = 0;
                    }
                    else
                        posInLevel++;

                    currLevel++;
                }


                while(!binQTemp.empty()){
                    auto ele = binQTemp.front();
                    if(ele.getID() >= numClasses)
                        binQ.push_back(ele);
                    binQTemp.pop_front();

                }
                // STAT per (sub)tree layout 

                while(!binQ.empty()){
                    std::deque<fpBaseNodeStat<T, Q>> binST;
                    auto ele = binQ.front();
                    binQ.pop_front();
                    binST.push_back(ele);
                    while(!binST.empty()){
                        auto ele = binST.front();
                        binST.pop_front(); 
                        finalbin.push_back(ele);
                        nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));
                        if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
                            continue;

                        else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses())
                            binST.push_back(bin[ele.returnRightNodeID()]);

                        else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
                            binST.push_back(bin[ele.returnLeftNodeID()]); 

                        else {
                            binST.push_back(bin[ele.returnLeftNodeID()]); 
                            binST.push_back(bin[ele.returnRightNodeID()]); 
                        }
                    }
                }		
                int siz = finalbin.size();
                for (auto i=numClasses; i<siz; i++){
                    finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                    finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                }
            }

            inline void BINStatLayout(int depthIntertwined){
                std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
                std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();

                int numNodesToProc = std::pow(2, depthIntertwined) - 1; 
                auto numClasses = fpSingleton::getSingleton().returnNumClasses();


                for(auto i = 0; i < numClasses; ++i){
                    finalbin.push_back(bin[i]);
                    nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
                }

                for(auto i = 0; i < binstr.returnNumTrees(); ++i){
                    binQTemp.push_back(bin[i+numClasses]);
                }

                // Intertwined levels
                int currLevel = 0;
                int posInLevel = 0; 

                while(currLevel < numNodesToProc*binstr.returnNumTrees()) {
                    auto ele = binQTemp.front();
                    ele.printID();
                    ele.printNode();
                    binQTemp.pop_front();
                    if(ele.getID()>= numClasses) {
                        ele.setSTNum(currLevel);
                        finalbin.push_back(ele);
                        nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));
                        bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum());
                        bin[ele.returnRightNodeID()].setSTNum(ele.getSTNum());
                        binQLeft.push_back(bin[ele.returnLeftNodeID()]); 
                        binQRight.push_back(bin[ele.returnRightNodeID()]); 
                    }
                    else {
                        binQLeft.push_back(bin[ele.returnRightNodeID()]); 
                        binQRight.push_back(bin[ele.returnRightNodeID()]); 
                    }

                    if(posInLevel == binstr.returnNumTrees() - 1)
                    {
                        while(!binQLeft.empty()){
                            auto ele = binQLeft.front();
                            binQTemp.push_back(ele);
                            binQLeft.pop_front();
                        }
                        while(!binQRight.empty()){
                            auto ele = binQRight.front();
                            binQTemp.push_back(ele);
                            binQRight.pop_front();
                        }
                        posInLevel = 0;
                    }
                    else
                        posInLevel++;

                    currLevel++;
                }


                while(!binQTemp.empty()){
                    auto ele = binQTemp.front();
                    if(ele.getID() >= numClasses)
                        binQ.push_back(ele);
                    binQTemp.pop_front();
                }

                // STAT per (sub)tree layout 

                while(!binQ.empty()){
                    std::deque<fpBaseNodeStat<T, Q>> binST;
                    auto ele = binQ.front();
                    binQ.pop_front();
                    binST.push_back(ele);
                    while(!binST.empty()){
                        auto ele = binST.front();
                        binST.pop_front(); 
                        finalbin.push_back(ele);
                        nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));
                        if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
                            continue;

                        else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses())
                        binST.push_front(bin[ele.returnRightNodeID()]);

                        else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
                        binST.push_front(bin[ele.returnLeftNodeID()]); 

                        else {
                            if(bin[ele.returnLeftNodeID()].getCard() <= bin[ele.returnRightNodeID()].getCard()){
                                binST.push_front(bin[ele.returnLeftNodeID()]); 
                                binST.push_front(bin[ele.returnRightNodeID()]); 
                            }
                            else{
                                binST.push_front(bin[ele.returnRightNodeID()]); 
                                binST.push_front(bin[ele.returnLeftNodeID()]); 
                            }
                        }
                    }
                }

                int siz = finalbin.size();
                for (auto i=numClasses; i<siz; i++){
                    finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                    finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                }

            }
            inline bool qComp(fpBaseNodeStat<T, Q> &node1, fpBaseNodeStat<T, Q> &node2)
            {
                if(node1.getCard() == node2.getCard())
                    return node1.getID() < node2.getID();
                return node1.getCard() > node2.getCard();
            }


            inline void alignLayout(int depthIntertwined){
                int pos_in_block = 0;
                std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
                std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();

                int numNodesToProc = std::pow(2, depthIntertwined) - 1; 
                auto numClasses = fpSingleton::getSingleton().returnNumClasses();


                for(auto i = 0; i < numClasses; ++i){
                    finalbin.push_back(bin[i]);
                    nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
                }

                for(auto i = 0; i < binstr.returnNumTrees(); ++i){
                    binQTemp.push_back(bin[i+numClasses]);
                }

                // Intertwined levels
                int currLevel = 0;
                int posInLevel = 0; 

                while(currLevel < numNodesToProc*binstr.returnNumTrees()) {
                    auto ele = binQTemp.front();
                    ele.printID();
                    ele.printNode();
                    binQTemp.pop_front();
                    if(ele.getID()>= numClasses) {
                        ele.setSTNum(currLevel);
                        finalbin.push_back(ele);
                        nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));
                        bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum());
                        bin[ele.returnRightNodeID()].setSTNum(ele.getSTNum());
                        binQLeft.push_back(bin[ele.returnLeftNodeID()]); 
                        binQRight.push_back(bin[ele.returnRightNodeID()]); 
                    }
                    else {
                        binQLeft.push_back(bin[ele.returnRightNodeID()]); 
                        binQRight.push_back(bin[ele.returnRightNodeID()]); 
                    }

                    if(posInLevel == binstr.returnNumTrees() - 1)
                    {
                        while(!binQLeft.empty()){
                            auto ele = binQLeft.front();
                            binQTemp.push_back(ele);
                            binQLeft.pop_front();
                        }
                        while(!binQRight.empty()){
                            auto ele = binQRight.front();
                            binQTemp.push_back(ele);
                            binQRight.pop_front();
                        }
                        posInLevel = 0;
                    }
                    else
                        posInLevel++;

                    currLevel++;
                }


                  std::deque<fpBaseNodeStat<T, Q>> binST;
                while(!binQTemp.empty()){
                    auto ele = binQTemp.front();
                    if(ele.getID() >= numClasses){
                        binQ.push_back(ele);
                        binST.push_back(ele);
                    }
                    binQTemp.pop_front();
                }

                // STAT per (sub)tree layout 
                int subtree_flag = 0;
                //std::sort(binQ.begin(), binQ.end(), );
                std::cout<<"Before binQ size: "<<binQ.size()<<"\n";
                //std::sort(binQ.begin(), binQ.end(), [this](auto l, auto r){return qComp(l, r);} );
                std::cout<<"After binQ size: "<<binQ.size()<<"\n";
                //while(!binQ.empty()){
                  //  std::deque<fpBaseNodeStat<T, Q>> binST;
                    //auto ele = binQ.front();
                    //binQ.pop_front();
                    //binST.push_back(ele);
                    pos_in_block = (finalbin.size()-1) % BLOCK_SIZE;
                    subtree_flag = 0;
                    while(!binST.empty()){
                        //If we are at the beginning of a block, then we pop from the back of the queue (i.e we take a new "tree")
                        auto ele = binST.front();
                        if(pos_in_block == BLOCK_SIZE-1){
                            if(subtree_flag == 0) {
                                ele = binST.front();
                                binST.pop_front();
                                subtree_flag = 1;
                            }
                            else{
                                subtree_flag++;
                                int max = -1;
                                int positer = 0;
                                int ecount = 0;

                                for(auto ii: binST) {
                                    if((ii.getCard() > max) || (ii.getCard() == max && ii.returnDepth() < ele.returnDepth())){
                                        max = ii.getCard();
                                        positer = ecount;
                                        ele = ii;
                                    }
                                    ecount++;
                                }
                                binST.erase(binST.begin() + positer, binST.begin() + positer +1);

                            }
                        }
                        else{
                            ele = binST.front();
                            binST.pop_front();
                        }
                        finalbin.push_back(ele);
                        pos_in_block = (pos_in_block + 1)%BLOCK_SIZE;
                        nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));
                        if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
                            continue;

                        else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses())
                            binST.push_front(bin[ele.returnRightNodeID()]);

                        else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
                            binST.push_front(bin[ele.returnLeftNodeID()]); 

                        else {
                            if(bin[ele.returnLeftNodeID()].getCard() <= bin[ele.returnRightNodeID()].getCard()){
                                binST.push_front(bin[ele.returnLeftNodeID()]); 
                                binST.push_front(bin[ele.returnRightNodeID()]); 
                            }
                            else{
                                binST.push_front(bin[ele.returnRightNodeID()]); 
                                binST.push_front(bin[ele.returnLeftNodeID()]); 
                            }
                        }
                    }
                //}

                int siz = finalbin.size();
                for (auto i=numClasses; i<siz; i++){
                    finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                    finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                }

            }

            inline void BINDFSLayout(int depthIntertwined){
                int subtree_size = 0;
                std::vector<int> subtree_size_vec;
                std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
                std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();

                int numNodesToProc = std::pow(2, depthIntertwined) - 1; 
                auto numClasses = fpSingleton::getSingleton().returnNumClasses();


                for(auto i = 0; i < numClasses; ++i){
                    finalbin.push_back(bin[i]);
                    nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
                }

                for(auto i = 0; i < binstr.returnNumTrees(); ++i){
                    binQTemp.push_back(bin[i+numClasses]);
                }

                // Intertwined levels
                int currLevel = 0;
                int posInLevel = 0; 
                //if(nodeTreeMap[binQ.front().getID()] != i)
                //  continue

                while(currLevel < numNodesToProc*binstr.returnNumTrees()) {
                    auto ele = binQTemp.front();
                    ele.printID();
                    ele.printNode();
                    binQTemp.pop_front();
                    if(ele.getID()>= numClasses) {
                        ele.setSTNum(currLevel);
                        finalbin.push_back(ele);
                        nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));
                        bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum());
                        bin[ele.returnRightNodeID()].setSTNum(ele.getSTNum());
                        binQLeft.push_back(bin[ele.returnLeftNodeID()]); 
                        binQRight.push_back(bin[ele.returnRightNodeID()]); 
                    }
                    else {
                        binQLeft.push_back(bin[ele.returnRightNodeID()]); 
                        binQRight.push_back(bin[ele.returnRightNodeID()]); 
                    }

                    if(posInLevel == binstr.returnNumTrees() - 1)
                    {
                        while(!binQLeft.empty()){
                            auto ele = binQLeft.front();
                            binQTemp.push_back(ele);
                            binQLeft.pop_front();
                        }
                        while(!binQRight.empty()){
                            auto ele = binQRight.front();
                            binQTemp.push_back(ele);
                            binQRight.pop_front();
                        }
                        posInLevel = 0;
                    }
                    else
                        posInLevel++;

                    currLevel++;
                }


                while(!binQTemp.empty()){
                    auto ele = binQTemp.front();
                    if(ele.getID() >= numClasses)
                        binQ.push_back(ele);
                    binQTemp.pop_front();
                }

                // STAT per (sub)tree layout 

                while(!binQ.empty()){
                    std::deque<fpBaseNodeStat<T, Q>> binST;
                    auto ele = binQ.front();
                    binQ.pop_front();
                    binST.push_back(ele);
                    while(!binST.empty()){
                        auto ele = binST.front();
                        binST.pop_front(); 
                        subtree_size++;
                        finalbin.push_back(ele);
                        nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));
                        if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
                            continue;

                        else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses())
                            binST.push_front(bin[ele.returnRightNodeID()]);

                        else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
                            binST.push_front(bin[ele.returnLeftNodeID()]); 

                        else {
                            binST.push_front(bin[ele.returnLeftNodeID()]); 
                            binST.push_front(bin[ele.returnRightNodeID()]); 
                        }
                    }
                    subtree_size_vec.push_back(subtree_size);
                    subtree_size = 0;
                }
                int siz = finalbin.size();
                for (auto i=numClasses; i<siz; i++){
                    finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                    finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                }
                std::fstream fsiz;
                fsiz.open("residual_sizes.csv", std::ios::out);
                for(auto size_item: subtree_size_vec)
                    fsiz<<size_item<<",";
                fsiz.close();

            }

            inline bool myCompFunction(fpBaseNodeStat<T, Q> &node1, fpBaseNodeStat<T, Q> &node2)
            {

                //If nodes belong to the same subtree sort by new node index
                if(node1.getSTNum() == node2.getSTNum())
                    return nodeNewIdx[node1.getID()] < nodeNewIdx[node2.getID()];


                //sort by class if nodes belong to subtrees of different majority class
                if(map_subtree_to_class[node1.getSTNum()] != map_subtree_to_class[node2.getSTNum()])
                    return class_size_in_st[map_subtree_to_class[node1.getSTNum()]] < class_size_in_st[map_subtree_to_class[node2.getSTNum()]];

                //if classes are the same sort by size of subtree

                //f(map_subtree_to_size[node1.getSTNum()] == map_subtree_to_size[node2.getSTNum()])
                //	return nodeNewIdx[node1.getID()] < nodeNewIdx[node2.getID()];

                //return map_subtree_to_size[node1.getSTNum()] < map_subtree_to_size[node2.getSTNum()];
                return nodeNewIdx[node1.getID()] < nodeNewIdx[node2.getID()];

            }

            inline void BINStatClassLayout(int depthIntertwined){
                int total_tree_card = 0;
                int num_classes_in_subtree = 0;
                double eps = 0;
                int card[102] = {0};
                int max = -1;
                int subtree_class = -1;
                std::memset(class_size_in_st, 0, 102*sizeof(class_size_in_st[0])); 

                //std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
                std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
                //number of nodes of a binary tree as a function of height
                int numNodesToProc = std::pow(2, depthIntertwined) - 1; 
                auto numClasses = fpSingleton::getSingleton().returnNumClasses();
                //binstr.setNumTrees(128);
                //fpSingleton::getSingleton().setNumClasses(10);
                //auto numClasses = 10;

                for(auto i = 0; i<numClasses; ++i){
                    finalbin.push_back(bin[i]);
                    nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));

                }
                //roots of trees
                for(auto i = 0; i < binstr.returnNumTrees(); ++i){
                    //bin[i+numClasses].setSTNum(stno++);
                    binQTemp.push_back(bin[i+numClasses]);
                }
                // Intertwined levels
                int currLevel = 0; 

                int posInLevel = 0;
                //process intertwined(BIN) levels	
                while(currLevel < numNodesToProc*binstr.returnNumTrees()) {
                    auto ele = binQTemp.front();
                    ele.printID();
                    ele.printNode();
                    binQTemp.pop_front();
                    if(ele.getID()>= numClasses) {
                        ele.setSTNum(currLevel);
                        finalbin.push_back(ele);
                        bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum());
                        bin[ele.returnRightNodeID()].setSTNum(ele.getSTNum());
                        binQLeft.push_back(bin[ele.returnLeftNodeID()]); 
                        binQRight.push_back(bin[ele.returnRightNodeID()]); 
                    }
                    else {
                        binQLeft.push_back(bin[ele.returnRightNodeID()]); 
                        binQRight.push_back(bin[ele.returnRightNodeID()]); 
                    }

                    if(posInLevel == binstr.returnNumTrees() - 1)
                    {
                        while(!binQLeft.empty()){
                            auto ele = binQLeft.front();
                            binQTemp.push_back(ele);
                            binQLeft.pop_front();
                        }
                        while(!binQRight.empty()){
                            auto ele = binQRight.front();
                            binQTemp.push_back(ele);
                            binQRight.pop_front();
                        }
                        posInLevel = 0;
                    }
                    else
                        posInLevel++;

                    currLevel++;
                }


                while(!binQTemp.empty()){
                    auto ele = binQTemp.front();
                    if(ele.getID() >= numClasses)
                        binQ.push_back(ele);
                    binQTemp.pop_front();
                }


                //initial subtree size map for each subtree num
                for(int i=0; i<=currLevel*100; ++i)
                    map_subtree_to_size[i] = 0;

                std::vector<fpBaseNodeStat<T, Q>> newfinalbin;
                std::vector<fpBaseNodeStat<T, Q>> newfinalbin2;
                for(auto i: finalbin)
                    newfinalbin2.push_back(i);	

                int inter_siz = newfinalbin2.size();

                // STAT per (sub)tree layout 
                //		int stno = -1; 

                int new_st = currLevel;
                //int numNodesInST = 0;
                int curr_subtree = binQ.front().getSTNum();
                //int old_subtree=-1;
                int leaf_present = 0;
                while(!binQ.empty()){
                    leaf_present = 0;
                    std::deque<fpBaseNodeStat<T, Q>> binST;
                    auto ele = binQ.front();
                    binQ.pop_front();
                    ele.setSTNum(new_st);
                    curr_subtree = ele.getSTNum();
                    binST.push_back(ele);
                    //if(curr_subtree != old_subtree){
                    std::memset(card, 0, 102*sizeof(card[0])); 
                    //	if(curr_subtree != old_subtree){
                    total_tree_card = 0;
                    num_classes_in_subtree=0;
                    //stno++;
                    //	numNodesInST = 0;

                    //	}
                    while(!binST.empty()){
                        auto ele = binST.front();
                        //		numNodesInST++;
                        ele.setSTNum(new_st);
                        //if ele is a leaf node, then check the class and cardinality
                        //if(!ele.isInternalNode() && ele.returnClass()<numClasses){

                        if( ele.returnLeftNodeID()<numClasses){
                            if(card[ele.returnLeftNodeID()] == 0)
                                num_classes_in_subtree++;
                            card[ele.returnLeftNodeID()] += ele.getLeftLeafCard();
                            map_subtree_to_size[ele.getSTNum()]+=ele.getLeftLeafCard();
                            total_tree_card += ele.getLeftLeafCard();
                            leaf_present = 1;
                        }
                        if( ele.returnRightNodeID()<numClasses){
                            if(card[ele.returnRightNodeID()] == 0)
                                num_classes_in_subtree++;
                            card[ele.returnRightNodeID()] += ele.getRightLeafCard();
                            total_tree_card += ele.getRightLeafCard();
                            map_subtree_to_size[ele.getSTNum()]+=ele.getRightLeafCard();
                            leaf_present = 1;
                        }

                        binST.pop_front(); 
                        finalbin.push_back(ele);
                        if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
                        {
                            continue;
                        }

                        else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) {
                            bin[ele.returnRightNodeID()].setSTNum(ele.getSTNum());
                            binST.push_front(bin[ele.returnRightNodeID()]);
                        }

                        else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()){
                            bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum());
                            binST.push_front(bin[ele.returnLeftNodeID()]); 
                        }
                        else {
                            if(bin[ele.returnLeftNodeID()].getCard() <=  bin[ele.returnRightNodeID()].getCard()){
                                bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum());
                                binST.push_front(bin[ele.returnLeftNodeID()]); 
                                bin[ele.returnRightNodeID()].setSTNum(ele.getSTNum());
                                binST.push_front(bin[ele.returnRightNodeID()]); 
                            }
                            else{
                                bin[ele.returnRightNodeID()].setSTNum(ele.getSTNum());
                                binST.push_front(bin[ele.returnRightNodeID()]); 
                                bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum());
                                binST.push_front(bin[ele.returnLeftNodeID()]); 
                            }
                        }
                    }
                    new_st++;
                    //compute max class
                    //	if(curr_subtree != old_subtree){
                    max = -1;
                    subtree_class = 103;
                    //	}
                    //if(num_classes_in_subtree > 0)
                    //	eps = 1 / (double)num_classes_in_subtree;
                    if(total_tree_card > 0){
                        for(int i=0; i<numClasses; ++i){
                            //			std::cout<<"class num: "<<i<<"card[i]: "<<card[i]<<" / "<<total_tree_card<<"\n";
                            if(card[i] > max && ((double)(card[i]) / (double)(total_tree_card) > eps)){
                                subtree_class = i;
                                max = card[i];
                            }
                        }
                    }
                    else
                    {
                        std::cout<<"total_tree_card != 0!!!!\n";
                        std::cout<<"leaf_present: "<<leaf_present<<"\n";
                    }
                    class_size_in_st[subtree_class]++;
                    map_subtree_to_class[curr_subtree] = subtree_class;
                    }
                    int siz = finalbin.size();

                    newfinalbin.clear();
                    for(int i = inter_siz; i<siz; ++i)
                        newfinalbin.push_back(finalbin[i]);

                    nodeNewIdx.clear();
                    for(auto i=0; i < siz; ++i){
                        nodeNewIdx.insert(std::pair<int, int>(finalbin[i].getID(), i));
                    }


                    std::sort(newfinalbin.begin(), newfinalbin.end(), [this](auto l, auto r){return myCompFunction(l, r);} );
                    finalbin.clear();
                    for(auto i:newfinalbin2)
                        finalbin.push_back(i);
                    for(auto i:newfinalbin)
                        finalbin.push_back(i);
                    nodeNewIdx.clear();
                    for(auto i=0; i < siz; ++i){
                        nodeNewIdx.insert(std::pair<int, int>(finalbin[i].getID(), i));
                    }
                    siz = finalbin.size();
                    for (auto i=numClasses; i<siz; i++){
                        finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                        finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                    }
                    nodeNewIdx.clear();

                }



                inline void BFSLayout(){
                    std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
                    int numClasses = fpSingleton::getSingleton().returnNumClasses();
                    std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
                    std::deque<fpBaseNodeStat<T, Q>> binST;
                    finalbin.clear();
                    nodeNewIdx.clear();

                    for(int i = 0; i < numClasses; ++i){
                        finalbin.push_back(bin[i]);
                        nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
                    }

                    int firstNodeInTree = 1;
                    for(int i = 0; i < binstr.returnNumTrees(); ++i){
                        binST.push_back(bin[i+numClasses]);
                        firstNodeInTree = 1;
                        while(!binST.empty()){
                            auto ele = binST.front();
                            binST.pop_front();
                            if (firstNodeInTree == 1){
                                treeRootPos.push_back(finalbin.size()); 
                                firstNodeInTree = 0;
                            }
                            finalbin.push_back(ele);
                            nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));

                            if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
                                continue;

                            else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses())
                                binST.push_back(bin[ele.returnRightNodeID()]);

                            else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
                                binST.push_back(bin[ele.returnLeftNodeID()]); 

                            else{
                                binST.push_back(bin[ele.returnLeftNodeID()]); 
                                binST.push_back(bin[ele.returnRightNodeID()]); 
                            }
                        }
                    }

                    int siz = finalbin.size();
                    for (auto i=numClasses; i<siz; i++){
                        finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                        finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                        finalbin[i].setDepth(bin[nodeNewIdx[i]].returnDepth());
                    }
                }

                inline std::vector<fpBaseNodeStat<T, Q>> getFinalBin(){
                    return finalbin;
                }

                inline void statLayout(){
                    std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
                    int numClasses = fpSingleton::getSingleton().returnNumClasses();
                    std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
                    std::deque<fpBaseNodeStat<T, Q>> binST;
                    finalbin.clear();
                    nodeNewIdx.clear();
                    for(int i = 0; i < numClasses; ++i){
                        finalbin.push_back(bin[i]);
                        nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
                    }

                    int pos_in_block=0;
                    int firstNodeInTree = 1;
                    int subtree_flag = 0;
                    int leaf_node = 0;
                    for(int i = 0; i < binstr.returnNumTrees(); ++i){
                        binST.push_back(bin[i+numClasses]);
                        firstNodeInTree = 1;
                        pos_in_block = (finalbin.size()-1) % BLOCK_SIZE;
                        subtree_flag = 0;
                        while(!binST.empty()){
                            auto ele = binST.front();
                            binST.pop_front();

                            if (firstNodeInTree == 1){
                                treeRootPos.push_back(finalbin.size()); 
                                firstNodeInTree = 0;
                            }

                            finalbin.push_back(ele);
                            nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));

                            if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
                                continue;

                            else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses())
                                binST.push_front(bin[ele.returnRightNodeID()]);

                            else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
                                binST.push_front(bin[ele.returnLeftNodeID()]); 

                            else{
                                if(bin[ele.returnLeftNodeID()].getCard() <= bin[ele.returnRightNodeID()].getCard()){
                                    binST.push_front(bin[ele.returnLeftNodeID()]); 
                                    binST.push_front(bin[ele.returnRightNodeID()]); 
                                }
                                else{
                                    binST.push_front(bin[ele.returnRightNodeID()]); 
                                    binST.push_front(bin[ele.returnLeftNodeID()]); 
                                }
                            }
                        }
                    }

                    int siz = finalbin.size();
                    for (auto i=numClasses; i<siz; i++){
                        finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                        finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                        //		finalbin[i].setDepth(bin[nodeNewIdx[i]].returnDepth());
                    }
                }

                inline fpBaseNodeStat<T, Q> genBlankNode(){
                    fpBaseNodeStat<T, Q> tempNode; 
                    tempNode.setLeftValue(-1);
                    tempNode.setRightValue(-1);
                    tempNode.setDepth(1000);
                    return tempNode;
                }

                inline void newStatLayout(){
                    int front_subtree_id = 0;
                    int count_small_st = 0;
                    std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
                    int numClasses = fpSingleton::getSingleton().returnNumClasses();
                    std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
                    std::deque<fpBaseNodeStat<T, Q>> binST;
                    finalbin.clear();
                    nodeNewIdx.clear();
                    for(int i = 0; i < numClasses; ++i){
                        finalbin.push_back(bin[i]);
                        nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
                    }

                    int pos_in_block=0;
                    int firstNodeInTree = 1;
                    int subtree_flag = 0;
                    for(int i = 0; i < binstr.returnNumTrees(); ++i){
                        binST.push_back(bin[i+numClasses]);
                    }

                    pos_in_block = (finalbin.size()-1) % BLOCK_SIZE;
                    subtree_flag = 0;
                    front_subtree_id = binST.front().getID();
                    while(!binST.empty()){
                        auto ele = binST.front();
                        if(pos_in_block == BLOCK_SIZE-1 ){
                            if(subtree_flag == 0) {
                                ele = binST.front();
                                binST.pop_front();
                                subtree_flag++;
                            }
                            else{
                                subtree_flag++;
                                int max = -1;
                                int positer = 0;
                                int ecount = 0;

                                for(auto ii: binST) {
                                    if((ii.getCard() > max) || (ii.getCard() == max && ii.returnDepth() < ele.returnDepth())){
                                        max = ii.getCard();
                                        positer = ecount;
                                        ele = ii;
                                    }
                                    ecount++;
                                }
                                binST.erase(binST.begin() + positer, binST.begin() + positer +1);

                            }
                            if(binST.size() > 0)
                                front_subtree_id = binST.front().getID();
                        }
                        else{

                            ele = binST.front();
                            if(ele.getID() == front_subtree_id && pos_in_block > 0 && subtree_flag > 0){
                                //std::cout<<"here: "<<count_small_st<<"\n";
                                //fflush(stdout);
                                count_small_st++;
                                fpBaseNodeStat<T, Q> blank_node = genBlankNode();
                                for(int k=pos_in_block+1; k<= BLOCK_SIZE-1; k++){
                                    finalbin.push_back(blank_node);
                                }
                                pos_in_block = BLOCK_SIZE -1;
                                continue;
                            }
                            else
                                binST.pop_front();
                        }

                        finalbin.push_back(ele);
                        auto siz1 = finalbin.size() - 1;
                        if (finalbin[siz1].returnDepth() == 0){
                            treeRootPos.push_back(siz1); 
                        }

                        pos_in_block = (pos_in_block + 1)%BLOCK_SIZE;
                        nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));

                        if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
                            continue;

                        else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses())
                            binST.push_front(bin[ele.returnRightNodeID()]);

                        else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
                            binST.push_front(bin[ele.returnLeftNodeID()]); 

                        else{
                            if(bin[ele.returnLeftNodeID()].getCard() <= bin[ele.returnRightNodeID()].getCard()){
                                binST.push_front(bin[ele.returnLeftNodeID()]); 
                                binST.push_front(bin[ele.returnRightNodeID()]); 
                            }
                            else{
                                binST.push_front(bin[ele.returnRightNodeID()]); 
                                binST.push_front(bin[ele.returnLeftNodeID()]); 
                            }
                        }
                    }

                    int siz = finalbin.size();
                    for (auto i=numClasses; i<siz; i++){
                        if(!(finalbin[i].returnLeftNodeID() == -1 && finalbin[i].returnRightNodeID() == -1) ){
                        finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                        finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                        }
                        //		finalbin[i].setDepth(bin[nodeNewIdx[i]].returnDepth());
                    }
                }


                inline void DFSLayout(){
                    std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
                    int numClasses = fpSingleton::getSingleton().returnNumClasses();
                    std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
                    std::deque<fpBaseNodeStat<T, Q>> binST;
                    int subtree_size = 0;
                    std::vector<int> subtree_size_vec;
                    finalbin.clear();
                    nodeNewIdx.clear();
                    for(int i = 0; i < numClasses; ++i){
                        finalbin.push_back(bin[i]);
                        nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
                    }

                    int firstNodeInTree = 1;
                    for(int i = 0; i < binstr.returnNumTrees(); ++i){
                        binST.push_back(bin[i+numClasses]);
                        firstNodeInTree = 1;
                        while(!binST.empty()){
                            auto ele = binST.front();
                            binST.pop_front();
                            if (firstNodeInTree == 1){
                                treeRootPos.push_back(finalbin.size()); 
                                firstNodeInTree = 0;
                            }
                            finalbin.push_back(ele);
                            subtree_size++;
                            nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));

                            if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
                                continue;

                            else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses())
                                binST.push_front(bin[ele.returnRightNodeID()]);

                            else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
                                binST.push_front(bin[ele.returnLeftNodeID()]); 

                            else{
                                binST.push_front(bin[ele.returnLeftNodeID()]); 
                                binST.push_front(bin[ele.returnRightNodeID()]); 
                            }
                        }
                        subtree_size_vec.push_back(subtree_size);
                        subtree_size = 0;
                    }

                    int siz = finalbin.size();
                    for (auto i=numClasses; i<siz; i++){
                        if(!(finalbin[i].returnLeftNodeID() == -1 && finalbin[i].returnRightNodeID() == -1) ){
                            finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                            finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                        }
                        //finalbin[i].setDepth(bin[nodeNewIdx[i]].returnDepth());
                    }
                }

                inline void writeToFileStat(){
                    std::ofstream f;
                    fpBaseNodeStat<T,Q> nodeToWrite;
                    for(int j = 0; j < 1; j++){
                        f.open(("cifar_trained_stat" + std::to_string(j) + ".bin").c_str(), std::ios::out|std::ios::binary);
                        for(auto i: finalbin){
                            nodeToWrite = i;
                            f.write((char*)&nodeToWrite, sizeof(nodeToWrite));
                        }
                        f.close();
                    }
                }

                inline void writeToFile(std::vector<int> roots, int numBins){
#pragma omp critical
                    {
                        std::ofstream f;
                        fpBaseNode<T,Q> nodeToWrite;
                        for(int j = 0; j < NUM_FILES; j++){
                            f.open((filename + std::to_string(j) + ".bin").c_str(), std::ios::out|std::ios::app|std::ios::binary);
                            for(auto i: finalbin){
                                nodeToWrite = i;
                                f.write((char*)&nodeToWrite, sizeof(nodeToWrite));
                            }
                            f.close();

                        }
                        std::string treeroot_fname = "treeroots.csv";
                        f.open(treeroot_fname.c_str(), std::ios::out|std::ios::app);
                        for (auto root: roots)
                            f<<root<<"\n";
                        f.close();

                        f.open("/data4/binstart.txt", std::ios::out | std::ios::app);
                        if(numBins > 1){
                            f<<finalbin.size()<<"\n";
                        }
                        else if(numBins == 1){
                            f<<0<<"\n";
                        }
                        f.close();
                    }

                }


                std::size_t getFilesize(const char* filename) {
                    struct stat st;
                    stat(filename, &st);
                    return st.st_size;
                }

                inline fpBaseNode<T, Q> *serializeMmap(size_t &numNodes){
                    MemoryMapped mmappedObj(filename.c_str(), 0);
                    fpBaseNode<T, Q> *data = (fpBaseNode<T, Q>*)mmappedObj.getData();
                    numNodes = mmappedObj.mappedSize() / sizeof(finalbin[0]);
                    return data;
                }

                inline void serializeUnmap(){
                    //TODO : fill this 
                }

                inline void printFinalBin(){
                    int count = 0;
                    std::cout<<"final bin: \n";
                    std::cout<<"******************************************************************************\n";
                    for (auto i: finalbin){
                        std::cout<<count++<<": ";
                        i.printNode();
                    }
                }


                };
            }
#endif //binLayout_h
