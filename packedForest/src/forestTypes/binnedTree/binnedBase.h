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
#include <functional>

std::string global_fname = "/data4/bfsars";
std::fstream fout;
std::string global_str;
std::vector<int> treeRootPos;
std::vector<int> blocks;
std::vector<double>etime;
std::vector<int> sizesbin;
std::fstream f_time;
#define NUM_FILES 900

std::vector<MemoryMapped> mmappedObj_vec(NUM_FILES);
MemoryMapped mmappedObj;
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
				calcBinSizes();
				fpDisplayProgress printProgress;
				bins.resize(numBins);
				std::vector<BinLayout<T, Q>> binvector;
				std::string layout_str = fpSingleton::getSingleton().returnLayout();
				int depth = fpSingleton::getSingleton().returnDepthIntertwined();
#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
				for(int j = 0; j < numBins; ++j){
					bins[j].createBin(binSizes[j], binSeeds[j], 1);
		    			
					BinLayout<T, Q> bins_serialize(bins[j], global_fname) ;
                    			bins[j].setBin(bins_serialize.getFinalBin());
                    			treeRootPos = bins_serialize.treeRootPos;
                        		
					if(layout_str.compare("bfs") == 0)
                        			bins_serialize.BFSLayout();
					else if(layout_str.compare("stat") == 0)
                        			bins_serialize.statLayout();
					else if(layout_str.compare("binbfs") == 0)
                        			bins_serialize.BINBFSLayout(depth);
					else if(layout_str.compare("binstat") == 0)
                        			bins_serialize.BINStatLayout(depth);
                        		else
                        			bins_serialize.BINStatClassLayout(depth);

                    			auto start = std::chrono::steady_clock::now();
					#pragma omp critical
					{
						binvector.push_back(bins_serialize);
				        	sizesbin.push_back((int)bins_serialize.finalbin.size());	
					}
					auto end = std::chrono::steady_clock::now();
                    			std::cout<<"Time to serialize/write to file: " <<std::chrono::duration_cast<std::chrono::seconds>(end - start).count()<<" nanoseconds.\n";
                		}

				for(auto single_bin: binvector)
					single_bin.writeToFile(treeRootPos);
				std::fstream fsiz;
				fsiz.open("/data4/binstart.txt", std::ios::out);
				for(auto i: sizesbin)
					fsiz<<i<<"\n";
				fsiz.close();
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
				int j, tmp_val, uniqueCount=0;
				int treesPerBin;
				std::fstream fi;
				fi.open("/data4/rand_file.txt");
				for(int i=0; i<20000; ++i)
					fi>>j;	
				for(int i=0; i<20000; ++i)
					fi>>j;	
				for(int i=0; i<20000; ++i)
					fi>>j;	
				fi.close();	
			
				fi.open("/data4/binstart.txt");
				sizesbin.clear();
				int sumsum = 0;
				for(int i=0; i<fpSingleton::getSingleton().returnNumThreads(); ++i){
					fi>>tmp_val;
					sizesbin.push_back(sumsum);
					sumsum+=tmp_val;
				}
				fi.close();
				std::vector<int> predictions(fpSingleton::getSingleton().returnNumClasses(),0);
                        	treesPerBin = fpSingleton::getSingleton().returnNumTrees() / fpSingleton::getSingleton().returnNumThreads();
				
				binStruct<T, Q> temp = binStruct<T, Q>(treesPerBin);
				global_str = global_fname + std::to_string(observationNumber%NUM_FILES) + ".bin";
                       		mmappedObj.open(global_str, 0); 
                        	data = (fpBaseNode<T, Q>*)mmappedObj.getData();
				auto start = std::chrono::steady_clock::now();
				treeRootPos.clear();
#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
                		for(int k = 0; k < numBins; ++k){
                			fpBaseNode<T, Q> * mmapped_file_pos = data + sizesbin[k];	
					if(!fromFile)
					    bins[k].predictBinObservation(observationNumber, predictions);
		    			else{
						temp.predictBinObservation(uniqueCount, treeRootPos, mmapped_file_pos, observationNumber, predictions);
                        			//blocks.push_back(uniqueCount);
                    			}
                		}
				auto end = std::chrono::steady_clock::now();
				mmappedObj.close();
				
				etime.push_back(std::chrono::duration<double, std::milli>(end - start).count());

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
				std::fstream fi; 
				fi.open("/data4/randfile.txt", std::ios::out);	
				for(int i=0; i<10000; ++i)
					fi<<(i+1)%6;
				fi.close();
			
				int numTried = 0;
				int numWrong = 0;
    				for (int i = 0; i <fpSingleton::getSingleton().returnNumObservations();i++){
					++numTried;
					int predClass = predictClass(i);

					if(predClass != fpSingleton::getSingleton().returnTestLabel(i)){
						++numWrong;
					}
				}
				f_time.open("elapsed_time_parallel.csv", std::ios::out);
				for(auto t: etime)
					f_time<<t<<",";
				f_time.close();
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
