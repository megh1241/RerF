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
					bin[i+numClasses].setSTNum(-1*depthIntertwined);
					binQ.push_back(bin[i+numClasses]);
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

			/*	if(depthIntertwined == 1){
					for(auto i: bin)
						finalbin.push_back(i);
					return;
				}*/

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
				int stno = 0;
				double eps = 0;
				int card[20] = {0};
				int max = -1;
				int subtree_class = -1;
				std::memset(class_size_in_st, 0, 20*sizeof(class_size_in_st[0])); 
				
				//std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
				std::vector< fpBaseNodeStat<T,Q> > bin = binstr.getBin();
				//number of nodes of a binary tree as a function of height
				int numNodesToProc = std::pow(2, depthIntertwined) - 1; 
				//auto numClasses = fpSingleton::getSingleton().returnNumClasses();
				binstr.setNumTrees(128);
				fpSingleton::getSingleton().setNumClasses(10);
				auto numClasses = 10;
				std::cout<<"printing bin INIT\n";
				std::cout<<"*******************************************\n";

				for(auto i = 0; i<numClasses; ++i){
					finalbin.push_back(bin[i]);
                                        nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));

				}
				//roots of trees
				//for(auto i = 0; i < binstr.returnNumTrees(); ++i){
				for(auto i = 0; i < 128; ++i){
					//bin[i+numClasses].setSTNum(stno++);
					binQTemp.push_back(bin[i+numClasses]);
				}
				std::cout<<"*******************************************\n";
				std::cout<<"Printing queue!\n";
				// Intertwined levels
				int currLevel = 0; 
		
				int posInLevel = 0;
				//process intertwined(BIN) levels	
				//while(currLevel < numNodesToProc*binstr.returnNumTrees()) {
				while(currLevel < numNodesToProc*128) {
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
				for(int i=0; i<=currLevel*90; ++i)
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
				int old_subtree=-1;
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
						std::memset(card, 0, 20*sizeof(card[0])); 
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
						if( ele.returnClass()<numClasses){
							if(card[ele.returnClass()] == 0)
								num_classes_in_subtree++;
							card[ele.returnClass()] += ele.getCard();
							total_tree_card += ele.getCard();
							leaf_present = 1;
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
					new_st++;
					//compute max class
				//	if(curr_subtree != old_subtree){
						max = -1;
						subtree_class = 12;
				//	}
					//if(num_classes_in_subtree > 0)
					//	eps = 1 / (double)num_classes_in_subtree;
					if(total_tree_card > 0){
						for(int i=0; i<numClasses; ++i){
							std::cout<<"class num: "<<i<<"card[i]: "<<card[i]<<" / "<<total_tree_card<<"\n";
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
					std::cout<<"subtree_class: "<<subtree_class<<"\n";
					class_size_in_st[subtree_class]++;
					map_subtree_to_class[curr_subtree] = subtree_class;
					old_subtree = curr_subtree;
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


				std::cout<<"Printing map_subtree_to_size!\n";
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
				//std::sort(newfinalbin.begin(), newfinalbin.end(), [this](auto l, auto r){return myCompFunction(l, r);} );
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
