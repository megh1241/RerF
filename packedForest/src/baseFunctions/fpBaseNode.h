#ifndef fpBaseNode_h
#define fpBaseNode_h

#include <stdio.h>
#include <fstream>
#include <iterator>
#include <type_traits>

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
		fpBaseNode(int classNum ):left(0), right(classNum), depth(-1){}

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

       	 	inline void outputFeature(std::ostream &out, std::vector<int>& featureVec, fpBaseNode<T, F> & obj){
            		if(obj.isInternalNodeFront()){
                		for(auto i: featureVec){
                    			out<<i<<"\n";
                		}
            		}
        	}

        inline void inputFeature(std::istream &in, int feat, fpBaseNode<T, F> & obj){
            in>>obj.left;
            in>>obj.right;
            in>>obj.cutValue;
            in>>obj.feature;
            //std::cout<<obj.left<<"\n"<<obj.right<<"\n"<<obj.cutValue<<"\n"<<obj.feature<<"\n";
        }
        
        inline void inputFeature(std::istream &in, const fp::weightedFeature& feat, fpBaseNode<T, F> & obj){
            in>>obj.left;
            in>>obj.right;
            in>>obj.cutValue;
        }
        
        inline void inputFeature(std::istream &in, std::vector<int>& featureVec, fpBaseNode<T, F> & obj){
            in>>obj.left;
            in>>obj.right;
            in>>obj.cutValue;
            for(auto i: featureVec){
                in>>i;
            }
        }

        inline void outputFeature(std::ostream &out, int feat, fpBaseNode<T, F> & obj){
            if(obj.isInternalNodeFront())
                out << obj.left << "\n" <<obj.right<<"\n"<<obj.cutValue<<"\n"<<obj.feature<<"\n";
            else
                out<<0<<"\n"<<obj.right<<"\n"<<0<<"\n"<<0<<"\n";
        
            /*if(obj.isInternalNodeFront())
                out << obj.left << "\n"<<obj.right << "\n"<<obj.cutValue <<"\n"<< obj.feature;
            else
                out<<0<<"\n"<<obj.right<<"\n"<<0<<"\n"<<0;*/
        }
        
        inline void outputFeature(std::ostream &out, const fp::weightedFeature& feat, fpBaseNode<T, F> & obj){
            out << obj.left << "\n" <<obj.right<<"\n"<<obj.cutValue<<"\n";
        }

		inline void virtual printNode(){
			if(isInternalNode()){
				std::cout << "internal ";
			}else{
				std::cout << "leaf ";
			}
			std::cout << "cutValue " << cutValue << ", left " << left << ", right " << right << ", depth " << depth << "\n";
		}

        friend std::ostream & operator << (std::ostream &out, fpBaseNode<T, F> & obj){
            obj.outputFeature(out, obj.feature, obj);
            return out;
        }

        friend std::istream & operator >> (std::istream &in, fpBaseNode<T, F> & obj){
            obj.inputFeature(in, obj.feature, obj);
            return in;
        }
};

template <typename T, typename F>
class fpBaseNodeStat : public fpBaseNode<T, F>
{
		int cardinality;
        	int id;
		//subtreeNum = -2 indicates class, =-1 indicates interleaved BIN
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
			//fpBaseNode<T, F>(cutValue, depth, feature);
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
		
		int getID(){
			return id;
		}
		
		void setID(int idToSet){
			id = idToSet;
		}
		inline void printNode(){
			fpBaseNode<T, F>::printNode();
			std::cout<<" id: "<<id<<"\n";
		}

};






#endif //fpBaseNode_h
