#ifndef binLayout_h
#define binLayout_h

#include <vector>
#include <assert.h>
#include "../baseFunctions/fpBaseNode.h"
#include "../forestTypes/binnedTree/binnedBase.h"
#include "../forestTypes/binnedTree/binStruct.h"

namespace fp{

template<typename T, typename Q>
    class BinLayout
    {
        std::vector<binStruct<T, Q> > binstr;
        std::vector<fpBaseNode<T, Q>> finalbin;
        public:
            BinLayout(std::vector<binStruct<T, Q> > tempbins): binstr(tempbins){};
            
            inline void initFinalBin(int binNum){
               
                /*  Copy the class nodes and root nodes to the finalbin */ 
                
                std::vector< fpBaseNode<T,Q> > bin = binstr[binNum].getBin();
                std::vector<int> treeEndPos = binstr[binNum].getTreeEndPos();
                int currPos = 0, firstPos = treeEndPos[binNum];
               
                for(auto node: bin)
                    if(currPos++ <= firstPos)
                        finalbin.push_back(node);
                    else
                        break;
                printFinalBin();
                printTreePos();
            }
            
            inline void BFSLayout();
            inline void StatLayout();
            inline void BINStatLayout(int depthIntertwined){
                initFinalBin(0); 
                std::vector< fpBaseNode<T,Q> > bin = binstr[0].getBin();
                std::vector<int> treeEndPos = binstr[0].getTreeEndPos();
                for (int depth = 1; depth < depthIntertwined; ++depth){
                    for(auto node : bin){
                        if (node.returnDepth() == depth)
                            finalbin.push_back(node);    
                    }
                }
            }
            
            
            inline void printFinalBin(){
                std::cout<<"final bin: \n";
                for (auto i: finalbin)
                    i.printNode();
            }
            
            inline void printTreePos(){
                std::vector<int> treeEndPos = binstr[0].getTreeEndPos();
                std::cout<<"Position Vector\n";
                for(auto i : treeEndPos)
                    std::cout<<i<<"  ";
                std::cout<<"\n";
            }
            
            inline void BINStatClassLayout(int depthIntertwined);
            inline void serializeToDisk();
            inline void deserializeFromDisk();    
    };
}
#endif //binLayout_h
