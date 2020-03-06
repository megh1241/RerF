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
            inline void alignLayout(int depthIntertwined){
				std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
				std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
				std::vector<fpBaseNodeStat<T, Q>> subtreebin;

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

				int path_size=0;
				std::vector<int> path_vec;
				while(!binQ.empty()){
					std::deque<fpBaseNodeStat<T, Q>> binST;
					auto ele = binQ.front();
					binQ.pop_front();
					binST.push_back(ele);
					while(!binST.empty()){
						path_size++;
						auto ele = binST.front();
						binST.pop_front(); 
						finalbin.push_back(ele);
						subtreebin.push_back(ele);
						nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));
						if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) || (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
						{
							path_vec.push_back(path_size);
							path_size = 0;
						}
						if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
							continue;

						else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses())
							push_insert(binST, bin[ele.returnRightNodeID()]);
							//binST.push_front(bin[ele.returnRightNodeID()]);
						else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
							push_insert(binST, bin[ele.returnLeftNodeID()]);
							//binST.push_front(bin[ele.returnLeftNodeID()]); 

						else {
							if(bin[ele.returnLeftNodeID()].getCard() <= bin[ele.returnRightNodeID()].getCard()){
								push_insert(binST, bin[ele.returnLeftNodeID()]);
								push_insert(binST, bin[ele.returnRightNodeID()]);
								//binST.push_front(bin[ele.returnLeftNodeID()]); 
								//binST.push_front(bin[ele.returnRightNodeID()]); 
							}
							else{
								push_insert(binST, bin[ele.returnRightNodeID()]);
								push_insert(binST, bin[ele.returnLeftNodeID()]);
								//binST.push_front(bin[ele.returnRightNodeID()]); 
								//binST.push_front(bin[ele.returnLeftNodeID()]); 
							}
						}
					}
					path_vec.push_back(path_size);
					path_size = 0;
				}

				
				std::fstream fsiz;
				fsiz.open("path_sizes22.csv", std::ios::out);
				for(auto size_item: path_vec)
					fsiz<<size_item<<",";
				fsiz.close();

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
                            push_insert(binST, bin[ele.returnRightNodeID()]);
                            //binST.push_front(bin[ele.returnRightNodeID()]);

                        else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
                            push_insert(binST, bin[ele.returnLeftNodeID()]);
                            //binST.push_front(bin[ele.returnLeftNodeID()]); 

                        else {
                            if(bin[ele.returnLeftNodeID()].getCard() <= bin[ele.returnRightNodeID()].getCard()){
                                push_insert(binST, bin[ele.returnLeftNodeID()]);
                                push_insert(binST, bin[ele.returnRightNodeID()]);
                                //binST.push_front(bin[ele.returnLeftNodeID()]); 
                                //binST.push_front(bin[ele.returnRightNodeID()]); 
                            }
                            else{
                                push_insert(binST, bin[ele.returnRightNodeID()]);
                                push_insert(binST, bin[ele.returnLeftNodeID()]);
                                //binST.push_front(bin[ele.returnRightNodeID()]); 
                                //binST.push_front(bin[ele.returnLeftNodeID()]); 
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

            inline void BINSBFSDFSLayout(int depthIntertwined){
                std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
                std::vector< fpBaseNodeStat<T,Q> > newfinalbin;
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

                int st_start_indx = finalbin.size();
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

                nodeNewIdx.clear();
                for(int i=0; i<st_start_indx; ++i)
                {
                    newfinalbin.push_back(finalbin[i]);
                    nodeNewIdx.insert( std::pair<int, int>(finalbin[i].getID(), newfinalbin.size()-1));
                }

                int start_residual=0, end_residual, dist, new_idx;
                int new_end_residual;
                while(1){
                    if(start_residual >= finalbin.size());
                    break;
                    end_residual = findClosestLeaf(start_residual, finalbin);
                    if( start_residual/BLOCK_SIZE  == end_residual/BLOCK_SIZE ){
                        for(int i=start_residual; i<=end_residual; i++)
                        {
                            if(finalbin[i].getBlank != -1){
                                newfinalbin.push_back(finalbin[i]);
                                nodeNewIdx.insert( std::pair<int, int>(finalbin[i].getID(), newfinalbin.size()-1));
                                finalbin[i].setBlank(-1);
                            }
                        }
                        start_residual = end_residual + 1;
                    }
                    else{

                        if(finalbin[start_residual].returnLeftNodeID() == start_residual+1)
                        {    
                            new_idx = finalbin[start_residual].returnRightNodeID();
                        }
                        else{
                            new_idx = finalbin[start_residual].returnLeftNodeID();
                        }
                        new_end_residual = findClosestLeaf(new_idx, finalbin);
                        if((new_end_residual - new_idx + 1) <= (128-start_residual%128) )
                        {
                            for(int i=new_idx; i<=new_end_residual; ++i){
                                if(finalbin[i].getBlank != -1){
                                    newfinalbin.push_back(finalbin[i]);
                                    nodeNewIdx.insert( std::pair<int, int>(finalbin[start_residual].getID(), newfinalbin.size()-1));
                                    finalbin[i].setBlank(-1);
                                }
                            }
                        }
                        else {
                            if(finalbin[start_residual].getBlank != -1){
                                newfinalbin.push_back(finalbin[start_residual]);
                                nodeNewIdx.insert( std::pair<int, int>(finalbin[start_residual].getID(), newfinalbin.size()-1));
                                finalbin[start_residual].setBlank(-1);
                            }
                            start_residual++;
                        }
                    }

                }

            }


            inline void efficientLeafLayout(int depthIntertwined){
                std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
                std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
                int numNodesToProc = std::pow(2, depthIntertwined) - 1; 
                auto numClasses = fpSingleton::getSingleton().returnNumClasses();
                //set parent node
                //set path leaf counts
                std::vector<int> leafIndices;
                for(int i=0; i<numClasses + binstr.returnNumTrees(); ++i) {
                    bin[i].setParent(i);
                    bin[i].setPathLeafCount(0);
                }

                for(int i=numClasses; i<bin.size(); ++i){
                    //set parent node
                    if(bin[i].returnLeftNodeID() >= numClasses){
                        bin[bin[i].returnLeftNodeID()].setParent(i);
                    }
                    if(bin[i].returnRightNodeID() >= numClasses){
                        bin[bin[i].returnRightNodeID()].setParent(i);
                    }

                    //set path leaf counts
                    bin[i].setPathLeafCount(bin[bin[i].getParent()].getPathLeafCount());
                    if(bin[i].returnLeftNodeID() < numClasses)
                        bin[i].setPathLeafCount( bin[i].getPathLeafCount() + bin[i].getLeftLeafCard() );
                    if(bin[i].returnRightNodeID() < numClasses)
                        bin[i].setPathLeafCount( bin[i].getPathLeafCount() + bin[i].getRightLeafCard() );

                    if(bin[i].returnLeftNodeID() < numClasses && bin[i].returnRightNodeID() < numClasses){
                        leafIndices.push_back(i);

                    }
                }


                int idnum = 1;
                for(auto leaf_index: leafIndices){
                    int idx = leaf_index;
                    int idx_parent = 0;
                    idnum++;
                    while(1)
                    {
                        idx_parent = bin[idx].getParent();
                        if(idx_parent < numClasses +  binstr.returnNumTrees())
                            break;
                        if(bin[idx_parent].getPathLeafCount() >= bin[idx].getPathLeafCount())
                            break;
                        bin[idx_parent].setPathLeafCount(bin[idx].getPathLeafCount());
                        idx = idx_parent;
                    }
                }
                std::cout<<"past stage 2\n";
                fflush(stdout);


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
                        //push_insert(binST, bin[ele.returnRightNodeID()]);
                        else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
                            binST.push_front(bin[ele.returnLeftNodeID()]);
                        //push_insert(binST,bin[ele.returnLeftNodeID()]); 

                        else {
                            if(bin[ele.returnLeftNodeID()].getPathLeafCount() <= bin[ele.returnRightNodeID()].getPathLeafCount()){
                                binST.push_front(bin[ele.returnLeftNodeID()]);
                                binST.push_front(bin[ele.returnRightNodeID()]);
                                // push_insert(binST, bin[ele.returnLeftNodeID()]); 
                                //push_insert(binST, bin[ele.returnRightNodeID()]); 
                            }
                            else{
                                binST.push_front(bin[ele.returnRightNodeID()]);
                                binST.push_front(bin[ele.returnLeftNodeID()]);
                                //push_insert(binST, bin[ele.returnRightNodeID()]); 
                                //push_insert(binST, bin[ele.returnLeftNodeID()]); 
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


            inline int searchInFinalbin(fpBaseNodeStat<T, Q> ele, std::vector<fpBaseNodeStat<T, Q>> bin){
                for(auto i: bin)
                    if(i.getID() == ele.getID())
                        return 1;
                return 0;
            }

            static inline bool myfunction(fpBaseNodeStat<T, Q> a, fpBaseNodeStat<T, Q>b)
            {
                return a.getID() < b.getID();
            }

            inline void LeafStatLayout(int depthIntertwined){
                std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
                std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();

                int numNodesToProc = std::pow(2, depthIntertwined) - 1; 
                auto numClasses = fpSingleton::getSingleton().returnNumClasses();
                int size = bin.size();
                int j=0, count=0;
                for(int i=0; i<size; ++i){
                    j=i;
                    count = 0;
                    while(1){
                        if(j >= bin.size())
                            break;
                        if(bin[j].returnLeftNodeID() < numClasses || bin[j].returnRightNodeID() < numClasses)
                        {
                            count++;
                            break;
                        }

                        j++;
                        count++;
                    }
                    bin[i].setPathLen(count);
                }

                for(int i=numClasses; i<size; ++i){
                    if(bin[i].returnLeftNodeID() >= numClasses)
                        bin[bin[i].returnLeftNodeID()].setParent(i);
                    if(bin[i].returnRightNodeID() >= numClasses)
                        bin[bin[i].returnRightNodeID()].setParent(i);
                }

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
                int flag = 0;
                while(!binQ.empty()){
                    std::deque<fpBaseNodeStat<T, Q>> binST;
                    auto ele = binQ.front();
                    binQ.pop_front();
                    binST.push_back(ele);
                    flag = 0;
                    while(!binST.empty()){
                        auto ele = binST.front();
                        binST.pop_front(); 
                            finalbin.push_back(ele);
                            nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));

                        /*if(!std::binary_search(finalbin.begin(), finalbin.end(), ele, myfunction));
                        {
                            finalbin.push_back(ele);
                            nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));
                        }*/
                        if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
                            continue;

                        else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses())
                            binST.push_front(bin[ele.returnRightNodeID()]);

                        else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
                            binST.push_front(bin[ele.returnLeftNodeID()]); 

                        else {
                            //Other child is your parent's other child
                            /*
                            if(ele.getID() == bin[ele.getParent()].returnLeftNodeID())
                                other_child = bin[ele.getParent()].returnRightNodeID();
                            else
                                other_child = bin[ele.getParent()].returnLeftNodeID();

                            */
                            auto ele_other = binST.back();
                            int other_child = ele_other.getID();
                            if(bin[ele.returnLeftNodeID()].getCard() <= bin[ele.returnRightNodeID()].getCard()){
                                std::cout<<ele.getPathLen()<<"\n";
                                if(bin[ele.returnRightNodeID()].getPathLen() + finalbin.size()%BLOCK_SIZE <= BLOCK_SIZE || flag == 1){
                                    binST.push_front(bin[ele.returnLeftNodeID()]); 
                                    binST.push_front(bin[ele.returnRightNodeID()]); 
                                }
                                else {
                                    //check if your parent's other child (sibling) is a good candidate
                                    if((other_child) >= numClasses && (bin[other_child].getPathLen() + finalbin.size()%BLOCK_SIZE <= BLOCK_SIZE)){
                                        std::cout<<"case: \n";
                                        /*
                                        binST.push_front(bin[other_child]);
                                        binST.pop_back();
                                        */
                                    }
                                    binST.push_front(bin[ele.returnLeftNodeID()]); 
                                    binST.push_front(bin[ele.returnRightNodeID()]); 

                                }
                            }
                            else{
                                if(bin[ele.returnLeftNodeID()].getPathLen() + finalbin.size()%BLOCK_SIZE <= BLOCK_SIZE || flag==1){
                                    binST.push_front(bin[ele.returnRightNodeID()]); 
                                    binST.push_front(bin[ele.returnLeftNodeID()]); 
                                }
                                else{
                                    if((other_child) >= numClasses && (bin[other_child].getPathLen() + finalbin.size()%BLOCK_SIZE <= BLOCK_SIZE)){
                                        std::cout<<"case: \n";
                                       /*
                                        binST.push_front(bin[other_child]);
                                        binST.pop_back();
                                        */
                                    }
                                    binST.push_front(bin[ele.returnRightNodeID()]); 
                                    binST.push_front(bin[ele.returnLeftNodeID()]);
                                }
                            }
                        }

                        flag = 1;
                    }
                }

                int siz = finalbin.size();
                for (auto i=numClasses; i<siz; i++){
                    finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                    finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                }

            }



            /*
               inline void LeafStatLayout(){
               std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
               int numClasses = fpSingleton::getSingleton().returnNumClasses();
               std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
               std::deque<fpBaseNodeStat<T, Q>> binST;
               finalbin.clear();
               nodeNewIdx.clear();
            //set path leaf counts
            std::vector<int> leafIndices;
            for(int i=0; i<numClasses + binstr.returnNumTrees(); ++i) {
            bin[i].setParent(i);
            bin[i].setPathLeafCount(0);
            }

            for(int i=numClasses; i<bin.size(); ++i){
//set parent node
if(bin[i].returnLeftNodeID() >= numClasses){
bin[bin[i].returnLeftNodeID()].setParent(i);
}
if(bin[i].returnRightNodeID() >= numClasses){
bin[bin[i].returnRightNodeID()].setParent(i);
}

//set path leaf counts
bin[i].setPathLeafCount(bin[bin[i].getParent()].getPathLeafCount());
if(bin[i].returnLeftNodeID() < numClasses)
bin[i].setPathLeafCount( bin[i].getPathLeafCount() + bin[i].getLeftLeafCard() );
if(bin[i].returnRightNodeID() < numClasses)
bin[i].setPathLeafCount( bin[i].getPathLeafCount() + bin[i].getRightLeafCard() );

if(bin[i].returnLeftNodeID() < numClasses && bin[i].returnRightNodeID() < numClasses){
leafIndices.push_back(i);

}
}


int idnum = 1;
for(auto leaf_index: leafIndices){
int idx = leaf_index;
int idx_parent = 0;
idnum++;
while(1)
{
idx_parent = bin[idx].getParent();
if(idx_parent < numClasses +  binstr.returnNumTrees())
break;
if(bin[idx_parent].getPathLeafCount() >= bin[idx].getPathLeafCount())
break;
bin[idx_parent].setPathLeafCount(bin[idx].getPathLeafCount());
idx = idx_parent;
}
}
std::cout<<"past stage 2\n";
fflush(stdout);

for(auto i = 0; i < numClasses; ++i){
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
            push_insert(binST, bin[ele.returnRightNodeID()]);
        //binST.push_front(bin[ele.returnRightNodeID()]);

        else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
            push_insert(binST, bin[ele.returnLeftNodeID()]);
        //binST.push_front(bin[ele.returnLeftNodeID()]); 

        else{
            if(bin[ele.getPathLeafCount()].getCard() <= bin[ele.getPathLeafCount()].getCard()){
                push_insert(binST, bin[ele.returnLeftNodeID()]);
                push_insert(binST, bin[ele.returnRightNodeID()]);
                //binST.push_front(bin[ele.returnLeftNodeID()]); 
                //binST.push_front(bin[ele.returnRightNodeID()]); 
            }
            else{
                push_insert(binST, bin[ele.returnRightNodeID()]);
                push_insert(binST, bin[ele.returnLeftNodeID()]);
                //binST.push_front(bin[ele.returnRightNodeID()]); 
                //binST.push_front(bin[ele.returnLeftNodeID()]); 
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
*/

inline void LeafLayout(int depthIntertwined){
    std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
    std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();

    int numNodesToProc = std::pow(2, depthIntertwined) - 1; 
    auto numClasses = fpSingleton::getSingleton().returnNumClasses();

    std::vector<fpBaseNodeStat<T, Q>> newfinalbin;

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

    int interSize = finalbin.size();

    // STAT per (sub)tree layout 

    std::vector<int> vector_of_start_iters;
    std::vector<int> vector_of_end_iters;

    while(!binQ.empty()){
        std::deque<fpBaseNodeStat<T, Q>> binST;
        auto ele = binQ.front();
        binQ.pop_front();
        binST.push_back(ele);
        while(!binST.empty()){
            auto ele = binST.front();
            binST.pop_front(); 
            finalbin.push_back(ele);
            vector_of_start_iters.push_back(finalbin.size()-1);
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
        vector_of_end_iters.push_back(finalbin.size()-1);
    }

    int siz = finalbin.size();
    for (auto i=numClasses; i<siz; i++){
        finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
        finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
    }
    for(auto i = 0; i < binstr.returnNumTrees(); ++i)
        finalbin[numClasses + i].setParent(numClasses+i);

    //set parent node
    //set path leaf counts
    for(int i=numClasses; i<finalbin.size(); ++i){
        finalbin[i].setID(i);
        //set parent node
        if(finalbin[i].returnLeftNodeID() >= numClasses){
            finalbin[finalbin[i].returnLeftNodeID()].setParent(i);
        }
        if(finalbin[i].returnRightNodeID() >= numClasses){
            finalbin[finalbin[i].returnRightNodeID()].setParent(i);
        }

        //set path leaf counts
        finalbin[i].setPathLeafCount(finalbin[finalbin[i].getParent()].getPathLeafCount());
        if(finalbin[i].returnLeftNodeID() < numClasses)
            finalbin[i].setPathLeafCount( finalbin[i].getPathLeafCount() + finalbin[i].getLeftLeafCard() );
        if(finalbin[i].returnRightNodeID() < numClasses)
            finalbin[i].setPathLeafCount( finalbin[i].getPathLeafCount() + finalbin[i].getRightLeafCard() );

    }


    nodeNewIdx.clear();
    for(int i=0; i<interSize; ++i){
        newfinalbin.push_back(finalbin[i]);
        nodeNewIdx.insert(std::pair<int, int>(finalbin[i].getID(), i));
    }

    //iterate through each subtree
    for(auto i = 0; i < vector_of_start_iters.size(); ++i){
        auto start_st = vector_of_start_iters[i];
        auto end_st = vector_of_end_iters[i];
        while(1){
            int flag = 0;
            int max = -1;
            int max_idx = 0;
            for(auto j=start_st; j<end_st; j++){
                //search for leaf with highest pathLeafCount until all leaves are taken
                if(finalbin[j].returnLeftNodeID() < numClasses && finalbin[j].returnRightNodeID() < numClasses){
                    if(finalbin[j].getBlank() > 0){
                        flag = 1;
                        if(finalbin[j].getPathLeafCount() > max){
                            max = finalbin[j].getPathLeafCount();
                            max_idx = j;
                        }

                    }
                }
            }
            if(flag == 0)
                break;

            //extract path backwards from leaf till root
            std::deque<fpBaseNodeStat<T, Q>> tempST;
            auto node = finalbin[max_idx];
            int new_idx = max_idx;
            while(1)
            {
                if(finalbin[new_idx].getID() < start_st)
                    break;
                if(finalbin[new_idx].getBlank() < 0)
                    break;
                //append path to a temporary stack
                tempST.push_front(finalbin[new_idx]);
                finalbin[new_idx].setBlank(-1);
                new_idx = finalbin[new_idx].getParent();
            }

            //pop from tempST and push onto the newfinalbin
            while(!tempST.empty()) {
                auto ele = tempST.front();
                newfinalbin.push_back(ele);
                nodeNewIdx.insert(std::pair<int, int>(ele.getID(), newfinalbin.size()-1));
                tempST.pop_front();
            }

        }

    }
    siz = newfinalbin.size();
    for (auto i=numClasses; i<siz; i++){
        newfinalbin[i].setLeftValue(nodeNewIdx[finalbin[newfinalbin[i].returnLeftNodeID()].getID()]);
        newfinalbin[i].setRightValue(nodeNewIdx[finalbin[newfinalbin[i].returnRightNodeID()].getID()]);
    }
    finalbin.clear();
    for (auto i: newfinalbin)
        finalbin.push_back(i);
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
        //	std::cout<<"subtree_class: "<<subtree_class<<"\n";
        class_size_in_st[subtree_class]++;
        map_subtree_to_class[curr_subtree] = subtree_class;
        //old_subtree = curr_subtree;
        }
        /*std::cout<<"PRINTING CLASS: !!\n";
          for(auto const& item: map_subtree_to_class)
          std::cout<<item.first<<": "<<item.second<<"\n";
          std::cout<<"PRINTING SIZE: !!\n";
          for(auto const& item: map_subtree_to_size)
          std::cout<<item.first<<": "<<item.second<<"\n";
          */
        int siz = finalbin.size();

        newfinalbin.clear();
        for(int i = inter_siz; i<siz; ++i)
            newfinalbin.push_back(finalbin[i]);

        nodeNewIdx.clear();
        for(auto i=0; i < siz; ++i){
            nodeNewIdx.insert(std::pair<int, int>(finalbin[i].getID(), i));
        }


        /*std::cout<<"Printing map_subtree_to_size!\n";
          for(int i = 0; i<currLevel+3 ; i++)
          std::cout<<map_subtree_to_size[i]<<"\n";

          std::cout<<"Printing map_subtree_to_class!\n";
          for(int i = 0; i<currLevel+3 ; i++)
          std::cout<<map_subtree_to_class[i]<<"\n";

          std::cout<<"Printing class_size_in_st!\n";
          for(int i=0; i<20; ++i)
          {
          std::cout<<"Printing newfinalbin!\n";
          std::cout<<"*******************************************\n";	
          std::cout<<"i: "<<i<<" class_size_in_st[i]: "<<class_size_in_st[i]<<"\n";
          }
          */
        std::sort(newfinalbin.begin(), newfinalbin.end(), [this](auto l, auto r){return myCompFunction(l, r);} );
        finalbin.clear();
        for(auto i:newfinalbin2)
            finalbin.push_back(i);
        for(auto i:newfinalbin)
            finalbin.push_back(i);
        /*std::cout<<"Printing newfinalbin2!\n";
          std::cout<<"*******************************************\n";	
          for(auto i: newfinalbin2)
          {
          std::cout<<"id: "<<i.getID()<<"\n";
          std::cout<<"ST Num: "<<i.getSTNum()<<"\n";
          std::cout<<"class: "<<map_subtree_to_class[i.getSTNum()]<<"\n";
          std::cout<<"size: "<<map_subtree_to_size[i.getSTNum()]<<"\n";
          std::cout<<"*******************************************\n";	
          }*/	
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
            finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
            finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
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
