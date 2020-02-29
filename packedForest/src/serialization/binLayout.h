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
				for(auto i: binST){
					if ( ((float)(ele.getID())/(float)(ele.returnDepth())) < ((float)(i.getID())/(float)(i.returnDepth())))
					{
						pos = count;
						break;
					}
					
					count++;
				}
				if(pos == 0)
					pos = count;
				binST.insert(binST.begin() + pos, ele);
			}


			/*inline void leafLayout(int depthIntertwined){
				std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
				std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();

				int numNodesToProc = std::pow(2, depthIntertwined) -1 ; 
				auto numClasses = fpSingleton::getSingleton().returnNumClasses();


				for(int i=0; i<bin.size(); ++i){
					if(i < (numClasses + binstr.returnNumTrees()))
						continue;
					
					if(bin[i].returnLeftNodeID() >= numClasses)
						bin[bin[i].returnLeftNodeID()].setParentID(bin[i].getID());
					if(bin[i].returnRightNodeID() >= numClasses)
						bin[bin[i].returnRightNodeID()].setParentID(bin[i].getID());
				}

				
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


					
					
					
				for(auto st_iter = 0; st_iter <mainST.size(); st_iter++)
				{

					binST = mainST[st_iter];
					// STAT per (sub)tree layout
					int n_leaves = 0; 
					int minID = INT_MAX;
					for(int i=0; i<binST.size(); ++i)
					{
						if(binST[i].returnLeftNodeID() < numClasses && binST[i].returnRightNodeID() < numClasses)
							n_leaves++;
						binST[binST[i].returnLeftNodeID()].setParentNode(binST[i].getID());
						binST[binST[i].returnRightNodeID()].setParentNode(binST[i].getID());
						
						if(binST[i].returnLeftNodeID() < numClasses || binST[i].returnRightNodeID() < numClasses)
							binST[i].setLeafPathCount(binST[binST[i].returnParentNode()].pathLeafCount() + getCard());
						else
							binST[i].setPathLeafCount(binST[binST[i].returnParentNode()].pathLeafCount());
					}

					for(int i=0; i<n_leaves; i++)
					{
						int max = -1;
						int index_bin = 0;
						int count = 0;
						for(auto node: binST)
						{
							if(node.returnLeftNodeID() < numClasses && node.returnRightNodeID() < numClasses && node.returnBlank() > 0)
							{
								if (node.pathLeafCount() >= max){
									max = node.pathLeafCount();
									index_bin = count;	        	
								}
							}
							count++;
							if(node.getID() < minID)
								minID = node.getID();
						}

						auto start_node = binST[index_bin];
						auto node = start_node;
						std::deque<fpBaseNodeStat<T, Q>> tempStack;
						while(1)
						{
							//TODO check that parent is also within ST and that we haven't gone as far up as bin
							if(node.returnBlank() < 0)
								break;
							
							tempStack.push_front(node);
							binST[node].setBlank(-1);
							if(node.getParentNode() == -100)
								break;
							node = binST[node.getParentNode()];
						}
						while(!tempStack.empty())
						{
							auto ele = tempStack.pop_front();
							finalbin.push_back(ele);
							nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));
						}

					}
				}

			}*/


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
						finalbin.push_back(ele);
						nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));
						if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
							continue;

						else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses())
							binST.push_front(bin[ele.returnRightNodeID()]);

						else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
							binST.push_front(bin[ele.returnLeftNodeID()]); 

						else {
							//std::cout<<"Left card: "<<bin[ele.returnLeftNodeID()].getCard()<<"\n";
							//std::cout<<"right card: "<<bin[ele.returnRightNodeID()].getCard()<<"\n";
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
				/*std::cout<<"PRINTING BIN\n";
				std::fstream fbin, fbin2;
				fbin.open("bin_init.txt", std::ios::out);
				for(auto i: bin){
					fbin<<i.getID()<<",";
					fbin<<i.returnLeftNodeID()<<",";
					fbin<<i.returnRightNodeID()<<",";
					fbin<<i.returnCutValue()<<"\n";
				}
				fbin.close();
				std::cout<<"PRINTING FINAL BIN\n";
				fbin2.open("bin_final.txt", std::ios::out);
				for(auto i: finalbin){
					fbin2<<i.getID()<<",";
					fbin2<<i.returnLeftNodeID()<<",";
					fbin2<<i.returnRightNodeID()<<",";
					fbin2<<i.returnCutValue()<<"\n";
				}
				fbin2.close();
				*/
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
				std::vector<int> leaf_card_vec;
				std::memset(class_size_in_st, 0, 102*sizeof(class_size_in_st[0])); 
				
				//std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
				std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
				//number of nodes of a binary tree as a function of height
				int numNodesToProc = std::pow(2, depthIntertwined) - 1; 
				auto numClasses = fpSingleton::getSingleton().returnNumClasses();
				//binstr.setNumTrees(128);
				//fpSingleton::getSingleton().setNumClasses(10);
				//auto numClasses = 10;

			/*	
			 	for(auto i = 0; i<numClasses; ++i){
					finalbin.push_back(bin[i]);
                                        nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
				}
			*/
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
				int first_tree = 0;
				std::vector<std::vector<double>> per_subtree_dist;
				while(!binQ.empty()){
					leaf_present = 0;
					std::deque<fpBaseNodeStat<T, Q>> binST;
					auto ele = binQ.front();
					leaf_card_vec.push_back(ele.getCard());
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
							//leaf_card_vec.push_back(ele.getLeftLeafCard());
                                                        if(card[ele.returnLeftNodeID()] == 0)
                                                                num_classes_in_subtree++;
                                                        card[ele.returnLeftNodeID()] += ele.getLeftLeafCard();
							map_subtree_to_size[ele.getSTNum()]+=ele.getLeftLeafCard();
                                                        total_tree_card += ele.getLeftLeafCard();
                                                        leaf_present = 1;
                                                }
                                                if( ele.returnRightNodeID()<numClasses){
							//leaf_card_vec.push_back(ele.getRightLeafCard());
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
							//binST.push_front(bin[ele.returnRightNodeID()]);
							push_insert(binST, bin[ele.returnRightNodeID()]);
						}

						else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()){
							bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum());
							//binST.push_front(bin[ele.returnLeftNodeID()]); 
							push_insert(binST, bin[ele.returnLeftNodeID()]);
						}
						else {
							if(bin[ele.returnLeftNodeID()].getCard() <=  bin[ele.returnRightNodeID()].getCard()){
								bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum());
								//binST.push_front(bin[ele.returnLeftNodeID()]); 
								bin[ele.returnRightNodeID()].setSTNum(ele.getSTNum());
								//binST.push_front(bin[ele.returnRightNodeID()]); 
								push_insert(binST, bin[ele.returnLeftNodeID()]);
								push_insert(binST, bin[ele.returnRightNodeID()]);
							}
							else{
								bin[ele.returnRightNodeID()].setSTNum(ele.getSTNum());
								//binST.push_front(bin[ele.returnRightNodeID()]); 
								bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum());
								//binST.push_front(bin[ele.returnLeftNodeID()]); 
								push_insert(binST, bin[ele.returnRightNodeID()]);
								push_insert(binST, bin[ele.returnLeftNodeID()]);
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
						std::vector<double> card_frac;
						for(int i=0; i<numClasses; ++i){
				//			
				//			std::cout<<"class num: "<<i<<"card[i]: "<<card[i]<<" / "<<total_tree_card<<"\n";
							if(card[i] > max && ((double)(card[i]) / (double)(total_tree_card) > eps)){
								subtree_class = i;
								max = card[i];
							}
							
							//if(first_tree == 0){	
								card_frac.push_back((double)(card[i]));
								//card_frac.push_back((double)(card[i]) / (double)(total_tree_card));
							//}
						}
						per_subtree_dist.push_back(card_frac);
					}
					else
					{
						std::cout<<"total_tree_card != 0!!!!\n";
						std::cout<<"leaf_present: "<<leaf_present<<"\n";
					}
				//	std::cout<<"subtree_class: "<<subtree_class<<"\n";
					class_size_in_st[subtree_class]++;
					map_subtree_to_class[curr_subtree] = subtree_class;
					if(first_tree == 0)
						first_tree = 1;
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
			 	for(auto i = 0; i<numClasses; ++i){
					finalbin.push_back(bin[i]);
                                        //nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
				}
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
				siz = finalbin.size();
				for(auto i=0; i < siz; ++i){
					nodeNewIdx.insert(std::pair<int, int>(finalbin[i].getID(), i));
				}
				siz = finalbin.size();
				for (auto i=0; i<siz-numClasses; i++){
					finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
					finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
				}
				nodeNewIdx.clear();
				std::fstream fsiz;
				fsiz.open("st_card.csv", std::ios::out);
				for(auto size_item: leaf_card_vec)
					fsiz<<size_item<<",";
				fsiz.close();
				
				fsiz.open("per_subtree.csv", std::ios::out);
				for(int i=0; i<per_subtree_dist.size(); ++i)
				{
					for(int j=0; j<per_subtree_dist[0].size(); ++j)
					{
						fsiz<<per_subtree_dist[i][j]<<",";
					}
					fsiz<<"\n";
				}
				/*
				for(auto st: per_subtree_dist){
					for(auto ele: st)
						fsiz<<ele<<",";
					fsiz<<"\n";
				}*/
				fsiz.close();


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
							push_insert(binST, bin[ele.returnRightNodeID()]);
							//binST.push_front(bin[ele.returnRightNodeID()]);
						else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
							push_insert(binST, bin[ele.returnLeftNodeID()]);
							//binST.push_front(bin[ele.returnLeftNodeID()]); 

						else{
								std::cout<<"printing left and right cardinalities!\n";
								std::cout << bin[ele.returnLeftNodeID()].getCard() <<"\n";
								std::cout << bin[ele.returnRightNodeID()].getCard() <<"\n"; 
								if(bin[ele.returnLeftNodeID()].getCard() <= bin[ele.returnRightNodeID()].getCard()){
									//binST.push_front(bin[ele.returnLeftNodeID()]); 
									//binST.push_front(bin[ele.returnRightNodeID()]); 
									push_insert(binST, bin[ele.returnLeftNodeID()]);
									push_insert(binST, bin[ele.returnRightNodeID()]);
								}
								else{
									std::cout<<"else!!\n";
									//binST.push_front(bin[ele.returnRightNodeID()]); 
									//binST.push_front(bin[ele.returnLeftNodeID()]); 
									push_insert(binST, bin[ele.returnRightNodeID()]);
									push_insert(binST, bin[ele.returnLeftNodeID()]);
								}
						}
					}
				}

				int siz = finalbin.size();
				for (auto i=numClasses; i<siz; i++){
					finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
					finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
					//finalbin[i].setDepth(bin[nodeNewIdx[i]].returnDepth());
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
