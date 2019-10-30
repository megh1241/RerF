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

#define NUM_FILES 10

namespace fp{

template<typename T, typename Q>
    class BinLayout
    {
        binStruct<T, Q> binstr;
        std::vector<fpBaseNode<T, Q>> finalbin;
        std::deque<fpBaseNode<T, Q>> binQ;
        std::map<int, int> nodeNewIdx;
        std::string filename;
        std::vector<std::string> filename_vec;
        public:
            std::vector<int> treeRootPos;
            BinLayout(binStruct<T, Q> tempbins, std::string fname): binstr(tempbins){
                //TODO: initialize in fpSingleton
                filename = fname;
            }
            
            inline std::string returnFilename(){
               /* Return the file in which the forest was written to */
                return filename;
            };
            
            
            
            inline void BINBFSLayout(int depthIntertwined){
                std::vector< fpBaseNode<T,Q> > bin = binstr.getBin();
                std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();

                int numNodesToProc = std::pow(2, depthIntertwined) - 2; 
                int d;
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

                for(auto i = 0; i < binstr.numOfTreesInBin; ++i)
                    binQ.push_back(bin[i+numClasses]);

                // Intertwined levels
                int currLevel = 0; 
                    //if(nodeTreeMap[binQ.front().getID()] != i)
                      //  continue;
                while(currLevel <= numNodesToProc*binstr.numOfTreesInBin) {
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
                            binQ.push_back(bin[ele.returnLeftNodeID()]); 
                            binQ.push_back(bin[ele.returnRightNodeID()]); 
                        }
                        
                }

                // STAT per (sub)tree layout 
                
                while(!binQ.empty()){
                    std::deque<fpBaseNode<T, Q>> binST;
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
                auto siz = finalbin.size();
                for (auto i=0; i<siz; i++){
                    finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                    finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                }

            }
            
            inline void BINStatLayout(int depthIntertwined){
                std::vector< fpBaseNode<T,Q> > bin = binstr.getBin();
                std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();

                int numNodesToProc = std::pow(2, depthIntertwined) - 2; 
                int d;
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

                for(auto i = 0; i < binstr.numOfTreesInBin; ++i)
                    binQ.push_back(bin[i+numClasses]);

                // Intertwined levels
                int currLevel = 0; 
                    //if(nodeTreeMap[binQ.front().getID()] != i)
                      //  continue;
                while(currLevel <= numNodesToProc*binstr.numOfTreesInBin) {
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
                            binQ.push_back(bin[ele.returnLeftNodeID()]); 
                            binQ.push_back(bin[ele.returnRightNodeID()]); 
                        }
                        
                }

                // STAT per (sub)tree layout 
                
                while(!binQ.empty()){
                    std::deque<fpBaseNode<T, Q>> binST;
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
                auto siz = finalbin.size();
                for (auto i=0; i<siz; i++){
                    finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                    finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                }

            }
            

            inline void BINStatClassLayout(int depthIntertwined){
                int total_tree_card = 0;
		int num_classes_in_subtree = 0;
		std::map<int, int> map_node_to_subtree;
		std::map<int, int> map_subtree_to_class;
		std::map<int, int> nodeCardinalityMap = binstr.getNodeCardinalityMap();
		std::vector< fpBaseNode<T,Q> > bin = binstr.getBin();
                std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();

                int numNodesToProc = std::pow(2, depthIntertwined) - 2; 
                int d;
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

                for(auto i = 0; i < binstr.numOfTreesInBin; ++i)
                    binQ.push_back(bin[i+numClasses]);

                // Intertwined levels
                int currLevel = 0; 
                    //if(nodeTreeMap[binQ.front().getID()] != i)
                      //  continue;
                while(currLevel <= numNodesToProc*binstr.numOfTreesInBin) {
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
                            binQ.push_back(bin[ele.returnLeftNodeID()]); 
                            binQ.push_back(bin[ele.returnRightNodeID()]); 
                        }
                        
                }

                // STAT per (sub)tree layout 
                int stno = -1; 
                while(!binQ.empty()){
                    std::deque<fpBaseNode<T, Q>> binST;
                    auto ele = binQ.front();
                    binQ.pop_front();
                    binST.push_back(ele);
		    total_tree_card = 0;
		    num_classes_in_subtree=0;
		    int card[1000] = {0};
		    stno++;
                    while(!binST.empty()){
			auto ele = binST.front();
			map_node_to_subtree[ele.getID()] = stno;
			//if ele is a leaf node, then check the class and cardinality
			if(!ele.isInternalNode()){
			    total_tree_card += nodeCardinalityMap[ele.getID()];
			    if(card[ele.returnClass()] == 0)
				num_classes_in_subtree++;
			    card[ele.returnClass()] += nodeCardinalityMap[ele.getID()];
			}
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
		    //compute max class
		    int max = -1;
		    int subtree_class = -1;
		    double eps = 1 / (double)num_classes_in_subtree;
		    for(int i=0; i<numClasses; ++i){
		    	if(card[i] > max && ((double)(card[i]) / (double)(total_tree_card) > eps)){
				subtree_class = card[i];
			}
		    }
		    map_subtree_to_class[stno] = subtree_class;
                }
                auto siz = finalbin.size();
                for (auto i=0; i<siz; i++){
                    finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                    finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                }
            }
            
            

            inline void BFSLayout(){
                std::vector< fpBaseNode<T,Q> > bin = binstr.getBin();
                int numClasses = fpSingleton::getSingleton().returnNumClasses();
                std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
                std::deque<fpBaseNode<T, Q>> binST;
                finalbin.clear();
                nodeNewIdx.clear();
                for(int i = 0; i < numClasses; ++i){
                    finalbin.push_back(bin[i]);
                    nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
                }

                int firstNodeInTree = 1;
                for(int i = 0; i < binstr.numOfTreesInBin; ++i){
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

                auto siz = finalbin.size();
                for (auto i=numClasses; i<siz; i++){
                    finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                    finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                    finalbin[i].setDepth(bin[nodeNewIdx[i]].returnDepth());
                }
            }

            inline std::vector<fpBaseNode<T, Q>> getFinalBin(){
                return finalbin;
            }

            inline void statLayout(){
                std::vector< fpBaseNode<T,Q> > bin = binstr.getBin();
		std::map<int, int> nodeCardinalityMap = binstr.getNodeCardinalityMap();
                int numClasses = fpSingleton::getSingleton().returnNumClasses();
                std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
                std::deque<fpBaseNode<T, Q>> binST;
                finalbin.clear();
                nodeNewIdx.clear();
                for(int i = 0; i < numClasses; ++i){
                    finalbin.push_back(bin[i]);
                    nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
                }

                int firstNodeInTree = 1;
                for(int i = 0; i < binstr.numOfTreesInBin; ++i){
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
			    if(nodeCardinalityMap[bin[ele.returnLeftNodeID()].getID()] > nodeCardinalityMap[bin[ele.returnRightNodeID()].getID()]){ 
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

                auto siz = finalbin.size();
                for (auto i=numClasses; i<siz; i++){
                    finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                    finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                    finalbin[i].setDepth(bin[nodeNewIdx[i]].returnDepth());
                }
            }

            
            inline void writeToFile(){
                std::ofstream f;
                for(int j = 0; j < NUM_FILES; j++){
                    f.open((filename + std::to_string(j) + ".bin").c_str(), std::ios::out|std::ios::binary);
                    for(auto i: finalbin)
                        f.write((char*)&i, sizeof(i));
                    f.close();
                }
                f.open("rand_file.bin");
                
                for(int j = 0; j < 10000; j++)
                {
                    f.write((char*)&j, sizeof(j));
                } 
                f.close();
                
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
