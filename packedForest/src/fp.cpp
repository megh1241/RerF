#include "packedForest.h"
#include <iostream>
#include <exception>
#include <cstring>

int main(int argc, char* argv[]) {
    std::cout<<"Format: ./bin/fp <alg> <dataset> <num cores> <layout> <depth intertwined> <train/test>\n";

    if (argc != 7) 
	    return -1;
    
    
    int alg = atoi(argv[1]);
    int dataSet = atoi(argv[2]);
    int numCores = atoi(argv[3]);
    std::string layout_user(argv[4]);
    int depth_intertwined = atoi(argv[5]);
    std::string train_or_test(argv[6]);
    
    
    if (alg == 0) {
        // Remove and change this block for testing.
        std::cout << "test algorithm selected without code." << std::endl;
        try {
            return 0;
        }
        catch(const std::exception& e) {
            std::cerr << e.what() << '\n';
        }
    } else {
        try{
            fp::fpForest<float> forest;

            switch(alg){
                case 1:
                    forest.setParameter("forestType", "rerf");
                    break;
                case 2:
                    forest.setParameter("forestType", "rfBase");
                    break;
                case 3:
                    forest.setParameter("forestType", "rerf");
                    forest.setParameter("binSize", 1000);
                    forest.setParameter("binMin", 1000);
                    break;
                case 4:
                    forest.setParameter("forestType", "rfBase");
                    forest.setParameter("binSize", 100);
                    forest.setParameter("binMin", 1000);
                    break;
                case 7:
                    forest.setParameter("forestType", "binnedBase");
                    forest.setParameter("numTreeBins", numCores);
                    //forest.setParameter("maxDepth", 20);
                    break;
                case 8:
                    forest.setParameter("forestType", "binnedBaseRerF");
                    forest.setParameter("numTreeBins", numCores);
                    break;
                case 9:
                    forest.setParameter("forestType", "binnedBase");
                    forest.setParameter("numTreeBins", numCores);
                    forest.setParameter("maxDepth", 5);
                    break;
                case 10:
                    forest.setParameter("forestType", "binnedBaseRerF");
                    forest.setParameter("numTreeBins", numCores);
                    forest.setParameter("maxDepth", 2);
                    break;
                case 11:
                    forest.setParameter("forestType", "binnedBaseTern");
                    forest.setParameter("numTreeBins", numCores);
                    forest.setParameter("methodToUse", 1);
                    break;
                case 12:
                    forest.setParameter("forestType", "binnedBaseTern");
                    forest.setParameter("numTreeBins", numCores);
                    forest.setParameter("methodToUse", 2);
                    forest.setParameter("imageHeight", 28);
                    forest.setParameter("imageWidth", 28);
                    forest.setParameter("patchHeightMax", 5);
                    forest.setParameter("patchHeightMin", 5);
                    forest.setParameter("patchWidthMax", 5);
                    forest.setParameter("patchWidthMin", 5);
                    std::cout << "\nForcing dataset to be MNIST:\n";
                    dataSet = 3;
                    break;
                case 13:
                    forest.setParameter("forestType", "urf");
                    break;
                case 14:
                    forest.setParameter("forestType", "urerf");
                    break;

                default:
                    std::cout << "unknown alg selected" << std::endl;
                    return -1;
                    break;
            }


            switch(dataSet){
                case 1:
                    forest.setParameter("CSVFileName", "res/iris.csv");
                    forest.setParameter("columnWithY", 4);
                    break;
                case 2:
                    forest.setParameter("CSVFileName", "res/higgs2.csv");
                    forest.setParameter("columnWithY", 0);
                    break;
                case 3:
                    forest.setParameter("CSVFileName", "res/mnist.csv");
                    forest.setParameter("columnWithY", 0);
                    break;
                case 4:
                    forest.setParameter("CSVFileName", "res/HIGGS.csv");
                    forest.setParameter("columnWithY", 0);
                    break;
                case 5:
                    forest.setParameter("CSVFileName", "../experiments/res/higgsData.csv");
                    forest.setParameter("columnWithY", 0);
                    break;
                case 6:
                    forest.setParameter("CSVFileName", "../experiments/res/p53.csv");
                    forest.setParameter("columnWithY", 5408);
                    break;
                case 8: //TODO: better way (automatically ? ) to find the number of columns in a given csv file
                    forest.setParameter("CSVFileName", "datasets/cifar-10.csv");
                    forest.setParameter("columnWithY", 3072);
                    break;
                case 7: 
                    forest.setParameter("CSVFileName", "/data4/cifar-100.csv");
                    forest.setParameter("columnWithY", 0);
                    break;
                case 9: 
                    forest.setParameter("CSVFileName", "res/fashion-mnist_train.csv");
                    forest.setParameter("columnWithY", 0);
                    break;
                default:
                    std::cout << "unknown dataset selected" << std::endl;
                    return -1;
                    break;
            }

            //forest.setParameter("numTreesInForest", 2048);
            forest.setParameter("numTreesInForest", 128);
            forest.setParameter("minParent", 1);
            forest.setParameter("numCores", numCores);
            //forest.setParameter("seed",-1661580697);
            forest.setParameter("seed",1);
	    forest.setParameter("layout", layout_user);
	    forest.setParameter("depthIntertwined", depth_intertwined);

	    std::string train_str("train");
	    if(train_or_test.compare(train_str) == 0)
            	forest.growForest();
	    else {
            	forest.initTestForest();
		std::cout << "error: " << forest.testAccuracy() << "\n";
	    }


        }catch(std::exception& e){
            std::cout << "standard error: " << e.what() << std::endl;
        }
    }
}

