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
#define NUM_FILES 90 

using namespace std::placeholders;
using std::placeholders::_1;
using std::placeholders::_2;

std::map<int, int> map_subtree_to_class;
std::map<int, int> map_subtree_to_size;
int class_size_in_st[20];

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

					if(posInLevel == binstr.returnNumTrees() - 1){
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

					if(posInLevel == binstr.returnNumTrees() - 1){
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

			inline bool myCompFunction(fpBaseNodeStat<T, Q> &node1, fpBaseNodeStat<T, Q> &node2)
			{
				//If nodes belong to the same subtree sort by new node index
				if(node1.getSTNum() == node2.getSTNum())
					return nodeNewIdx[node1.getID()] < nodeNewIdx[node2.getID()];

				//sort by class if nodes belong to subtrees of different majority class
				if(map_subtree_to_class[node1.getSTNum()] != map_subtree_to_class[node2.getSTNum()])
					return map_subtree_to_class[node1.getSTNum()] > map_subtree_to_class[node2.getSTNum()];

				return nodeNewIdx[node1.getID()] < nodeNewIdx[node2.getID()];
				
			}

			inline void BINStatClassLayout(int depthIntertwined){
				int total_tree_card = 0;
				int num_classes_in_subtree = 0;
				double eps = 0;
				int card[20] = {0};
				int max = -1;
				int subtree_class = fpSingleton::getSingleton().returnNumClasses();

				std::memset(class_size_in_st, 0, 20*sizeof(class_size_in_st[0])); 
				
				//std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
				std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();

				//number of nodes of a binary tree as a function of height
				int numNodesToProc = std::pow(2, depthIntertwined) - 1; 
				auto numClasses = fpSingleton::getSingleton().returnNumClasses();


				for(auto i = 0; i<numClasses; ++i){
					finalbin.push_back(bin[i]);
                                        nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));

				}
				//roots of trees
				for(auto i = 0; i < binstr.returnNumTrees(); ++i){
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

					if(posInLevel == binstr.returnNumTrees() - 1){
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
				for(int i=0; i<=currLevel*4; ++i)
					map_subtree_to_size[i] = 0;

				std::vector<fpBaseNodeStat<T, Q>> newfinalbin;
				std::vector<fpBaseNodeStat<T, Q>> newfinalbin2;
				for(auto i: finalbin)
					newfinalbin2.push_back(i);	
				
				int inter_siz = newfinalbin2.size();

				// STAT per (sub)tree layout 
		
				int new_st = currLevel;
				while(!binQ.empty()){
					std::deque<fpBaseNodeStat<T, Q>> binST;
					auto ele = binQ.front();
					binQ.pop_front();
					ele.setSTNum(new_st);
					binST.push_back(ele);
					std::memset(card, 0, 20*sizeof(card[0])); 
					total_tree_card = 0;
					num_classes_in_subtree=0;

					while(!binST.empty()){
						auto ele = binST.front();
						ele.setSTNum(new_st);
						//if ele is a leaf node, then check the class and cardinality
						if( ele.returnClass()<numClasses){
							if(card[ele.returnClass()] == 0)
								num_classes_in_subtree++;
							card[ele.returnClass()] += ele.getCard();
							total_tree_card += ele.getCard();
						}
						map_subtree_to_size[ele.getSTNum()]++;
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
					//compute max class
					max = -1;
					subtree_class = fpSingleton::getSingleton().returnNumClasses();
					
					if(total_tree_card > 0){
						for(int i=0; i<numClasses; ++i){
							//std::cout<<"class num: "<<i<<"card[i]: "<<card[i]<<" / "<<total_tree_card<<"\n";
							if(card[i] > max && ((double)(card[i]) / (double)(total_tree_card) > eps)){
								subtree_class = i;
								max = card[i];
							}
						}
					}
					class_size_in_st[subtree_class]++;
					map_subtree_to_class[new_st] = subtree_class;
					new_st++;
				}
				int siz = finalbin.size();
				
				newfinalbin.clear();
				for(int i = inter_siz; i<siz; ++i)
					newfinalbin.push_back(finalbin[i]);

				nodeNewIdx.clear();
				for(auto i=0; i < siz; ++i)
					nodeNewIdx.insert(std::pair<int, int>(finalbin[i].getID(), i));

				std::sort(newfinalbin.begin(), newfinalbin.end(), [this](auto l, auto r){return myCompFunction(l, r);} );
				finalbin.clear();
				for(auto i:newfinalbin2)
					finalbin.push_back(i);
				for(auto i:newfinalbin)
					finalbin.push_back(i);
					
			
				nodeNewIdx.clear();
				for(auto i=0; i < siz; ++i)
					nodeNewIdx.insert(std::pair<int, int>(finalbin[i].getID(), i));
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
							if(bin[ele.returnLeftNodeID()].getCard() > bin[ele.returnRightNodeID()].getCard()){ 
								binST.push_front(bin[ele.returnLeftNodeID()]); 
								binST.push_front(bin[ele.returnRightNodeID()]);
							}
							else{
								if(bin[ele.returnLeftNodeID()].getCard() <= bin[ele.returnRightNodeID()].getCard()){
									binST.push_front(bin[ele.returnRightNodeID()]); 
									binST.push_front(bin[ele.returnLeftNodeID()]); 
								}
								else{
									binST.push_front(bin[ele.returnLeftNodeID()]); 
									binST.push_front(bin[ele.returnRightNodeID()]); 
								}
							}	    
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


			inline void writeToFileStat(){
				std::ofstream f;
				fpBaseNodeStat<T,Q> nodeToWrite;
				for(int j = 0; j < NUM_FILES; j++){
					f.open((filename + std::to_string(j) + ".bin").c_str(), std::ios::out|std::ios::binary);
					for(auto i: finalbin){
						nodeToWrite = i;
						f.write((char*)&nodeToWrite, sizeof(nodeToWrite));
					}
					f.close();
				}
			}

			inline void writeToFile(std::vector<int> roots){
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
					f.open(treeroot_fname.c_str(), std::ios::out);
					for (auto root: roots)
						f<<root<<"\n";
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
