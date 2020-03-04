#ifndef binStruct_h
#define binStruct_h
#include "../../baseFunctions/fpBaseNode.h"
#include "../../baseFunctions/MWC.h"
#include "obsIndexAndClassVec.h"
#include "zipClassAndValue.h"
#include "processingNodeBin.h"
#include <vector>
#include <deque>
#include <map>
#include <assert.h>
#include <chrono>
#include <random>
#include <cstdlib>
#include <fstream>
#include <set>

#define NUM_FILES 10 
#define BLOCK_SIZE 128
int counter = 0;
std::fstream ff;
namespace fp{

	template <typename T, typename Q>
		class binStruct
		{
			protected:
				float OOBAccuracy;
				float correctOOB;
				float totalOOB;
				std::vector<processingNodeBin<T,Q> > nodeQueue;
				std::deque<processingNodeBin<T,Q> > nodeQueueInter;
				std::deque<processingNodeBin<T,Q> > nodeQueueRight;
				std::deque<processingNodeBin<T,Q> > nodeQueueLeft;
				int numberOfNodes;

				int currTree;
				int uid;
				std::map<int, int> nodeTreeMap;

				std::vector<obsIndexAndClassVec> indicesHolder;
				std::vector<zipClassAndValue<int, T> > zipper;

				std::vector<int> nodeIndices;

				randomNumberRerFMWC randNum;

				inline bool rightNode(){
					return false;
				}

				inline bool leftNode(){
					return true;
				}
				int numOfTreesInBin;

			public:
				std::vector< fpBaseNodeStat<T,Q> > bin;
				binStruct() : OOBAccuracy(-1.0),correctOOB(0),totalOOB(0),numberOfNodes(0),numOfTreesInBin(0),currTree(0), uid(fpSingleton::getSingleton().returnNumClasses())
			{
				//	ff.open("elapsed_time_bfs.csv",std::ios::app);

			}
				binStruct(int numTrees) : OOBAccuracy(-1.0),correctOOB(0),totalOOB(0),numberOfNodes(0),numOfTreesInBin(numTrees),currTree(0), uid(fpSingleton::getSingleton().returnNumClasses())
			{
				//	ff.open("elapsed_time_bfs.csv", std::ios::app);

			}

				binStruct(int numTrees, std::vector< fpBaseNodeStat<T,Q> > bin_here) : OOBAccuracy(-1.0),correctOOB(0),totalOOB(0),numberOfNodes(0),numOfTreesInBin(numTrees),currTree(0), uid(fpSingleton::getSingleton().returnNumClasses())
			{
				bin.clear();
				for(auto i: bin_here)
					bin.push_back(i);
				//bin = bin_here;
				ff.open("elapsed_time_bfs.csv", std::ios::app);

			}


				~binStruct(){
					ff.close();
				}
				inline std::vector< fpBaseNodeStat<T,Q> > getBin(){
					return bin;
				}

				inline int getNumTrees()
				{
					return numOfTreesInBin;
				}

				inline void setBin(std::vector< fpBaseNodeStat<T,Q> > newbin){
					bin = newbin;
				}

				inline void loadFirstNode(){
					nodeQueue.emplace_back(0,0,0,randNum);
					nodeQueue.back().setupRoot(indicesHolder[currTree], zipper);
					nodeQueue.back().processNode();
					if(nodeQueue.back().isLeafNode()){
						makeRootALeaf();
					}else{
						copyProcessedRootToBin();
						createRootChildNodes();
					}
				}

				inline void makeRootALeaf(){
					bin[returnRootLocation()].setClass(nodeQueue.back().returnNodeClass());
					bin[returnRootLocation()].setDepth(0);
				}


				inline void loadFirstNodeInter(){
					nodeQueueInter.emplace_back(currTree,0,0,randNum);
					nodeQueueInter.front().setupRoot(indicesHolder[currTree], zipper);
					nodeQueueInter.front().processNode();
					if(nodeQueueInter.front().isLeafNode()){
						makeRootALeafInter();
					}else{
						copyProcessedRootToBinInter();
						createRootChildNodesInter();
					}
				}

				inline void makeRootALeafInter(){
					bin[returnRootLocation()].setClass(nodeQueueInter.front().returnNodeClass());
					bin[returnRootLocation()].setDepth(0);
				}

				inline void setSharedVectors(){
					indicesHolder[currTree].resetVectors();

					int numUnusedObs = fpSingleton::getSingleton().returnNumObservations();
					int randomObsID;
					int tempMoveObs;

					for(int n = 0; n < fpSingleton::getSingleton().returnNumObservations(); n++){
						randomObsID = randNum.gen(fpSingleton::getSingleton().returnNumObservations());
						indicesHolder[currTree].insertIndex(nodeIndices[randomObsID], fpSingleton::getSingleton().returnLabel(nodeIndices[randomObsID]));

						if(randomObsID < numUnusedObs){
							--numUnusedObs;
							tempMoveObs = nodeIndices[numUnusedObs];
							nodeIndices[numUnusedObs] = nodeIndices[randomObsID];
							nodeIndices[randomObsID] = tempMoveObs;
						}
					}
				}

				inline bool shouldProcessNode(){
					return !nodeQueue.back().isLeafNode();
				}


				inline int positionOfNextNode(){
					return (int)bin.size()-1;
				}


				inline int parentNodesPosition(){
					return (int)bin.size()-1;
				}


				inline void makeLeafNodes(){
					for(int i= 0; i < fpSingleton::getSingleton().returnNumClasses(); ++i){
						bin[i].setSharedClass(i);
						bin[i].setID(i);
						bin[i].setSTNum(-100);
						//nodeTreeMap.insert(std::pair<int, int>(bin[i].getID(), -1));
					}
					for(int i= fpSingleton::getSingleton().returnNumClasses(); i < fpSingleton::getSingleton().returnNumClasses()+numOfTreesInBin; ++i){
						bin[i].setID(uid++);
						bin[i].setSTNum(i-fpSingleton::getSingleton().returnNumClasses());
						//nodeTreeMap.insert(std::pair<int, int>(bin[i].getID(), i-fpSingleton::getSingleton().returnNumClasses()));
					}
				}


				inline int returnDepthOfNode(){
					assert(!nodeQueue.empty());
					return bin[nodeQueue.back().returnParentNodeNumber()].returnDepth()+1;
				}


				inline int returnDepthOfNodeInter(){
					//	assert(!nodeQueueInter.empty());
					return bin[nodeQueueInter.front().returnParentNodeNumber()].returnDepth()+1;
				}


				inline void copyProcessedNodeToBin(){
					bin.emplace_back(nodeQueue.back().returnNodeCutValue(), returnDepthOfNode(), nodeQueue.back().returnNodeCutFeature(), uid++, nodeQueue.back().returnNodeSize());
					auto pos = bin.size()-1;
					bin[pos].setSTNum( nodeQueue.back().exposeTreeNum()); 
				}


				inline void copyProcessedNodeToBinInter(){
					bin.emplace_back(nodeQueueInter.front().returnNodeCutValue(), returnDepthOfNodeInter(), nodeQueueInter.front().returnNodeCutFeature(), uid++, nodeQueue.front().returnNodeSize());
					auto pos = bin.size()-1;
					bin[pos].setSTNum( nodeQueueInter.front().exposeTreeNum()); 
				}


				inline void copyProcessedRootToBin(){
					bin[returnRootLocation()].setCutValue(nodeQueue.back().returnNodeCutValue());
					bin[returnRootLocation()].setDepth(0);
					bin[returnRootLocation()].setFeatureValue(nodeQueue.back().returnNodeCutFeature());
				}

				inline void copyProcessedRootToBinInter(){
					bin[returnRootLocation()].setCutValue(nodeQueueInter.front().returnNodeCutValue());
					bin[returnRootLocation()].setDepth(0);
					bin[returnRootLocation()].setFeatureValue(nodeQueueInter.front().returnNodeCutFeature());
				}


				inline int returnRootLocation(){
					return currTree+fpSingleton::getSingleton().returnNumClasses();
				}


				inline void linkParentToChild(){
					if(nodeQueue.back().returnIsLeftNode()){
						bin[nodeQueue.back().returnParentNodeNumber()].setLeftValue(positionOfNextNode());
					}else{
						bin[nodeQueue.back().returnParentNodeNumber()].setRightValue(positionOfNextNode());
					}
				}


				inline void linkParentToLeaf(){
					assert(nodeQueue.back().returnParentNodeNumber() >= fpSingleton::getSingleton().returnNumClasses());
					assert(nodeQueue.back().returnParentNodeNumber() <= parentNodesPosition());

					assert(nodeQueue.back().returnNodeClass() >= 0);
					assert(nodeQueue.back().returnNodeClass() < fpSingleton::getSingleton().returnNumClasses());

					if(nodeQueue.back().returnIsLeftNode()){
						bin[nodeQueue.back().returnParentNodeNumber()].setLeftValue(nodeQueue.back().returnNodeClass());
					}else{
						bin[nodeQueue.back().returnParentNodeNumber()].setRightValue(nodeQueue.back().returnNodeClass());
					}
				}

				inline void linkParentToChildInter(){
					if(nodeQueueInter.front().returnIsLeftNode()){
						bin[nodeQueueInter.front().returnParentNodeNumber()].setLeftValue(positionOfNextNode());
					}else{
						bin[nodeQueueInter.front().returnParentNodeNumber()].setRightValue(positionOfNextNode());
					}
				}


				inline void linkParentToLeafInter(){
					assert(nodeQueueInter.front().returnParentNodeNumber() >= fpSingleton::getSingleton().returnNumClasses());
					assert(nodeQueueInter.front().returnParentNodeNumber() <= parentNodesPosition());

					assert(nodeQueueInter.front().returnNodeClass() >= 0);
					assert(nodeQueueInter.front().returnNodeClass() < fpSingleton::getSingleton().returnNumClasses());

					if(nodeQueueInter.front().returnIsLeftNode()){
						bin[nodeQueueInter.front().returnParentNodeNumber()].setLeftValue(nodeQueueInter.front().returnNodeClass());
					}else{
						bin[nodeQueueInter.front().returnParentNodeNumber()].setRightValue(nodeQueueInter.front().returnNodeClass());
					}
				}

				inline void createChildNodes(){
					nodeIterators nodeIts(nodeQueue.back().returnNodeIterators());
					zipperIterators<int,T> zipIts(nodeQueue.back().returnZipIterators());
					int childDepth = returnDepthOfNode()+1;
					if(nodeQueue.back().isLeftChildLarger()){
						nodeQueue.pop_back();
						//TODO: don't emplace_back if should be leaf node.
						nodeQueue.emplace_back(1,parentNodesPosition(), childDepth, randNum);
						nodeQueue.back().setupNode(nodeIts, zipIts, rightNode());
						nodeQueue.emplace_back(1,parentNodesPosition(), childDepth, randNum);
						nodeQueue.back().setupNode(nodeIts, zipIts, leftNode());
					}else{
						nodeQueue.pop_back();
						nodeQueue.emplace_back(1,parentNodesPosition(), childDepth, randNum);
						nodeQueue.back().setupNode(nodeIts, zipIts, leftNode());
						nodeQueue.emplace_back(1,parentNodesPosition(), childDepth, randNum);
						nodeQueue.back().setupNode(nodeIts, zipIts, rightNode());
					}
				}


				inline void createRootChildNodes(){
					nodeIterators nodeIts(nodeQueue.back().returnNodeIterators());
					zipperIterators<int,T> zipIts(nodeQueue.back().returnZipIterators());
					int childDepth = returnDepthOfNode()+1;
					if(nodeQueue.back().isLeftChildLarger()){
						nodeQueue.pop_back();
						//TODO: don't emplace_back if should be leaf node.
						nodeQueue.emplace_back(1,returnRootLocation(), childDepth,randNum);
						nodeQueue.back().setupNode(nodeIts, zipIts, rightNode());
						nodeQueue.emplace_back(1,returnRootLocation(), childDepth, randNum);
						nodeQueue.back().setupNode(nodeIts, zipIts, leftNode());
					}else{
						nodeQueue.pop_back();
						nodeQueue.emplace_back(1,returnRootLocation(), childDepth,randNum);
						nodeQueue.back().setupNode(nodeIts, zipIts, leftNode());
						nodeQueue.emplace_back(1,returnRootLocation(), childDepth,randNum);
						nodeQueue.back().setupNode(nodeIts, zipIts, rightNode());
					}
				}

				inline void createChildNodesInter(){
					nodeIterators nodeIts(nodeQueueInter.front().returnNodeIterators());
					zipperIterators<int,T> zipIts(nodeQueueInter.front().returnZipIterators());
					int childDepth = returnDepthOfNodeInter()+1;
					auto parentTreeNum = nodeQueueInter.front().exposeTreeNum();
					nodeQueueInter.pop_front();
					//TODO: don't emplace_back if should be leaf node.
					nodeQueueInter.emplace_back(parentTreeNum,parentNodesPosition(), childDepth, randNum);
					nodeQueueInter.back().setupNode(nodeIts, zipIts, leftNode());

					nodeQueueRight.emplace_back(parentTreeNum,parentNodesPosition(), childDepth, randNum);
					nodeQueueRight.back().setupNode(nodeIts, zipIts, rightNode());
				}


				inline void createRootChildNodesInter(){
					nodeIterators nodeIts(nodeQueueInter.front().returnNodeIterators());
					zipperIterators<int,T> zipIts(nodeQueueInter.front().returnZipIterators());
					int childDepth = returnDepthOfNodeInter()+1;
					auto parentTreeNum = nodeQueueInter.front().exposeTreeNum();

					nodeQueueInter.pop_front();
					nodeQueueLeft.emplace_back(parentTreeNum,returnRootLocation(), childDepth,randNum);
					nodeQueueLeft.back().setupNode(nodeIts, zipIts, leftNode());

					nodeQueueRight.emplace_back(parentTreeNum,returnRootLocation(), childDepth,randNum);
					nodeQueueRight.back().setupNode(nodeIts, zipIts, rightNode());

				}

				inline void processLeafNode(){
					assert(nodeQueue.back().returnNodeSize() > 0);
					assert(nodeQueue.back().returnNodeSize() <= fpSingleton::getSingleton().returnNumObservations());
					linkParentToLeaf();
					nodeQueue.pop_back();
				}

				inline void processLeafNodeInter(){
					assert(nodeQueueInter.front().returnNodeSize() > 0);
					assert(nodeQueueInter.front().returnNodeSize() <= fpSingleton::getSingleton().returnNumObservations());
					linkParentToLeafInter();
					nodeQueueInter.pop_front();
				}


				inline int returnNumTrees(){
					return numOfTreesInBin;
				}


				inline void processInternalNode(){
					copyProcessedNodeToBin();
					linkParentToChild();
					createChildNodes();
				}


				inline void processNode(){
					// process the node, i.e. calculate best split, ...
					nodeQueue.back().processNode();
					if (nodeQueue.back().isLeafNode()) {
						// label the processed node as a leaf.
						processLeafNode();
					}
					else {
						// label the processed node as internal.
						processInternalNode();
					}
				}

				inline void processInternalNodeInter(){
					copyProcessedNodeToBinInter();
					linkParentToChildInter();
					createChildNodesInter();
				}


				inline void processNodeInter(){
					// process the node, i.e. calculate best split, ...
					nodeQueueInter.front().processNode();
					if (nodeQueueInter.front().isLeafNode()) {
						// label the processed node as a leaf.
						processLeafNodeInter();
					}
					else {
						// label the processed node as internal.
						processInternalNodeInter();
					}
				}


				inline void intertwineRootsLayout(){
					for(; currTree < numOfTreesInBin; ++currTree){
						setSharedVectors();
						loadFirstNode();	
						while(!nodeQueue.empty())
							processNode();
					}
				}

				inline void intertwineMultipleLevelsLayout(int depthInter){
					for(; currTree < numOfTreesInBin; ++currTree){
						setSharedVectors();
						loadFirstNodeInter();
					}

					for(auto ele: nodeQueueLeft)
						nodeQueueInter.push_back(ele);

					for(auto ele: nodeQueueRight)
						nodeQueueInter.push_back(ele);
					nodeQueueRight.clear();

					int topNodeTreeNum=0, d = 0, numNodesInterleaved = std::pow(2, depthInter) - 2;
					while(d++ <  depthInter){
						for(int currTree=0; currTree< numOfTreesInBin; ++currTree){
							topNodeTreeNum=nodeQueueInter.front().exposeTreeNum();
							if(topNodeTreeNum != currTree)
								continue;
							processNodeInter();
						}

						while(!nodeQueueRight.empty()){
							processingNodeBin<T,Q> ele = nodeQueueRight.front();
							nodeQueueRight.pop_front();
							nodeQueueInter.push_back(ele);
						}
					}

					while(!nodeQueueInter.empty()){
						auto ele = nodeQueueInter.front();
						currTree = ele.exposeTreeNum(); 
						nodeQueueInter.pop_front();
						nodeQueue.emplace_back(ele);
						while(!nodeQueue.empty()){
							processNode();
						}
					}
				}

				inline void intertwineClassLayout(){
					int a =9;
				}

				inline void createBin(int numTrees, int randSeed, int depthInter){
					numOfTreesInBin = numTrees;
					randNum.initialize(randSeed);
					initializeStructures();
					auto numClasses = fpSingleton::getSingleton().returnNumClasses();
					for(int i=0; i<numClasses; ++i)
						bin[i].setID(i);	
					if(depthInter == 1)
						intertwineRootsLayout();
					else if (depthInter > 1)
						intertwineMultipleLevelsLayout(depthInter);
					else
						intertwineClassLayout();

					//printBin();    
					removeStructures();
				}

				inline void initializeStructures(){
					for(int i = 0; i < numOfTreesInBin; ++i)
						indicesHolder.push_back(obsIndexAndClassVec(fpSingleton::getSingleton().returnNumClasses()));

					zipper.resize(fpSingleton::getSingleton().returnNumObservations());
					nodeIndices.resize(fpSingleton::getSingleton().returnNumObservations());
					for(int i = 0; i < fpSingleton::getSingleton().returnNumObservations(); ++i){
						nodeIndices[i] =i;
					}
					bin.resize(numOfTreesInBin+fpSingleton::getSingleton().returnNumClasses());
					makeLeafNodes();
				}

				inline void removeStructures(){
					std::vector<processingNodeBin<T,Q> >().swap( nodeQueue );
					std::vector<zipClassAndValue<int, T> >().swap( zipper );
					std::vector<int>().swap( nodeIndices);
				}

				inline std::map<int, int> getNodeTreeMap(){
					return nodeTreeMap;
				}

				inline int returnMaxDepth(){
					int maxDepth=0;
					for(auto& node : bin){
						// +1 accounts for the leaf nodes which are never created (optimization that cuts memory required for a forest in half)
						if(maxDepth < node.returnDepth()+1){
							maxDepth = node.returnDepth()+1;
						}
					}
					return maxDepth;
				}

				inline int returnNumLeafNodes(){
					return (int)bin.size() - fpSingleton::getSingleton().returnNumClasses() + numOfTreesInBin;
				}


				inline int returnLeafDepthSum(){
					int leafDepthSums=0;
					for(auto& node : bin){
						if(node.isInternalNodeFront()){
							if(node.returnLeftNodeID() < fpSingleton::getSingleton().returnNumClasses()){
								leafDepthSums += node.returnDepth()+1;
							}
							if(node.returnRightNodeID() < fpSingleton::getSingleton().returnNumClasses()){
								leafDepthSums += node.returnDepth()+1;
							}
						}
					}
					return leafDepthSums;
				}

				/////////////////////////
				// This is required to template the predictObservation function
				// //////////////////////////////
				template<typename U>
					struct identity { typedef U type; };
				inline void predictBinObservation(int observationNum, std::vector<int>& preds){
					predictBinObservation(observationNum,preds, identity<Q>());
				}
				inline void predictBinObservation(int &uniqueCount, std::vector<int> roots, fpBaseNode<T, Q>* data, int observationNum, std::vector<int>& preds){
					predictBinObservation(uniqueCount, roots, data,observationNum,preds, identity<Q>());
				}

				inline void predictBinObservation(std::vector<T>& observation, std::vector<int>& preds){
					predictBinObservation(observation,preds,identity<Q>());
				}

				////////////////////////////////

				//PredictForRF
				inline void predictBinObservation(int observationNum,std::vector<int>& preds, identity<int> ){
					std::cout<<"Here 0\n";
					fflush(stdout);
					std::cout<<"Number of trees in bin: "<<numOfTreesInBin<<"\n";
					//numOfTreesInBin = 12;
					fflush(stdout);
					std::vector<int> currNode(12);
					int numberNotInLeaf;
					int featureNum;
					T featureVal;
					int q;
					std::cout<<"Here 1\n";
					fflush(stdout);
					for( q=0; q<numOfTreesInBin; ++q){
						currNode[q] = q+fpSingleton::getSingleton().returnNumClasses();
						__builtin_prefetch(&bin[currNode[q]], 0, 3);
					}
					std::cout<<"Here 2\n";
					fflush(stdout);

					do{
						numberNotInLeaf = 0;

						for( q=0; q<numOfTreesInBin; ++q){

							if(bin[currNode[q]].isInternalNodeFront()){
								featureNum = bin[currNode[q]].returnFeatureNumber();
								featureVal = fpSingleton::getSingleton().returnTestFeatureVal(featureNum,observationNum);
								currNode[q] = bin[currNode[q]].fpBaseNode<T, Q>::nextNode(featureVal);
								__builtin_prefetch(&bin[currNode[q]], 0, 3);
								++numberNotInLeaf;
							}
						}

					}while(numberNotInLeaf);

					std::cout<<"Here 3\n";
					fflush(stdout);
					for( q=0; q<numOfTreesInBin; q++){
						//#pragma omp atomic update
						++preds[bin[currNode[q]].returnClass()];
					}
				}



				inline void predictBinObservation(int observationNum, std::vector<int>& preds, identity<std::vector<int> >){
					std::vector<int> currNode(numOfTreesInBin);
					int numberNotInLeaf;
					T featureVal;
					int  q;


					for( q=0; q<numOfTreesInBin; ++q){
						currNode[q] = q+fpSingleton::getSingleton().returnNumClasses();
						__builtin_prefetch(&bin[currNode[q]], 0, 3);
					}

					do{
						numberNotInLeaf = 0;

						for( q=0; q<numOfTreesInBin; ++q){

							if(bin[currNode[q]].isInternalNodeFront()){
								featureVal = 0;
								for(auto i : bin[currNode[q]].returnFeatureNumber()){
									featureVal += fpSingleton::getSingleton().returnTestFeatureVal(i,observationNum);
								}
								currNode[q] = bin[currNode[q]].fpBaseNode<T, Q>::nextNode(featureVal);
								__builtin_prefetch(&bin[currNode[q]], 0, 3);
								++numberNotInLeaf;
							}
						}

					}while(numberNotInLeaf);

					for( q=0; q<numOfTreesInBin; q++){
#pragma omp atomic update
						++preds[bin[currNode[q]].returnClass()];
					}
				}


				inline void predictBinObservation(int observationNum, std::vector<int>& preds, identity<weightedFeature>){
					std::vector<int> currNode(numOfTreesInBin);
					int numberNotInLeaf;
					T featureVal;
					int weightNum;
					int  q;


					for( q=0; q<numOfTreesInBin; ++q){
						currNode[q] = q+fpSingleton::getSingleton().returnNumClasses();
						//__builtin_prefetch(&bin[currNode[q]], 0, 3);
					}

					do{
						numberNotInLeaf = 0;

						for( q=0; q<numOfTreesInBin; ++q){

							if(bin[currNode[q]].isInternalNodeFront()){
								featureVal = 0;
								weightNum = 0;
								for(auto i : bin[currNode[q]].returnFeatureNumber().returnFeatures()){
									featureVal += fpSingleton::getSingleton().returnTestFeatureVal(i,observationNum)*bin[currNode[q]].returnFeatureNumber().returnWeights()[weightNum++];
								}
								currNode[q] = bin[currNode[q]].fpBaseNode<T, Q>::nextNode(featureVal);
								//__builtin_prefetch(&bin[currNode[q]], 0, 3);
								++numberNotInLeaf;
							}
						}

					}while(numberNotInLeaf);

					for( q=0; q<numOfTreesInBin; q++){
#pragma omp atomic update
						++preds[bin[currNode[q]].returnClass()];
					}
				}

				/////////////////////////START HERE////////////////////////////////////
				inline void predictBinObservation(int &uniqueCount, std::vector<int>& roots, fpBaseNode<T, Q>*bin, int observationNum,std::vector<int>& preds, identity<int> ){
					std::vector<int> currNode(numOfTreesInBin);
					int numberNotInLeaf;
					int featureNum;
					T featureVal;
					int q;
					std::vector<int> v;
					std::vector<int> v_num_nodes;
					auto start = std::chrono::steady_clock::now();
					if(roots.size()>0){
						for( q=0; q<numOfTreesInBin; ++q){
							v_num_nodes.push_back(currNode[q]);
							currNode[q] = roots[q];
							//	__builtin_prefetch(&bin[currNode[q]], 0, 3);
							std::cout<<"ROOTS: "<<roots[q]<<"\n";
						}
					}
					else {
						for( q=0; q<numOfTreesInBin; ++q){
							v_num_nodes.push_back(currNode[q]);
							currNode[q] = q+fpSingleton::getSingleton().returnNumClasses();;
							//	__builtin_prefetch(&bin[currNode[q]], 0, 3);
						}
					}
					do{
						numberNotInLeaf = 0;

						for( q=0; q<numOfTreesInBin; ++q){
							if(bin[currNode[q]].isInternalNodeFront()){
								v.push_back(currNode[q]/BLOCK_SIZE);
								v_num_nodes.push_back(currNode[q]);
								featureNum = bin[currNode[q]].returnFeatureNumber();
								featureVal = fpSingleton::getSingleton().returnTestFeatureVal(featureNum,observationNum);
								currNode[q] = bin[currNode[q]].fpBaseNode<T, Q>::nextNode(featureVal);
								//__builtin_prefetch(&bin[currNode[q]], 0, 3);
								++numberNotInLeaf;
							}
						}

					}while(numberNotInLeaf);
					auto end = std::chrono::steady_clock::now();

					for( q=0; q<numOfTreesInBin; q++){
						//#pragma omp atomic update
						++preds[bin[currNode[q]].returnClass()];
					}

					std::cout<<"Number of nodes traversed: "<<v_num_nodes.size()<<"\n";  
					std::cout<<"Elapsed time: " <<std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()<<" nanoseconds.\n";
					ff<<std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()<<",";
					std::sort(v.begin(), v.end());
					uniqueCount = std::set<int>( v.begin(), v.end() ).size();
					std::cout<<"unique count: "<< uniqueCount<<"\n";

				}



				inline void predictBinObservation(int &uniqueCount, std::vector<int> roots, fpBaseNode<T, Q>*bin, int observationNum, std::vector<int>& preds, identity<std::vector<int> >){
					std::vector<int> currNode(numOfTreesInBin);
					int numberNotInLeaf;
					T featureVal;
					int  q;

					for( q=0; q<numOfTreesInBin; ++q){
						currNode[q] = q+fpSingleton::getSingleton().returnNumClasses();
						//__builtin_prefetch(&bin[(counter++)%NUM_FILES][currNode[q]], 0, 3);
					}

					do{
						numberNotInLeaf = 0;

						for( q=0; q<numOfTreesInBin; ++q){

							if(bin[currNode[q]].isInternalNodeFront()){
								featureVal = 0;
								for(auto i : bin[currNode[q]].returnFeatureNumber()){
									featureVal += fpSingleton::getSingleton().returnTestFeatureVal(i,observationNum);
								}
								currNode[q] = bin[currNode[q]].fpBaseNode<T, Q>::nextNode(featureVal);
								//__builtin_prefetch(&bin[(counter++)%NUM_FILES][currNode[q]], 0, 3);
								++numberNotInLeaf;
							}
						}

					}while(numberNotInLeaf);

					for( q=0; q<numOfTreesInBin; q++){
						//#pragma omp atomic update
						++preds[bin[currNode[q]].returnClass()];
					}
				}


				inline void predictBinObservation(int &uniqueCount, std::vector<int> roots, fpBaseNode<T, Q>*bin,int observationNum, std::vector<int>& preds, identity<weightedFeature>){
					std::vector<int> currNode(numOfTreesInBin);
					int numberNotInLeaf;
					T featureVal;
					int weightNum;
					int  q;
					for( q=0; q<numOfTreesInBin; ++q){
						currNode[q] = q+fpSingleton::getSingleton().returnNumClasses();
						//		__builtin_prefetch(&bin[(counter++)%NUM_FILES][currNode[q]], 0, 3);
					}

					do{
						numberNotInLeaf = 0;

						for( q=0; q<numOfTreesInBin; ++q){

							if(bin[currNode[q]].isInternalNodeFront()){
								featureVal = 0;
								weightNum = 0;
								for(auto i : bin[currNode[q]].returnFeatureNumber().returnFeatures()){
									featureVal += fpSingleton::getSingleton().returnTestFeatureVal(i,observationNum)*bin[currNode[q]].returnFeatureNumber().returnWeights()[weightNum++];
								}
								currNode[q] = bin[currNode[q]].fpBaseNode<T, Q>::nextNode(featureVal);
								//				__builtin_prefetch(&bin[(counter++)%NUM_FILES][currNode[q]], 0, 3);
								++numberNotInLeaf;
							}
						}

					}while(numberNotInLeaf);

					for( q=0; q<numOfTreesInBin; q++){
						//#pragma omp atomic update
						++preds[bin[currNode[q]].returnClass()];
					}
				}

				inline void predictBinObservation(std::vector<T>& observation, std::vector<int>& preds,identity<int> ){
					std::vector<int> currNode(numOfTreesInBin);
					int numberNotInLeaf;
					int featureNum;
					int q;

					for( q=0; q<numOfTreesInBin; ++q){
						currNode[q] = q+fpSingleton::getSingleton().returnNumClasses();
						__builtin_prefetch(&bin[currNode[q]], 0, 3);
					}

					do{
						numberNotInLeaf = 0;

						for( q=0; q<numOfTreesInBin; ++q){

							if(bin[currNode[q]].isInternalNodeFront()){
								featureNum = bin[currNode[q]].returnFeatureNumber();
								currNode[q] = bin[currNode[q]].fpBaseNode<T, Q>::nextNode(observation[featureNum]);
								__builtin_prefetch(&bin[currNode[q]], 0, 3);
								++numberNotInLeaf;
							}
						}

					}while(numberNotInLeaf);

					for( q=0; q<numOfTreesInBin; q++){
						//#pragma omp atomic update
						++preds[bin[currNode[q]].returnClass()];
					}
				}


				inline void predictBinObservation(std::vector<T>& observation, std::vector<int>& preds, identity<std::vector<int> >){
					std::vector<int> currNode(numOfTreesInBin);
					int numberNotInLeaf;
					T featureVal;
					int  q;


					for( q=0; q<numOfTreesInBin; ++q){
						currNode[q] = q+fpSingleton::getSingleton().returnNumClasses();
						__builtin_prefetch(&bin[currNode[q]], 0, 3);
					}

					do{
						numberNotInLeaf = 0;

						for( q=0; q<numOfTreesInBin; ++q){

							if(bin[currNode[q]].isInternalNodeFront()){
								featureVal = 0;
								for(auto i : bin[currNode[q]].returnFeatureNumber()){
									featureVal +=observation[i];
								}
								currNode[q] = bin[currNode[q]].fpBaseNode<T, Q>::nextNode(featureVal);
								__builtin_prefetch(&bin[currNode[q]], 0, 3);
								++numberNotInLeaf;
							}
						}

					}while(numberNotInLeaf);

					for( q=0; q<numOfTreesInBin; q++){
						//#pragma omp atomic update
						++preds[bin[currNode[q]].returnClass()];
					}
				}


				//Prediction function for ternary sparse matrix
				inline void predictBinObservation(std::vector<T>& observation, std::vector<int>& preds, identity<weightedFeature>){
					std::vector<int> currNode(numOfTreesInBin);
					int numberNotInLeaf;
					T featureVal;
					int weightNum;
					int  q;


					for( q=0; q<numOfTreesInBin; ++q){
						currNode[q] = q+fpSingleton::getSingleton().returnNumClasses();
						__builtin_prefetch(&bin[currNode[q]], 0, 3);
					}

					do{
						numberNotInLeaf = 0;

						for( q=0; q<numOfTreesInBin; ++q){

							if(bin[currNode[q]].isInternalNodeFront()){
								featureVal = 0;
								weightNum = 0;
								for(auto i : bin[currNode[q]].returnFeatureNumber().returnFeatures()){
									featureVal +=observation[i]*bin[currNode[q]].returnFeatureNumber().returnWeights()[weightNum++];
								}
								currNode[q] = bin[currNode[q]].fpBaseNode<T, Q>::nextNode(featureVal);
								__builtin_prefetch(&bin[currNode[q]], 0, 3);
								++numberNotInLeaf;
							}
						}

					}while(numberNotInLeaf);

					for( q=0; q<numOfTreesInBin; q++){
						//#pragma omp atomic update
						++preds[bin[currNode[q]].returnClass()];
					}
				}
				///////////////////////////////////
				/// Test Functions not to be used in production
				//////////////////////////////////


				inline std::vector< fpBaseNode<T,Q> >& exposeBinTest(){
					return bin;
				}


				void printBin(){
					int count=0;
					std::cout << "\n";
					for(auto nd : bin){
						std::cout<<count++<<": ";
						nd.printNode();
					}
				}

		};

}//fp
#endif //binStruct_h
