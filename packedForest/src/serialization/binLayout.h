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
        std::vector<binStruct<T, Q> > bins;
        std::vector<fpBaseNode<>;
        public:
            BinLayout(std::vector<binStruct<T, Q> > tempbins): bins(tempbins);
            inline void BFSLayout();
            inline void StatLayout();
            inline void BINStatLayout(int depthIntertwined);
            inline void BINStatClassLayout(int depthIntertwined);
            inline void serializeToDisk();
            inline void deserializeFromDisk();    
    };

#endif //binLayout_h
