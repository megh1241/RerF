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
std::fstream fblock;
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

			BinLayout<T, Q> mergeBinLayoutVecs(std::vector<BinLayout<T, Q>>layoutVec){
				int count = 0, mainsize = 0, offset = 0;
				int numClasses = fpSingleton::getSingleton().returnNumClasses();
				std::vector<fpBaseNodeStat<T, Q>> finalBinVec;
				for(auto binl : layoutVec){
					if(count == 0){
						finalBinVec = binl.getFinalBin();
						mainsize = finalBinVec.size();
					}
					else{
						offset = mainsize - numClasses;
						std::vector<fpBaseNodeStat<T, Q>> currVec = binl.getFinalBin();
						for(int i=numClasses; i<currVec.size(); ++i){
							if(currVec[i].getID() >= numClasses)
								currVec[i].setID(currVec[i].getID() + offset);
							if(currVec[i].returnLeftNodeID() >= numClasses)
								currVec[i].setLeftValue( currVec[i].returnLeftNodeID() + offset);
							if(currVec[i].returnRightNodeID() >= numClasses)
								currVec[i].setRightValue( currVec[i].returnRightNodeID() + offset);
							finalBinVec.push_back(currVec[i]);
						}
						mainsize = finalBinVec.size();
					}
					count++;
				}
				binStruct<T, Q> tempbin;
				tempbin.setBin(finalBinVec);
				BinLayout<T, Q> mergedBin(tempbin, global_fname);
				return mergedBin;
				
			}

			inline void growBins(){
				int mergeVec = 1;
				calcBinSizes();
				fpDisplayProgress printProgress;
				bins.resize(numBins);
				std::vector<BinLayout<T, Q>> binvector;
				std::string layout_str = fpSingleton::getSingleton().returnLayout();
				int depth = fpSingleton::getSingleton().returnDepthIntertwined();
#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
				for(int j = 0; j < numBins; ++j){
					binStruct<T, Q> tempbin;
					tempbin.createBin(binSizes[j], binSeeds[j], 1);
		    			
					BinLayout<T, Q> bins_serialize(tempbin, global_fname) ;
					tempbin.setBin(bins_serialize.getFinalBin());
					

					//bins_serialize.setBin();
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
				 		bins[j] = tempbin;
						//       	sizesbin.push_back((int)bins_serialize.finalbin.size());	
					}

					auto end = std::chrono::steady_clock::now();
                    			std::cout<<"Time to serialize/write to file: " <<std::chrono::duration_cast<std::chrono::seconds>(end - start).count()<<" nanoseconds.\n";
					treeRootPos = bins_serialize.treeRootPos;
                		}

				if(fpSingleton::getSingleton().returnNumThreads() > 1 && mergeVec == 1)
				{
						BinLayout<T, Q> mergedBin = mergeBinLayoutVecs(binvector);
						mergedBin.writeToFile(treeRootPos, 1);
						mergedBin.writeToFileStat();
				}

				else
				{
				for(auto single_bin: binvector){
					single_bin.writeToFile(treeRootPos, numBins);
					single_bin.writeToFileStat();
				}
				}
				
				/*
				std::fstream fsiz;
				fsiz.open("/data4/binstart.txt", std::ios::out);
				if(numBins > 1){
					for(auto i: sizesbin)
						fsiz<<i<<"\n";
				}
				else if(numBins == 1){
					fsiz<<0<<"\n";
				}
				fsiz.close();
				*/
				
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


			inline void readRandomClearCache(){
				std::fstream fi;
				int j;
				fi.open("/data4/rand_file.txt");
				for(int i=0; i<30000; ++i)
					fi>>j;	
				for(int i=0; i<20000; ++i)
					fi>>j;	
				for(int i=0; i<20000; ++i)
					fi>>j;	
				fi.close();	
			}
			
	
			inline void writeRandomToFile(){
				std::fstream fi; 
				fi.open("/data4/rand_file.txt", std::ios::out);	
			for(int i=0; i<90000; ++i)
					fi<<(i+1)%6<<"\n";
				fi.close();
			}


			inline int predictClass(int observationNumber, bool fromFile = true, std::string filename = global_fname){							  int tmp_val ;
				int treesPerBin;
				readRandomClearCache();
				std::fstream fi;

				//Read locations of the first node of each bin when there are multiple bins
				if(fpSingleton::getSingleton().returnNumThreads() > 1){
					fi.open("/data4/binstart.txt");
					sizesbin.clear();
					int sumsum = 0;
					for(int i=0; i<fpSingleton::getSingleton().returnNumThreads(); ++i){
						fi>>tmp_val;
						sizesbin.push_back(sumsum);
						sumsum+=tmp_val;
					}
					fi.close();
				}
				else
					sizesbin.push_back(0);
			
				treeRootPos.clear();

				//Read the tree root positions for BFS and Stat layouts
				std::string layout_str = fpSingleton::getSingleton().returnLayout();	
				if(layout_str.compare("bfs") == 0 || layout_str.compare("stat") == 0){
					int rootpos;
					fi.open("treeroots.csv");
					while(!fi.eof()){
                    				fi>>rootpos;
						treeRootPos.push_back(rootpos);
					}
					fi.close();
				}

				std::vector<int> predictions(fpSingleton::getSingleton().returnNumClasses(),0);
                        	treesPerBin = fpSingleton::getSingleton().returnNumTrees() / fpSingleton::getSingleton().returnNumThreads();
				
				binStruct<T, Q> temp = binStruct<T, Q>(treesPerBin);
				global_str = global_fname + std::to_string(observationNumber%NUM_FILES) + ".bin";
                       		mmappedObj.open(global_str, 0); 
                        	data = (fpBaseNode<T, Q>*)mmappedObj.getData();
				int num_threads = fpSingleton::getSingleton().returnNumThreads();
				auto start = std::chrono::steady_clock::now();

				//std::cout<<"Num threads! "<< fpSingleton::getSingleton().returnNumThreads() <<"\n";
//std::cout<<"***************************************\n";
#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
				for(int k = 0; k < numBins; ++k){
                			int uniqueCount = 0;
					//std::cout<<"sizesbin[k]: "<<sizesbin[k]<<"\n";
					//fflush(stdout);
                			fpBaseNode<T, Q> * mmapped_file_pos = data + sizesbin[k];	
					if(!fromFile)
					    bins[k].predictBinObservation(observationNumber, predictions);
		    			else{
						temp.predictBinObservation(uniqueCount, treeRootPos, mmapped_file_pos, observationNumber, predictions);
                        			if(num_threads == 1)
							blocks.push_back(uniqueCount);
                    			}
                		}
//std::cout<<"***************************************\n";
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
				writeRandomToFile();		
				
				std::string layout_str = fpSingleton::getSingleton().returnLayout();
				
				int numTried = 0;
				int numWrong = 0;
    				//for (int i = 0; i < 1; i++){
    				for (int i = 0; i <fpSingleton::getSingleton().returnNumObservations();i++){
					++numTried;
					int predClass = predictClass(i);

					if(predClass != fpSingleton::getSingleton().returnTestLabel(i)){
						++numWrong;
					}
				}
				std::cout << "\nnumWrong= " << numWrong << "\n";
				
				int depth_inter = fpSingleton::getSingleton().returnDepthIntertwined();
				std::string depth_str = std::to_string(depth_inter);
				
				//Write elapsed times to file
				f_time.open("elapsed_time_" + layout_str + "_depth_"+ depth_str + ".csv", std::ios::out);
				for(auto t: etime)
					f_time<<t<<",";
				f_time.close();
				
				//Write number of blocks to file (do not write in case of parallel bin experiment)
				int num_threads = fpSingleton::getSingleton().returnNumThreads(); 
				if(num_threads == 1){
					fblock.open("blocks_" + layout_str + "_depth_" + depth_str + ".csv", std::ios::out|std::ios::app);
					for (auto block: blocks)
						fblock<<block<<",";
					fblock.close();
				}
				
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
