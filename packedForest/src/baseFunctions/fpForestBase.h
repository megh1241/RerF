#ifndef fpForestBase_h
#define fpForestBase_h

#include "displayProgress.h"
#include <vector>
#include <stdio.h>
#include <ctime>
#include <chrono>
#include <cstdlib>
#include <eigen3/Eigen/Sparse>
#include <eigen3/Eigen/Core>

namespace fp{
	template <typename T>
		class fpForestBase
		{
			private:
				Eigen::SparseMatrix<int> spr;
			public:
				
				fpForestBase(){
				std::srand(unsigned(std::time(0)));
				}
				Eigen::SparseMatrix<int> eigenMat;
				virtual ~fpForestBase() {}
				virtual void printForestType() = 0;
				virtual void growForest() = 0;
				virtual float testForest() = 0;
				virtual int predictClass(std::vector<T>& observation) = 0;
				virtual std::vector<int> predictClassPost(std::vector<T>& observation) = 0;
				virtual int predictClass(const T* observation) = 0;
				inline Eigen::SparseMatrix<int> &returnSparseMat(){
					return spr;
				}
		};

}//namespace fp
#endif //fpForestBase.h
