#ifndef binnedBase_h
#define binnedBase_h

#include "../../baseFunctions/fpForestBase.h"
#include "../../baseFunctions/fpBaseNode.h"
#include "../../serialization/binLayout.h"

#include <vector>
#include <stdio.h>
#include <ctime>
#include <chrono>
#include <cstdlib>
#include "binStruct.h"
#include <random>
#include <numeric>

#include <iostream>
#include <algorithm>
#include <map>
#include <chrono>

std::string global_fname = "newfilebfs";
//MemoryMapped mmappedObj(global_fname.c_str(), 0);
std::string global_fname_csv = "profile_mnist_bfs2.csv";
std::fstream fout;
std::string global_str;
#define NUM_FILES 10000

std::vector<MemoryMapped> mmappedObj_vec(NUM_FILES);
namespace fp {

	template <typename T, typename Q>
		class binnedBase : public fpForestBase<T>
	{
		protected:
			std::vector<binStruct<T, Q> > bins;
			int numBins;
			std::map<std::pair<int, int>, double> pairMat;
			std::vector<int> binSizes;
			std::vector<int> binSeeds;
			inline void checkParameters(){
				if(fpSingleton::getSingleton().returnNumTreeBins() > fpSingleton::getSingleton().returnNumTrees()){
					fpSingleton::getSingleton().setNumTreeBins(fpSingleton::getSingleton().returnNumTrees());
				}

				if(fpSingleton::getSingleton().returnNumTreeBins() < 1){
					fpSingleton::getSingleton().setNumTreeBins(fpSingleton::getSingleton().returnNumThreads());
				}
			}

		public:
            fpBaseNode<T, Q> *data;
            std::vector<fpBaseNode<T, Q> *> data_vector; 
			~binnedBase(){}
			binnedBase(){
				checkParameters();
				numBins =  fpSingleton::getSingleton().returnNumTreeBins();
                generateSeedsForBins();
            }
            
			inline void generateSeedsForBins(){
				binSeeds.resize(numBins);
				for(int i = 0; i < numBins; ++i){
					binSeeds[i] = fpSingleton::getSingleton().genRandom(std::numeric_limits<int>::max());
                }
			}

			inline void printForestType(){
				std::cout << "This is a binned forest.\n";
			}

			inline void changeForestSize(){
				bins.reserve(numBins);
			}

			inline void calcBinSizes(){
				int minBinSize = fpSingleton::getSingleton().returnNumTrees()/numBins;
                binSizes.resize(numBins,minBinSize);
                int remainingTreesToBin = fpSingleton::getSingleton().returnNumTrees()-minBinSize*numBins;
				while(remainingTreesToBin != 0){
					++binSizes[--remainingTreesToBin];
				}
			}


			inline void growBins(){
                int depth_intertwined = 1;
				calcBinSizes();
				fpDisplayProgress printProgress;
				bins.resize(numBins);
//#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
				for(int j = 0; j < numBins; ++j){
					bins[j].createBin(binSizes[j], binSeeds[j], depth_intertwined);
                    
                    BinLayout<T, Q> binss(bins[j], global_fname) ;
                    //TODO: set flag for layout
                    binss.BINStatLayout2(3);
				    bins[j].setBin(binss.getFinalBin());
                    //TODO: set flag to write to file
                    auto start = std::chrono::steady_clock::now();
                    binss.writeToFile();
                        auto end = std::chrono::steady_clock::now();
                        std::cout<<"Time to serialize/write to file: " <<std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()<<" nanoseconds.\n";
                }
            }

			inline float reportOOB(){
				return -1;
			}
			inline std::map<std::string, int> calcBinStats(){
				int maxDepth=0;
				int totalLeafNodes=0;
				int totalLeafDepth=0;

				int tempMaxDepth;
				for(int i = 0; i < numBins; ++i){
					tempMaxDepth = bins[i].returnMaxDepth();
					maxDepth = ((maxDepth < tempMaxDepth) ? tempMaxDepth : maxDepth);

					totalLeafNodes += bins[i].returnNumLeafNodes();
					totalLeafDepth += bins[i].returnLeafDepthSum();
				}

				std::map<std::string, int> binStats;
				binStats["maxDepth"] = maxDepth;
				binStats["totalLeafDepth"] = totalLeafDepth;
				binStats["totalLeafNodes"] = totalLeafNodes;
				return binStats;
			}

			inline void binStats(){
				std::map<std::string, int> binStats = calcBinStats();

				std::cout << "max depth: " << binStats["maxDepth"] << "\n";
				std::cout << "avg leaf node depth: " << float(binStats["totalLeafDepth"])/float(binStats["totalLeafNodes"]) << "\n";
				std::cout << "num leaf nodes: " << binStats["totalLeafNodes"] << "\n";
			}

			void printBin0(){
				bins[0].printBin();
			}

			inline void growForest(){
				//	checkParameters();
				//TODO: change this so forest isn't grown dynamically.
				//changeForestSize();
				growBins();
				binStats();
			}



			inline int predictClass(int observationNumber, bool fromFile = true, std::string filename = global_fname){				
                std::vector<int> predictions(fpSingleton::getSingleton().returnNumClasses(),0);
                std::fstream f;
                f.open("rand_file.bin");
                int i;
                for(int j = 0; j < 10000000; j++)
                                    {
                                                            f.read((char*)&i, sizeof(i));
                                                                            }
                f.close();
//#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
                for(int k = 0; k < numBins; ++k){
                    if(!fromFile)
					    bins[k].predictBinObservation(observationNumber, predictions);
				    else{
                        binStruct<T, Q> temp = binStruct<T, Q>(binSizes[k]);
                        auto start = std::chrono::steady_clock::now();
                        temp.predictBinObservation(data_vector, observationNumber, predictions);
                        auto end = std::chrono::steady_clock::now();
                        std::cout<<"Elapsed time: " <<std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()<<" nanoseconds.\n";
                        fout<<std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<" ";
                    }
                }

				//assert(std::accumulate(predictions.begin(), predictions.end(),0) == fpSingleton::getSingleton().returnNumTrees());

				int bestClass = 0;
				for(int j = 1; j < fpSingleton::getSingleton().returnNumClasses(); ++j){
					if(predictions[bestClass] < predictions[j]){
						bestClass = j;
					}
				}
			 	return bestClass;
			}
            


			inline int predictClass(std::vector<T>& observation){
				std::vector<int> predictions(fpSingleton::getSingleton().returnNumClasses(),0);

#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
				for(int k = 0; k < numBins; ++k){
					bins[k].predictBinObservation(observation, predictions);
				}
				int bestClass = 0;
				for(int j = 1; j < fpSingleton::getSingleton().returnNumClasses(); ++j){
					if(predictions[bestClass] < predictions[j]){
						bestClass = j;
					}
				}
				return bestClass;
			}


			inline std::vector<int> predictClassPost(std::vector<T>& observation){
				std::vector<int> predictions(fpSingleton::getSingleton().returnNumClasses(),0);

#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
				for(int k = 0; k < numBins; ++k){
					bins[k].predictBinObservation(observation, predictions);
				}
				return predictions;
			}


			inline int predictClass(const T* observation){
				/*
					 std::vector<int> predictions(fpSingleton::getSingleton().returnNumClasses(),0);

#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
for(int k = 0; k < numBins; ++k){
bins[k].predictBinObservation(observation, predictions);
}

int bestClass = 0;
for(int j = 1; j < fpSingleton::getSingleton().returnNumClasses(); ++j){
if(predictions[bestClass] < predictions[j]){
bestClass = j;
}
}
return bestClass;
*/
			return 0;
			}

inline std::map<std::pair<int, int>, double> returnPairMat(){
                                        return pairMat;
                                }

inline float testForest(){
                size_t arrlen = 0;
                deserializeMmap(arrlen);
	int numTried = 0;
	int numWrong = 0;
    fout.open(global_fname_csv, std::ios::out );
    for (int i = 0; i <fpSingleton::getSingleton().returnNumObservations();i++){
		++numTried;
		int predClass = predictClass(i);

		if(predClass != fpSingleton::getSingleton().returnTestLabel(i)){
			++numWrong;
		}
	}
    fout.close();
	std::cout << "\nnumWrong= " << numWrong << "\n";

    return (float)numWrong/(float)numTried;
}

inline void deserializeMmap(size_t &numNodes){
    std::cout<<"In deserialize Mmap!\n";
    /*for(int i=0; i<NUM_FILES; i++){
        std::cout<<global_fname + std::to_string(i) + ".bin"<<"\n";
        fflush(stdout);
        global_str = global_fname + std::to_string(i) + ".bin";
        mmappedObj_vec[i].open(global_str, 0);
        data = (fpBaseNode<T, Q>*)mmappedObj_vec[i].getData();
        data_vector.push_back(data);
    }
    numNodes = mmappedObj_vec[0].mappedSize() / sizeof(data[0]);
*/
}

};

}// namespace fp
#endif //binnedBase_h
