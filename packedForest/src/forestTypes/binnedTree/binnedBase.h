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

std::string global_fname = "/data4/bfsars";
std::fstream fout;
std::string global_str;
std::vector<int> treeRootPos;
std::vector<int> blocks;
std::vector<double>etime;
std::vector<int> sizesbin;
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
				int depth_intertwined = 1;
				int threadnum;
				calcBinSizes();
				fpDisplayProgress printProgress;
				bins.resize(numBins);
				std::vector<BinLayout<T, Q>> binvector;

#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
				for(int j = 0; j < numBins; ++j){
					threadnum = omp_get_thread_num();
					bins[j].createBin(binSizes[j], binSeeds[j], 1);
		   		 	std::cout<<"before create bin2\n";
		    			BinLayout<T, Q> binss(bins[j], global_fname) ;
                    			//TODO: set flag for layout
		    			//binss.BINBFSLayout(1);
		    			binss.BINStatLayout(1);
		    			//binss.BINStatClassLayout(1);
                    			//binss.statLayout();
                    			//binss.BFSLayout();
                    			bins[j].setBin(binss.getFinalBin());
                    			treeRootPos = binss.treeRootPos;
                    
                    			//TODO: set flag to write to file
                    			auto start = std::chrono::steady_clock::now();
                    			//binss.writeToFile(treeRootPos);
					#pragma omp critical
					{
						binvector.push_back(binss);
				        	sizesbin.push_back((int)binss.finalbin.size());	
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


			inline void repackForest(std::string filename_bin){
				std::fstream f;
				f.open(filename_bin, std::ios::in);
				fpBaseNodeStat<T, Q> node_to_read;
				std::vector<fpBaseNodeStat<T, Q>> bin_vector_temp;
				while(!f.eof())
				{
					f>>node_to_read;
					bin_vector_temp.push_back(node_to_read);

				}
                        	binStruct<T, Q> temp = binStruct<T, Q>(128, bin_vector_temp);
                                BinLayout<T, Q> binss(temp, "/data4/newdata") ;
                                        //TODO: set flag for layout
                                        //binss.BINBFSLayout(1);
                                        binss.BINStatClassLayout(1);
                                        //binss.BINStatClassLayout(1);
                                        //binss.statLayout();
                                        //binss.BFSLayout();

                                        //TODO: set flag to write to file
                                        auto start = std::chrono::steady_clock::now();
                                        binss.writeToFile();
                                        auto end = std::chrono::steady_clock::now();
                                        std::cout<<"Time to serialize/write to file: " <<std::chrono::duration_cast<std::chrono::seconds>(end - start).count()<<" nanoseconds.\n";
			}


			inline int predictClass(int observationNumber, bool fromFile = true, std::string filename = global_fname){				
				int j, tmp_val;
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
				std::cout<<"*************************************\n";
				for(int i=0; i<fpSingleton::getSingleton().returnNumThreads(); ++i){
					fi>>tmp_val;
					std::cout<<"tmp_val: "<<tmp_val<<"\n";
					std::cout<<"sumsum: "<<sumsum<<"\n";
					sizesbin.push_back(sumsum);
					sumsum+=tmp_val;
				}
				std::cout<<"*************************************\n";
				fi.close();
				std::vector<int> predictions(fpSingleton::getSingleton().returnNumClasses(),0);
                		int uniqueCount;
                        	binStruct<T, Q> temp = binStruct<T, Q>(16);
                        	//global_str = global_fname + std::to_string(observationNumber%NUM_FILES) + ".bin";
                        	//mmappedObj.open(global_str, 0); 
				int threadnum;
				global_str = global_fname + std::to_string(observationNumber%NUM_FILES) + ".bin";
                       		mmappedObj.open(global_str, 0); 
                        	data = (fpBaseNode<T, Q>*)mmappedObj.getData();
                		//#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
                		//for(int k = 0; k < numBins; ++k){
                		for(int k = 0; k < 1; ++k){
                    			 //threadnum = omp_get_thread_num();
                    			 threadnum = k;
					 std::cout<<"threadnum: "<<threadnum<<"\n";
					if(!fromFile)
					    bins[k].predictBinObservation(observationNumber, predictions);
		    			else{
						global_str = global_fname + std::to_string(observationNumber%NUM_FILES) + ".bin";
                        			//#pragma omp critical
                        		//	mmappedObj.open(global_str, 0); 
                        	//		temp.predictBinObservation(uniqueCount, treeRootPos, data + sizesbin[k], observationNumber, predictions, etime);
                        			temp.predictBinObservation(uniqueCount, treeRootPos, data, observationNumber, predictions, etime);
                        			//blocks.push_back(uniqueCount);
                        			//#pragma omp critical
						//mmappedObj.close();
                    			}
                		}
				mmappedObj.close();

				//assert(std::accumulate(predictions.begin(), predictions.end(),0) == fpSingleton::getSingleton().returnNumTrees());

				int bestClass = 0;
				for(int j = 1; j < fpSingleton::getSingleton().returnNumClasses(); ++j){
					if(predictions[bestClass] < predictions[j]){
						bestClass = j;
					}
				}
                		//mmappedObj_vec[observationNumber%NUM_FILES].close();
                		//mmappedObj.close();
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
			//	mmappedObj.open("/data4/bfsars5366.bin", 0);
                        //	data = (fpBaseNode<T, Q>*)mmappedObj.getData();
			
				std::fstream fi; 
				fi.open("/data4/randfile.txt", std::ios::out);	
				for(int i=0; i<10000; ++i)
					fi<<(i+1)%6;
				fi.close();
				size_t arrlen = 0;
				//deserializeMmap(arrlen);
				int numTried = 0;
				int numWrong = 0;
				//repackForest("/data4/bfsars0.bin");
    				int tot = fpSingleton::getSingleton().returnNumObservations();
                       		//mmappedObj.open(global_str, 0); 
                        	//data = (fpBaseNode<T, Q>*)mmappedObj.getData();
    				for (int i = 0; i <fpSingleton::getSingleton().returnNumObservations();i++){
    				//for (int i = 0; i < 1; i++){
					++numTried;
					int predClass = predictClass(i);

					if(predClass != fpSingleton::getSingleton().returnTestLabel(i)){
						++numWrong;
					}
				}
				//mmappedObj.close();
    				fout.open("blocks_22threads.csv", std::ios::out);
    				for(auto i: blocks)
        				fout<<i<<",";
    				fout.close();
    				fout.open("binstat_time_22threads.csv", std::ios::out);
    				for(auto i: etime)
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
