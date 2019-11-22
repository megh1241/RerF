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

std::string global_fname = "/data3/bfs";
std::fstream fout;
std::string global_str;
std::vector<int> treeRootPos;
std::vector<int> blocks;
#define NUM_FILES 10

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
				std::fstream ff4;
				ff4.open("/data/rand_file.bin", std::ios::out);
                		for(int j = 0; j < 200000; j++)
                			ff4<<j;
				ff4.close();
				
                		int i;
				int depth_intertwined = 1;
				calcBinSizes();
				fpDisplayProgress printProgress;
				numBins = 2;
				fpSingleton::getSingleton().setNumTreeBins(2);
				bins.resize(numBins);
		    		std::cout<<"before create bin\n";
		    		fflush(stdout);
#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
				for(int j = 0; j < numBins; ++j){
					bins[j].createBin(binSizes[j], binSeeds[j], 1);
		   		 	/*std::cout<<"before create bin2\n";
                    			//TODO: set flag for layout
					BinLayout<T, Q> binss(bins[j], global_fname) ;
		    			binss.BINBFSLayout(3);
		    			//binss.BINStatLayout(2);
		    			//binss.BINStatClassLayout(1);
                    			//binss.statLayout();
                    			binss.BFSLayout();
                    			bins[j].setBin(binss.getFinalBin());
                    			treeRootPos = binss.treeRootPos;
                    			//TODO: set flag to write to file
                    			auto start = std::chrono::steady_clock::now();
                    			binss.writeToFile();
                    			auto end = std::chrono::steady_clock::now();
                    			std::cout<<"Time to serialize/write to file: " <<std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()<<" nanoseconds.\n";
                		*/
				}
				/*std::cout<<"PRINTING BINSS!!!!!\n";
				for(int j=0; j<numBins; ++j)
				{
					std::cout<<"***************************************************\n";
					for(auto k: bins[j].getBin())
					{
						k.printNode();
					}
					std::cout<<"***************************************************\n";
				}*/
				//reconcile bins
				int new_num_trees=0;
				int count=0;
				std::vector< fpBaseNodeStat<T,Q> > bin1;
				int numClasses = fpSingleton::getSingleton().returnNumClasses();
				for(int j=0; j<numBins; ++j)
				{
					new_num_trees += bins[j].getNumTrees();
					std::cout<<"Num trees: "<<new_num_trees<<"\n";
					std::vector< fpBaseNodeStat<T,Q> > bin2 = bins[j].getBin();
					if(j>=1){
						typename std::vector<fpBaseNodeStat<T, Q>>::const_iterator first = bin2.begin() + numClasses;
						typename std::vector<fpBaseNodeStat<T, Q>>::const_iterator last = bin2.end();
						std::vector<fpBaseNodeStat<T,Q>> binTemp(first, last);
						std::vector<fpBaseNodeStat<T,Q>> binTemp2;
						int offset = bin1.size();
						std::cout<<"checkpont 2\n";
						for(auto i: binTemp){
							std::cout<<"before\n";
							i.printNode();
							i.setID(count);
							count++;
							if (i.returnLeftNodeID() >= numClasses)
								i.setLeftValue(i.returnLeftNodeID() + offset - numClasses -1);
							if (i.returnRightNodeID() >= numClasses)
								i.setRightValue(i.returnRightNodeID() + offset - numClasses-1);
						
							std::cout<<"after\n";
							i.printNode();
							binTemp2.push_back(i);
						}
						bin1.insert(bin1.end(), binTemp2.begin(), binTemp2.end());
						binTemp2.clear();
						binTemp.clear();	
					}
					else{
						count = bin2.size();
						for(auto i: bin2)
							i.printNode();
						bin1= bin2;
					}
				}
				/*
				for(auto j: bin1)
					j.printNode();
				*/
				//binStruct<T, Q> newStructBin(new_num_trees, bin1);
				binStruct<T, Q> newStructBin(new_num_trees, bin1);
				BinLayout<T, Q> binss(newStructBin, global_fname) ;
                    		binss.BFSLayout();
                    		treeRootPos = binss.treeRootPos;
				binss.writeToFile();
				fpSingleton::getSingleton().setNumTreeBins(1);

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
                		int uniqueCount;
				std::fstream f;
                		
				f.open("rand_file.bin");
                		int i;
                		for(int j = 0; j < 200000; j++)
                			f.read((char*)&i, sizeof(i));
				f.close(); 
                		//#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
                		for(int k = 0; k < numBins; ++k){
                    			if(!fromFile)
					    bins[k].predictBinObservation(observationNumber, predictions);
		    			else{
                        			binStruct<T, Q> temp = binStruct<T, Q>(12);
                        			global_str = global_fname + std::to_string(observationNumber%NUM_FILES) + ".bin";
                        			mmappedObj_vec[observationNumber%NUM_FILES].open(global_str, 0);
                        			data = (fpBaseNode<T, Q>*)mmappedObj_vec[observationNumber%NUM_FILES].getData();
                        			temp.predictBinObservation(uniqueCount, treeRootPos, data, observationNumber, predictions);
                        			blocks.push_back(uniqueCount);
                        			std::cout<<"Observation number: "<<observationNumber<<"\n";
                        			fflush(stdout);
                    			}
                		}

				//assert(std::accumulate(predictions.begin(), predictions.end(),0) == fpSingleton::getSingleton().returnNumTrees());

				int bestClass = 0;
				for(int j = 1; j < fpSingleton::getSingleton().returnNumClasses(); ++j){
					if(predictions[bestClass] < predictions[j]){
						bestClass = j;
					}
				}
                		mmappedObj_vec[observationNumber%NUM_FILES].close();
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
    	int tot = fpSingleton::getSingleton().returnNumObservations();
    	for (int i = 0; i <fpSingleton::getSingleton().returnNumObservations();i++){
		++numTried;
		int predClass = predictClass(i);

		if(predClass != fpSingleton::getSingleton().returnTestLabel(i)){
			++numWrong;
		}
	}
    	fout.open("bfsblockscifar.csv", std::ios::out);
    	for(auto i: blocks)
        	fout<<i<<",";
    	fout.close();
	std::cout << "\nnumWrong= " << numWrong << "\n";

    	return (float)numWrong/(float)numTried;
}

inline void deserializeMmap(size_t &numNodes){
    // open file    
    std::ifstream inputFile("rootpos.txt");
    // test file open   
    if (inputFile) {        
            double value;
    }
}

};

}// namespace fp
#endif //binnedBase_
