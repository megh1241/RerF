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
std::vector<double>final_etime;
double time_per_batch;
std::vector<int> sizesbin;
std::fstream f_time;
std::fstream fblock;
int batchsize1 = 10;
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
				std::vector<fpBaseNodeStat<T, Q>> reVec;
				std::vector<fpBaseNodeStat<T, Q>> roots;
				std::map<int, int> nodeNewIdx;


				for(auto binl : layoutVec)
				{
					std::vector<fpBaseNodeStat<T, Q>> currVec = binl.getFinalBin();
					std::cout<<"************************\n";
					for(auto i: currVec){
					std::cout<<"id: "<<i.getID()<<"\n";
					i.printNode();
					}

				}
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
				std::cout<<"\n\n PRINTINF LATERTATYFSTUY\n\n";
				for(auto i: finalBinVec){
					std::cout<<"id: "<<i.getID()<<"\n";
					i.printNode();
				}
				std::vector<fpBaseNodeStat<T, Q>> bin;
                                int siz = finalBinVec.size();


				for(int i=0; i<siz; ++i)
					finalBinVec[i].setID(i);

				for(auto i: finalBinVec){
					bin.push_back(i);
				}
				//for(int i=0; i<bin.size(); i++)
				//	bin[i].setID(i);
				//sort finalBinVec
				for(auto node: finalBinVec)
				{
					if(node.getDepth() <= 0)
						roots.push_back(node);
					else
						reVec.push_back(node);
				}
				finalBinVec.clear();
				for(auto node: roots){
					finalBinVec.push_back(node);
				}
				for(auto node: reVec){
					finalBinVec.push_back(node);
				}

				nodeNewIdx.clear();
				for(int i=0; i<siz; ++i){
					nodeNewIdx.insert(std::pair<int, int>(finalBinVec[i].getID(), i));
				}

                                for (auto i=numClasses; i<siz; i++){
                                        finalBinVec[i].setLeftValue(nodeNewIdx[bin[finalBinVec[i].returnLeftNodeID()].getID()]);
                                        finalBinVec[i].setRightValue(nodeNewIdx[bin[finalBinVec[i].returnRightNodeID()].getID()]);
                                }
				for(int i=0; i<siz; ++i)
					finalBinVec[i].setID(i);

				binStruct<T, Q> tempbin(128);
				tempbin.setBin(finalBinVec);
				BinLayout<T, Q> mergedBin(tempbin, global_fname);
				mergedBin.setFinalBin(finalBinVec);
				bin.clear();
				roots.clear();
			        reVec.clear();
				nodeNewIdx.clear();
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
				int treesPerBin = fpSingleton::getSingleton().returnNumTrees() / fpSingleton::getSingleton().returnNumThreads();
				std::vector<std::string> layout_names = {"bfs", "stat", "binstat", "binbfs", "binstatclass"};

				//int numTrees = fpSingleton::getSingleton().returnN
#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
				for(int j = 0; j < numBins; ++j){
					binStruct<T, Q> tempbin;
					tempbin.createBin(treesPerBin, binSeeds[j], 1);
		    			
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
					else if(layout_str.compare("binstatclass") == 0)
                        			bins_serialize.BINStatClassLayout(depth);
                        		//else
                        		//	bins_serialize.BINStatClassLayout(depth);

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

				std::string filename_ser;
				if(fpSingleton::getSingleton().returnNumThreads() > 1 && mergeVec == 1)
				{
						BinLayout<T, Q> mergedBin = mergeBinLayoutVecs(binvector);
						BinLayout<T, Q> mergedBin2 = mergedBin;
						BinLayout<T, Q> mergedBin3 = mergedBin;
						BinLayout<T, Q> mergedBin4 = mergedBin;
						BinLayout<T, Q> mergedBin5 = mergedBin;
						BinLayout<T, Q> mergedBin6 = mergedBin;
						
						mergedBin4.BINStatLayout(depth);	
						filename_ser = "/data4/binstat";
						mergedBin4.setFilename(filename_ser);
						mergedBin4.writeToFile(mergedBin4.treeRootPos, 1);
						mergedBin4.writeToFileStat("/data4/binstats");
						
						mergedBin5.BINStatClassLayout(depth);	
						filename_ser = "/data4/binstatclass";
						mergedBin5.setFilename(filename_ser);
						mergedBin5.writeToFile(mergedBin5.treeRootPos, 1 );
						mergedBin5.writeToFileStat("/data4/binstatclasssdsfsd");
						
						mergedBin2.BFSLayout();	
						filename_ser = "/data4/bfs";
						mergedBin2.setFilename(filename_ser);
						mergedBin2.writeToFile(mergedBin2.treeRootPos, 1, "bfstreeroots.csv");
						mergedBin2.writeToFileStat("/data4/bfs2s");


						mergedBin3.statLayout();
						filename_ser = "/data4/stat";
						mergedBin3.setFilename(filename_ser);
						mergedBin3.writeToFile(mergedBin2.treeRootPos, 1, "stattreeroots.csv");
						mergedBin3.writeToFileStat("/data4/stat2s");
					
						mergedBin6.BINBFSLayout(depth);	
						filename_ser = "/data4/binbfs";
						mergedBin6.setFilename(filename_ser);
						mergedBin6.writeToFile(mergedBin6.treeRootPos, 1);
						mergedBin6.writeToFileStat("/data4/binbfssdfdsf");

				}

				else
				{
					for(auto single_bin: binvector){
						single_bin.writeToFile(treeRootPos, numBins);
						single_bin.writeToFileStat("/data4/nodeStat");
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


			inline int predictClass(int observationNumber, bool fromFile = true, std::string filename2 = "/data4/binbfs"){							  int tmp_val ;
				int treesPerBin;
				int batchsize = fpSingleton::getSingleton().returnBatchsize();
				if(observationNumber % batchsize == 0){
					readRandomClearCache();
					readRandomClearCache();
				}
				std::fstream fi;

				std::string layout_str = fpSingleton::getSingleton().returnLayout();
				std::string filename = "/data4/" + layout_str;
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
				if(layout_str.compare("bfs") == 0 || layout_str.compare("stat") == 0){
					int rootpos;
					fi.open(layout_str + "treeroots.csv");
					while(!fi.eof()){
                    				fi>>rootpos;
						treeRootPos.push_back(rootpos);
					}
					fi.close();
				}

				std::vector<int> predictions(fpSingleton::getSingleton().returnNumClasses(),0);
                        	treesPerBin = fpSingleton::getSingleton().returnNumTrees() / fpSingleton::getSingleton().returnNumThreads();
				
				binStruct<T, Q> temp = binStruct<T, Q>(treesPerBin);
				global_str = filename + std::to_string(observationNumber%NUM_FILES) + ".bin";
                       		mmappedObj.open(global_str, 0); 
                        	data = (fpBaseNode<T, Q>*)mmappedObj.getData();
				int num_threads = fpSingleton::getSingleton().returnNumThreads();
				auto start = std::chrono::steady_clock::now();

				//std::cout<<"Num threads! "<< fpSingleton::getSingleton().returnNumThreads() <<"\n";
//std::cout<<"***************************************\n";
#pragma omp parallel for num_threads(fpSingleton::getSingleton().returnNumThreads())
				for(int k = 0; k < 1; ++k){
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
				std::cout<<"Number of classes: !!\n"<<	fpSingleton::getSingleton().returnNumClasses()<<"\n";
				int batchsize = fpSingleton::getSingleton().returnBatchsize();
				writeRandomToFile();		
				
				std::string layout_str = fpSingleton::getSingleton().returnLayout();
			
				std::cout<<"Number of classes: !!\n"<<	fpSingleton::getSingleton().returnNumClasses()<<"\n";
				int numTried = 0;
				int numWrong = 0;
    				//for (int i = 0; i < 1; i++){
    				for (int i = 0; i <fpSingleton::getSingleton().returnNumObservations();i++){
					++numTried;
					int predClass = predictClass(i);

					if(predClass != fpSingleton::getSingleton().returnTestLabel(i)){
						++numWrong;
					}

					time_per_batch += etime[i];
					if(i%batchsize == 0)
					{
						final_etime.push_back(time_per_batch);
						time_per_batch = 0;
					}
				}
				std::cout << "\nnumWrong= " << numWrong << "\n";
				
				int depth_inter = fpSingleton::getSingleton().returnDepthIntertwined();
				std::string depth_str = std::to_string(depth_inter);
				
				//Write elapsed times to file
				f_time.open("elapsed_time_" + layout_str + "_depth_"+ depth_str + ".csv", std::ios::out);
				for(auto t: final_etime)
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
