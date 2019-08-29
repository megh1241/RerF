#ifndef fpBaseNode_h
#define fpBaseNode_h

#include <stdio.h>
#include <fstream>

template <typename T, typename F>
class alignas(32) fpBaseNode
{
	protected:
		int left;
		F feature;
		T cutValue;
		int right;
		int depth;
        int id;

	public:
		fpBaseNode():left(0), right(0), depth(0), id(0){}
		fpBaseNode(T cutValue, int depth, F feature, int uid): left(0),feature(feature),cutValue(cutValue),right(0), depth(depth), id(uid){}
		fpBaseNode(int classNum, int uid):left(0), right(classNum), depth(-1), id(uid){}

		inline bool isInternalNode(){
			return left;
		}

        inline int getID() {
            return id;
        }

        inline int setID(int uid) {
            id = uid;
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


		inline void virtual printNode(){
			if(isInternalNode()){
				std::cout << "internal ";
			}else{
				std::cout << "leaf ";
			}
			std::cout << "cutValue " << cutValue << ", left " << left << ", right " << right << ", depth " << depth << "\n";
		}
        
        friend std::ostream & operator << (std::ostream &out, const fpBaseNode<T, F> & obj){
            out << obj.left << "\n" <<obj.right<<"\n"<<obj.cutValue<<"\n";
            return out;
        }
        
        friend std::istream & operator >> (std::istream &in, const fpBaseNode<T, F> & obj){
            in>>obj.left;
            in>>obj.right;
            in>>obj.cutValue;
            //in>>obj.feature;
        }
};
#endif //fpBaseNode_h
