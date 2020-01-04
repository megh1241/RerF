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
#define NUM_FILES 900 

using namespace std::placeholders;
using std::placeholders::_1;
using std::placeholders::_2;

std::map<int, int> map_subtree_to_class;
std::map<int, int> map_subtree_to_size;

namespace fp{
	template<typename T, typename Q>
		class BinLayout
		{
			binStruct<T, Q> binstr;
			std::deque<fpBaseNodeStat<T, Q>> binQ;
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

				int numNodesToProc = std::pow(2, depthIntertwined) - 2; 
				auto numClasses = fpSingleton::getSingleton().returnNumClasses();

				if(depthIntertwined == 1){
					for(int i=0; i<binstr.returnNumTrees(); ++i)
						bin[i+numClasses].setSTNum(-1*depthIntertwined);
					for(auto i: bin)
						finalbin.push_back(i);
					return;
				}

				for(auto i = 0; i < numClasses; ++i){
					finalbin.push_back(bin[i]);
					nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
				}

				for(auto i = 0; i < binstr.returnNumTrees(); ++i) {
					bin[i+numClasses].setSTNum(-1*depthIntertwined);
					binQ.push_back(bin[i+numClasses]);
				}

				// Intertwined levels
				int currLevel = 0; 
				//if(nodeTreeMap[binQ.front().getID()] != i)
				//  continue;

				while(currLevel <= numNodesToProc*binstr.returnNumTrees()) {
					currLevel += 2;
					auto ele = binQ.front();
					binQ.pop_front();
					finalbin.push_back(ele);
					nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));
					if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
						continue;

					else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()){
						bin[ele.returnRightNodeID()].setSTNum(ele.getSTNum() + 1);
						binQ.push_back(bin[ele.returnRightNodeID()]);
					}

					else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()){
						bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum() + 1);
						binQ.push_back(bin[ele.returnLeftNodeID()]); 
					}
					else {
						bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum() + 1);
						binQ.push_back(bin[ele.returnLeftNodeID()]); 
						bin[ele.returnRightNodeID()].setSTNum(ele.getSTNum() + 1);
						binQ.push_back(bin[ele.returnRightNodeID()]); 
					}
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

				int numNodesToProc = std::pow(2, depthIntertwined) - 2; 
				auto numClasses = fpSingleton::getSingleton().returnNumClasses();

				if(depthIntertwined == 1){
					for(auto i: bin)
						finalbin.push_back(i);
					return;
				}


				for(auto i = 0; i < numClasses; ++i){
					finalbin.push_back(bin[i]);
					nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
				}

				for(auto i = 0; i < binstr.returnNumTrees(); ++i)
					binQ.push_back(bin[i+numClasses]);

				// Intertwined levels
				int currLevel = 0; 
				//if(nodeTreeMap[binQ.front().getID()] != i)
				//  continue;

				while(currLevel <= numNodesToProc*binstr.returnNumTrees()) {
					currLevel += 2;
					auto ele = binQ.front();
					binQ.pop_front();
					finalbin.push_back(ele);
					nodeNewIdx.insert(std::pair<int, int>(ele.getID(), finalbin.size()-1));
					if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
						continue;

					else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses())
						binQ.push_back(bin[ele.returnRightNodeID()]);

					else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
						binQ.push_back(bin[ele.returnLeftNodeID()]); 

					else {
						if(bin[ele.returnLeftNodeID()].getCard() > bin[ele.returnRightNodeID()].getCard()){
							binQ.push_back(bin[ele.returnLeftNodeID()]); 
							binQ.push_back(bin[ele.returnRightNodeID()]); 
						}
						else{
							binQ.push_back(bin[ele.returnRightNodeID()]); 
							binQ.push_back(bin[ele.returnLeftNodeID()]); 
						}
					}

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
							if(bin[ele.returnLeftNodeID()].getCard() > bin[ele.returnRightNodeID()].getCard()){
								binQ.push_back(bin[ele.returnLeftNodeID()]); 
								binQ.push_back(bin[ele.returnRightNodeID()]); 
							}
							else{
								binQ.push_back(bin[ele.returnRightNodeID()]); 
								binQ.push_back(bin[ele.returnLeftNodeID()]); 
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
				if(node1.getSTNum() == node2.getSTNum())
					return node1.getID() < node2.getID();

				// < 0 indicates BIN and class nodes
				if(node1.getSTNum() < 0 || node2.getSTNum() < 0)
					return node1.getSTNum() < node2.getSTNum();

				//sort by class if nodes belong to subtrees of different majority class
				if(map_subtree_to_class[node1.getSTNum()] != map_subtree_to_class[node2.getSTNum()])
					return map_subtree_to_class[node1.getSTNum()] < map_subtree_to_class[node1.getSTNum()];

				//if classes are the same sort by size of subtree
				return map_subtree_to_size[node1.getSTNum()] < map_subtree_to_size[node2.getSTNum()];
			}

			inline void BINStatClassLayout(int depthIntertwined){
				int total_tree_card = 0;
				int num_classes_in_subtree = 0;
				//std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
				std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();

				int numNodesToProc = std::pow(2, depthIntertwined) - 2; 
				auto numClasses = fpSingleton::getSingleton().returnNumClasses();
				/*if(depthIntertwined == 1){
				  for(auto i: bin)
				  finalbin.push_back(i);
				  return;
				  }*/


				if(depthIntertwined == 1)
					numNodesToProc = 1;

				for(auto i = 0; i < numClasses; ++i){
					finalbin.push_back(bin[i]);

				}

				for(auto i = 0; i < binstr.returnNumTrees(); ++i){
					bin[i+numClasses].setSTNum(-1*depthIntertwined);
					binQ.push_back(bin[i+numClasses]);
				}

				// Intertwined levels
				int currLevel = 0; 
				while(currLevel <= numNodesToProc*binstr.returnNumTrees()) {
					currLevel += 2;
					auto ele = binQ.front();
					binQ.pop_front();
					finalbin.push_back(ele);
					if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
						continue;

					else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()){
						bin[ele.returnRightNodeID()].setSTNum(ele.getSTNum() + 1);
						binQ.push_back(bin[ele.returnRightNodeID()]);
					}

					else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()){
						bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum() + 1);
						binQ.push_back(bin[ele.returnLeftNodeID()]); 
					}

					else {
						bin[ele.returnLeftNodeID()].setSTNum(ele.getSTNum() + 1);
						binQ.push_back(bin[ele.returnLeftNodeID()]); 
						bin[ele.returnRightNodeID()].setSTNum(ele.getSTNum() + 1);
						binQ.push_back(bin[ele.returnRightNodeID()]); 
					}

				}

				// STAT per (sub)tree layout 
				int stno = -1; 
				int numNodesInST = 0;
				while(!binQ.empty()){
					std::deque<fpBaseNodeStat<T, Q>> binST;
					auto ele = binQ.front();
					binQ.pop_front();
					binST.push_back(ele);
					total_tree_card = 0;
					num_classes_in_subtree=0;
					stno++;
					numNodesInST = 0;

					int card[10] = {0};
					while(!binST.empty()){
						auto ele = binST.front();
						numNodesInST++;
						ele.setSTNum(stno);
						//if ele is a leaf node, then check the class and cardinality
						if(!ele.isInternalNode() && ele.returnClass()<numClasses){
							total_tree_card += ele.getCard();
							if(card[ele.returnClass()] == 0)
								num_classes_in_subtree++;
							card[ele.returnClass()] += ele.getCard();
						}
						binST.pop_front(); 
						finalbin.push_back(ele);
						if((ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) && (ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()))
							continue;

						else if(ele.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()) {
							binST.push_back(bin[ele.returnRightNodeID()]);
							bin[ele.returnRightNodeID()].setSTNum(stno);
						}

						else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()){
							binST.push_back(bin[ele.returnLeftNodeID()]); 
							bin[ele.returnLeftNodeID()].setSTNum(stno);
						}
						else {
							if(bin[ele.returnLeftNodeID()].getCard() > bin[ele.returnRightNodeID()].getCard()){
								binST.push_back(bin[ele.returnLeftNodeID()]); 
								bin[ele.returnLeftNodeID()].setSTNum(stno);
								binST.push_back(bin[ele.returnRightNodeID()]); 
								bin[ele.returnRightNodeID()].setSTNum(stno);
							}
							else{
								binST.push_back(bin[ele.returnRightNodeID()]); 
								bin[ele.returnRightNodeID()].setSTNum(stno);
								binST.push_back(bin[ele.returnLeftNodeID()]); 
								bin[ele.returnLeftNodeID()].setSTNum(stno);
							}
						}
					}
					//compute max class
					int max = -1;
					int subtree_class = -1;
					double eps = 0.1;
					//if(num_classes_in_subtree > 0)
					//	eps = 1 / (double)num_classes_in_subtree;
					if(total_tree_card > 0){
						for(int i=0; i<numClasses; ++i){
							if(card[i] > max && ((double)(card[i]) / (double)(total_tree_card) > eps)){
								subtree_class = card[i];
							}
						}
					}
					map_subtree_to_class[stno] = subtree_class;
					map_subtree_to_size[stno] = numNodesInST;
				}
				int siz = finalbin.size();
				std::sort(finalbin.begin(), finalbin.end(), [this](auto l, auto r){return myCompFunction(l, r);} );
				nodeNewIdx.clear();
				for(auto i=0; i < siz; ++i)
					nodeNewIdx.insert(std::pair<int, int>(finalbin[i].getID(), i));

				for (auto i=numClasses; i<siz; i++){
					finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
					finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
				}
				nodeNewIdx.clear();

				std::cout<<"Size of final bin in binstatclass!!!!!!!:  "<<siz<<"\n";
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
							binST.push_back(bin[ele.returnRightNodeID()]);

						else if(ele.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses())
							binST.push_back(bin[ele.returnLeftNodeID()]); 

						else{
							if(bin[ele.returnLeftNodeID()].getCard() > bin[ele.returnRightNodeID()].getCard()){ 
								binST.push_back(bin[ele.returnLeftNodeID()]); 
								binST.push_back(bin[ele.returnRightNodeID()]);
							}
							else{
								if(bin[ele.returnLeftNodeID()].getCard() > bin[ele.returnRightNodeID()].getCard()){
									binST.push_back(bin[ele.returnLeftNodeID()]); 
									binST.push_back(bin[ele.returnRightNodeID()]); 
								}
								else{
									binST.push_back(bin[ele.returnRightNodeID()]); 
									binST.push_back(bin[ele.returnLeftNodeID()]); 
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
