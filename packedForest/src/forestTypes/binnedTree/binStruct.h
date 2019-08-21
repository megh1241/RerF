#ifndef binStruct_h
#define binStruct_h
#include "../../baseFunctions/fpBaseNode.h"
#include "../../baseFunctions/MWC.h"
#include "obsIndexAndClassVec.h"
#include "zipClassAndValue.h"
#include "processingNodeBin.h"
#include <vector>
#include <deque>
#include <assert.h>

namespace fp{

	template <typename T, typename Q>
		class binStruct
		{
			protected:
				float OOBAccuracy;
				float correctOOB;
				float totalOOB;
				std::vector< fpBaseNode<T,Q> > bin;
				std::vector<processingNodeBin<T,Q> > nodeQueue;
				std::vector<processingNodeBin<T,Q> > nodeQueueInter;
				std::vector<processingNodeBin<T,Q> > nodeQueueRight;
				std::vector<processingNodeBin<T,Q> > nodeQueueLeft;

				int numberOfNodes;

				int numOfTreesInBin;
				int currTree;

                std::vector<obsIndexAndClassVec> indicesHolder;
				std::vector<zipClassAndValue<int, T> > zipper;

				std::vector<int> nodeIndices;


				randomNumberRerFMWC randNum;

				//obsIndexAndClassVec indexHolder(numClasses);
				//std::vector<zipClassAndValue<int, float> > zipVec(testSize);

				inline bool rightNode(){
					return false;
				}

				inline bool leftNode(){
					return true;
				}

			public:
				binStruct() : OOBAccuracy(-1.0),correctOOB(0),totalOOB(0),numberOfNodes(0),numOfTreesInBin(0),currTree(0){
                    int numClasses = fpSingleton::getSingleton().returnNumClasses();
                    int numTrees = 10;
                    for(int i =0; i<numTrees; i++)
                        indicesHolder.push_back(obsIndexAndClassVec(numClasses));
                }


				inline void loadFirstNode(){
					//inline void loadFirstNode(obsIndexAndClassVec& indicesHolder, std::vector<zipClassAndValue<int, T> >& zipper){
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
					//inline void loadFirstNode(obsIndexAndClassVec& indicesHolder, std::vector<zipClassAndValue<int, T> >& zipper){
					
                    nodeQueueInter.emplace_back(currTree,0,0,randNum);
					nodeQueueInter.at(0).setupRoot(indicesHolder[currTree], zipper);
					nodeQueueInter.at(0).processNode();
                    /*std::cout<<"PRINTING nodeinter queue\n";
                    fflush(stdout);
                     nodeQueueInter.at(0).nodeIndices.print();
                    */
                     if(nodeQueueInter.at(0).isLeafNode()){
						makeRootALeafInter();
					}else{
						copyProcessedRootToBinInter();
						createRootChildNodesInter();
					}
				}

				inline void makeRootALeafInter(){
				    std::cout<<"ENTERRED makeRootALeafInter !!\n";
                fflush(stdout);    
                    bin[returnRootLocation()].setClass(nodeQueueInter.at(0).returnNodeClass());
					bin[returnRootLocation()].setDepth(0);
				}

				inline void setSharedVectors2(){
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
				inline void setSharedVectors(obsIndexAndClassVec& indicesInNode){
					indicesInNode.resetVectors();

					int numUnusedObs = fpSingleton::getSingleton().returnNumObservations();
					int randomObsID;
					int tempMoveObs;

					for(int n = 0; n < fpSingleton::getSingleton().returnNumObservations(); n++){
						randomObsID = randNum.gen(fpSingleton::getSingleton().returnNumObservations());
						indicesInNode.insertIndex(nodeIndices[randomObsID], fpSingleton::getSingleton().returnLabel(nodeIndices[randomObsID]));

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
					}
				}


				inline int returnDepthOfNode(){
					assert(!nodeQueue.empty());
					return bin[nodeQueue.back().returnParentNodeNumber()].returnDepth()+1;
				}

				
                inline int returnDepthOfNodeInter(){
				//	assert(!nodeQueueInter.empty());
					return bin[nodeQueueInter.at(0).returnParentNodeNumber()].returnDepth()+1;
				}


				inline void copyProcessedNodeToBin(){
					bin.emplace_back(nodeQueue.back().returnNodeCutValue(), returnDepthOfNode(), nodeQueue.back().returnNodeCutFeature());
				}


				inline void copyProcessedNodeToBinInter(){
					bin.emplace_back(nodeQueueInter.at(0).returnNodeCutValue(), returnDepthOfNodeInter(), nodeQueueInter.at(0).returnNodeCutFeature());
				}
				
                inline void copyProcessedRootToBin(){
					bin[returnRootLocation()].setCutValue(nodeQueue.back().returnNodeCutValue());
					bin[returnRootLocation()].setDepth(0);
					bin[returnRootLocation()].setFeatureValue(nodeQueue.back().returnNodeCutFeature());
				}

				inline void copyProcessedRootToBinInter(){
					bin[returnRootLocation()].setCutValue(nodeQueueInter.at(0).returnNodeCutValue());
					bin[returnRootLocation()].setDepth(0);
					bin[returnRootLocation()].setFeatureValue(nodeQueueInter.at(0).returnNodeCutFeature());
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
					if(nodeQueueInter.at(0).returnIsLeftNode()){
						bin[nodeQueueInter.at(0).returnParentNodeNumber()].setLeftValue(positionOfNextNode());
					}else{
						bin[nodeQueueInter.at(0).returnParentNodeNumber()].setRightValue(positionOfNextNode());
					}
				}


				inline void linkParentToLeafInter(){
					assert(nodeQueueInter.front().returnParentNodeNumber() >= fpSingleton::getSingleton().returnNumClasses());
					assert(nodeQueueInter.front().returnParentNodeNumber() <= parentNodesPosition());

					assert(nodeQueueInter.front().returnNodeClass() >= 0);
					assert(nodeQueueInter.front().returnNodeClass() < fpSingleton::getSingleton().returnNumClasses());

					if(nodeQueueInter.at(0).returnIsLeftNode()){
						bin[nodeQueueInter.at(0).returnParentNodeNumber()].setLeftValue(nodeQueueInter.at(0).returnNodeClass());
					}else{
						bin[nodeQueueInter.at(0).returnParentNodeNumber()].setRightValue(nodeQueueInter.at(0).returnNodeClass());
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
					nodeIterators nodeIts(nodeQueueInter.at(0).returnNodeIterators());
					zipperIterators<int,T> zipIts(nodeQueueInter.at(0).returnZipIterators());
					int childDepth = returnDepthOfNodeInter()+1;
					auto parentTreeNum = nodeQueueInter.at(0).exposeTreeNum();
                    nodeQueueInter.erase(nodeQueueInter.begin());
					//TODO: don't emplace_back if should be leaf node.
					nodeQueueInter.emplace_back(parentTreeNum,parentNodesPosition(), childDepth, randNum);
					nodeQueueInter.back().setupNode(nodeIts, zipIts, leftNode());
					
                    nodeQueueRight.emplace_back(parentTreeNum,parentNodesPosition(), childDepth, randNum);
					nodeQueueRight.back().setupNode(nodeIts, zipIts, rightNode());
				}


				inline void createRootChildNodesInter(){
					nodeIterators nodeIts(nodeQueueInter.at(0).returnNodeIterators());
                    zipperIterators<int,T> zipIts(nodeQueueInter.at(0).returnZipIterators());
					int childDepth = returnDepthOfNodeInter()+1;
					auto parentTreeNum = nodeQueueInter.at(0).exposeTreeNum();
                    std::cout<<"BEFORE!!!\n"; 
                    std::cout<<"Inside left createRootChildNodesInter(): \n";
                    fflush(stdout);
                    for(auto el : nodeQueueLeft)
                        el.nodeIndices.print();
                    std::cout<<"Inside rightcreateRootChildNodesInter(): \n";
                    fflush(stdout);
                    for(auto el : nodeQueueRight)
                      el.nodeIndices.print();
					//	nodeQueueInter.pop_front();
                    nodeQueueInter.erase(nodeQueueInter.begin());
					
                    processingNodeBin<T, Q> left = processingNodeBin<T, Q>(parentTreeNum,returnRootLocation(), childDepth,randNum);    
					left.setupNode(nodeIts, zipIts, leftNode());
                    processingNodeBin<T, Q> right = processingNodeBin<T, Q>(parentTreeNum,returnRootLocation(), childDepth,randNum);    
				    right.setupNode(nodeIts, zipIts, rightNode());
                    nodeQueueLeft.push_back(left);
                    nodeQueueRight.push_back(right);
                    //TODO: don't emplace_back if should be leaf node.
						//nodeQueueLeft.push_back(processingNodeBin<T, Q>(parentTreeNum,returnRootLocation(), childDepth,randNum));
						//nodeQueueLeft.back().setupNode(nodeIts, zipIts, leftNode());
						
                        //nodeQueueRight.push_back(processingNodeBin<T, Q>(parentTreeNum,returnRootLocation(), childDepth, randNum));
						//nodeQueueRight.back().setupNode(nodeIts, zipIts, rightNode());
                   
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
                    nodeQueueInter.erase(nodeQueueInter.begin());
					//nodeQueueInter.pop_front();
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
                    nodeQueueInter.at(0).processNode();
                    if (nodeQueueInter.at(0).isLeafNode()) {
						// label the processed node as a leaf.
						processLeafNodeInter();
					}
					else {
						// label the processed node as internal.
						processInternalNodeInter();
					}
				}


				inline void createBin(int numTrees, int randSeed, int depthInter){
					numOfTreesInBin = numTrees;
					randNum.initialize(randSeed);
					initializeStructures();
                    for(; currTree < numOfTreesInBin; ++currTree){
					    setSharedVectors2();
					    //setSharedVectors(indicesHolder);
                        std::cout<<currTree<<":*********************************************************\n";
                        loadFirstNodeInter();
                        //removeStructures2(); 
					    //initializeStructures3();
                    }
                    for(auto ele: nodeQueueLeft){
                        nodeQueueInter.push_back(ele);
                    }

                    for(auto ele: nodeQueueRight){
                        nodeQueueInter.push_back(ele);
                    }


                    int topNodeTreeNum=0, d = 0, depth2 = 2;
                   while(d < depth2 ){
                       std::cout<<"IN WHILE\n"; 
                       for(int currTree=0; currTree< numOfTreesInBin; ++currTree){
                            topNodeTreeNum=nodeQueueInter.front().exposeTreeNum();
                            if(topNodeTreeNum != currTree)
                               continue;
                            processNodeInter();
                            break;
                        }
                        while(!nodeQueueRight.empty()){
                            processingNodeBin<T,Q> ele = nodeQueueRight.front();
                            nodeQueueRight.erase(nodeQueueRight.begin());
                            nodeQueueInter.push_back(ele);
                        }
                        d++;
                    }
                    
                       std::cout<<"out of WHILE\n"; 
                   while(!nodeQueueInter.empty()){
                        auto ele = nodeQueueInter.front();
                        nodeQueueInter.erase(nodeQueueInter.begin());
					    nodeQueue.emplace_back(ele);
                        while(!nodeQueue.empty()){
							processNode();
                        }
                    }
                    
                
                    /*for(; currTree < numOfTreesInBin; ++currTree){
						setSharedVectors(indicesHolder);
						loadFirstNode();	
                        while(!nodeQueue.empty()){
							processNode();
						}
					}*/
                    //printBin();    
					removeStructures();
				}
					
                inline void initializeStructures2(){
                    zipper.resize(fpSingleton::getSingleton().returnNumObservations());
					nodeIndices.resize(fpSingleton::getSingleton().returnNumObservations());
					for(int i = 0; i < fpSingleton::getSingleton().returnNumObservations(); ++i){
						nodeIndices[i] =i;
					}
                }
                
                inline void initializeStructures3(){
                    zipper.resize(fpSingleton::getSingleton().returnNumObservations());
                }


				inline void initializeStructures(){
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
				
                inline void removeStructures2(){
                    std::vector<zipClassAndValue<int, T> >().swap( zipper );
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

				inline void predictBinObservation(std::vector<T>& observation, std::vector<int>& preds){
					predictBinObservation(observation,preds,identity<Q>());
				}

				////////////////////////////////

				//PredictForRF
				inline void predictBinObservation(int observationNum,std::vector<int>& preds, identity<int> ){
					std::vector<int> currNode(numOfTreesInBin);
					int numberNotInLeaf;
					int featureNum;
					T featureVal;
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
								featureVal = fpSingleton::getSingleton().returnTestFeatureVal(featureNum,observationNum);
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
						__builtin_prefetch(&bin[currNode[q]], 0, 3);
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
#pragma omp atomic update
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
#pragma omp atomic update
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
#pragma omp atomic update
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
