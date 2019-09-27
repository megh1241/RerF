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
#include "nodeStruct.cpp"

#define NUM_FILES 1000

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
            BinLayout(binStruct<T, Q> tempbins, std::string fname): binstr(tempbins){
                //TODO: initialize in fpSingleton
                filename = fname;
            }
            
            inline std::string returnFilename(){
               /* Return the file in which the forest was written to */
                return filename;
            };
            
            inline void initFinalBin(int binNum){
               
                /*  Copy the class nodes and root nodes to the finalbin */ 
                
                std::vector< fpBaseNode<T,Q> > bin = binstr.getBin();
                int numTrees = binstr.numOfTreesInBin;
                int numClasses = fpSingleton::getSingleton().returnNumClasses();  
                int numInitNodes = binstr.numOfTreesInBin + fpSingleton::getSingleton().returnNumClasses();        
                for(int i=0; i<numInitNodes; ++i){
                    finalbin.push_back(bin.at(i));
                    nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), i));
                }

                //printFinalBin();
            }
            
            inline void StatLayout();
            inline void BINStatLayout(int depthIntertwined){
                initFinalBin(0); 
                std::vector< fpBaseNode<T,Q> > bin = binstr.getBin();
                std::vector<int> treeEndPos = binstr.getTreeEndPos();
                for (int depth = 1; depth < depthIntertwined; ++depth){
                    for(auto node : bin){
                        if (node.returnDepth() == depth){
                            finalbin.push_back(node);   
                            nodeNewIdx.insert(std::pair<int, int>(node.getID(), finalbin.size()-1));
                        }
                    }
                }
            }
            
            inline void BINStatLayout2(int depthIntertwined){
                std::vector< fpBaseNode<T,Q> > bin = binstr.getBin();
                std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();

                int numNodesToProc = std::pow(2, depthIntertwined) - 2; 
                int d;
                auto numClasses = fpSingleton::getSingleton().returnNumClasses();
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
                        
                        //binQ.push_back(bin[ele.returnLeftNodeID()]); 
                        //binQ.push_back(bin[ele.returnRightNodeID()]); 
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
                //printFinalBin();
            }
            
            inline void BFSLayout(){
                std::vector< fpBaseNode<T,Q> > bin = binstr.getBin();
                std::map<int, int> nodeTreeMap = binstr.getNodeTreeMap();
                std::deque<fpBaseNode<T, Q>> binST;
                finalbin.clear();
                nodeNewIdx.clear();
                int numClasses = fpSingleton::getSingleton().returnNumClasses();
                for(int i = 0; i < numClasses; ++i){
                    finalbin.push_back(bin[i]);
                    nodeNewIdx.insert(std::pair<int, int>(bin[i].getID(), finalbin.size()-1));
                }

                for(int i = 0; i < binstr.numOfTreesInBin; ++i){
                    binST.push_back(bin[i+numClasses]);
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
                        
                        else{
                            binST.push_back(bin[ele.returnLeftNodeID()]); 
                            binST.push_back(bin[ele.returnRightNodeID()]); 
                        }
                    }
                }

                //printFinalBin();
                auto siz = finalbin.size();
                for (auto i=0; i<siz; i++){
                    finalbin[i].setLeftValue(nodeNewIdx[bin[finalbin[i].returnLeftNodeID()].getID()]);
                    finalbin[i].setRightValue(nodeNewIdx[bin[finalbin[i].returnRightNodeID()].getID()]);
                    //finalbin[i].setDepth(bin[nodeNewIdx[i]].returnDepth());
                }
printFinalBin();
            }

            inline std::vector<fpBaseNode<T, Q>> getFinalBin(){
                return finalbin;
            }

            inline void BINStatClassLayout(int depthIntertwined);
            
            inline void writeToFile(){
                std::ofstream f;
                for(int j = 0; j < NUM_FILES; j++){
                    f.open((filename + std::to_string(j) + ".bin").c_str(), std::ios::out|std::ios::binary);
                    for(auto i: finalbin)
                        f.write((char*)&i, sizeof(i));
                    f.close();
                }
                f.open("rand_file.bin");
                
                for(int j = 0; j < 10000000; j++)
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
