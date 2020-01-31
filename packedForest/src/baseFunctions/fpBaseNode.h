#ifndef fpBaseNode_h
#define fpBaseNode_h

#include <stdio.h>

template <typename T, typename F>
class alignas(32) fpBaseNode
{
	protected:
		int left;
		F feature;
		T cutValue;
		int right;
		int depth;

	public:
		fpBaseNode():left(0), right(0), depth(0){}
		fpBaseNode(T cutValue, int depth, F feature): left(0),feature(feature),cutValue(cutValue),right(0), depth(depth){}
		fpBaseNode(int classNum):left(0), right(classNum), depth(-1){}

		inline bool isInternalNode(){
			return left;
		}

		inline bool isInternalNodeFront(){
			return depth >= 0;
		}

		inline int returnDepth(){
			return depth;
		}

		inline void setDepth(int dep){
			depth = dep;
		}

		inline T returnCutValue(){
			return cutValue;
		}

		inline void setCutValue(T cVal){
			cutValue = cVal;
		}

		inline int returnLeftNodeID(){
			return left;	
		}

		inline int returnRightNodeID(){
			return right;
		}

		inline int returnClass(){
			return right;	
		}

		inline void setClass(int classNum){
			right = classNum;
			left = 0;
		}

		inline void setSharedClass(int classNum){
			right = classNum;
			left = -1;
			depth = -1;
		}

		inline void setLeftValue(int LVal){
			left = LVal;	
		}

		inline void setRightValue(int RVal){
			right = RVal;
		}

		inline bool goLeft(T featureValue){
			return featureValue <= cutValue;
		}

		/*
			 inline void setFeatureValue(int fVal){
			 feature = fVal;
			 }

			 inline void setFeatureValue(std::vector<int> fVal){
			 feature = fVal;
			 }
			 */

		inline void setFeatureValue(F fVal){
			feature = fVal;
		}

		inline F& returnFeatureNumber(){
			return feature;
		}


		inline int nextNode(T featureVal){
			return (featureVal <= cutValue) ? left : right;
		}

		inline int nextNodeHelper(std::vector<T>& observation, std::vector<int>& featureVec){
			T featureVal = 0;
			for(auto featureNumber : featureVec){
				featureVal += observation[featureNumber];
			}
			return (featureVal <= cutValue) ? left : right;
		}


		inline int nextNodeHelper(std::vector<T>& observation, int featureIndex){
			return (observation[featureIndex] <= cutValue) ? left : right;
		}


		inline int nextNode(std::vector<T>& observation){
			return	nextNodeHelper(observation, feature);
		}


		inline int nextNode(const T* observation){
			return	nextNodeHelper(observation, feature);
		}
		inline int nextNodeHelper(const T* observation, std::vector<int>& featureVec){
			T featureVal = 0;
			for(auto featureNumber : featureVec){
				featureVal += observation[featureNumber];
			}
			return (featureVal <= cutValue) ? left : right;
		}
		inline int nextNodeHelper(const T* observation, int featureIndex){
			return (observation[featureIndex] <= cutValue) ? left : right;
		}


		inline void addFeatureValue(int fVal){
			feature.push_back(fVal);
		}

                inline void printNode(){
                        std::cout << "cutValue " << cutValue <<"\n";
                        fflush(stdout);
                        std::cout << ", left " << left <<"\n";
                        fflush(stdout);
                        std::cout<<", right " << right <<"\n";
                        fflush(stdout);
                        std::cout<<", depth " << depth <<"\n";
                        fflush(stdout);
                        std::cout<<"_______________________________\n";
                        fflush(stdout);
                }

};

template <typename T, typename F>
class fpBaseNodeStat : public fpBaseNode<T, F>
{
                int cardinality;
                int id;
                int left_leaf_card;
                int right_leaf_card;
                int subtreeNum;
        public:
                fpBaseNodeStat(){
                        fpBaseNode<T, F>();
                }
                fpBaseNodeStat(T cutValue, int depth, F feature, int uid, int card){
                        cardinality = card;
                        id = uid;
                        this->setCutValue(cutValue);
                        this->setFeatureValue(feature);
                        this->setDepth(depth);
                }

                void setSTNum(int num){
                        subtreeNum = num;
                }

                int getSTNum(){
                        return subtreeNum;
                }

                int getCard(){
                        return cardinality;
                }

                void setLeftLeafCard(int cardi){
                        left_leaf_card = cardi;
                }

                void setRightLeafCard(int cardi){
                        right_leaf_card = cardi;
                }

                int getLeftLeafCard(){
                        return left_leaf_card;
                }

                int getRightLeafCard(){
                        return right_leaf_card;
                }

                int getID(){
                        return id;
                }

                void setID(int idToSet){
                        id = idToSet;
                }

                void printID(){
                        std::cout<<"Self ID: "<<id<<"\n";
                        std::cout<<"ST Num: "<<subtreeNum<<"\n";
                }
                void printSTNum(){
                        std::cout<<"ST Num: "<<subtreeNum<<"\n";
                }

};

#endif //fpBaseNode_h
