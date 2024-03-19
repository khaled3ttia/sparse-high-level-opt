#include <random>
#include <iomanip>

void generateMatrix(int nrows, int ncols, float nnzRate, float *&vals){
	std::random_device rd; 
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dist(0,1);
	std::uniform_real_distribution<> dist2(1, 200);
	for (int i = 0; i < nrows; i++){
		for (int j = 0; j < ncols; j++){
			if (dist(gen) < nnzRate){
				vals[i * ncols + j] = dist2(gen);
			}else {
				vals[i * ncols + j] = 0;
			}		

		}
	}

}

void printVector(int (&vec)[8], int size){

	for (int i = 0 ; i < size; i++) {
		if (i == 0) {
			std::cout << "[";

		}
		std::cout << vec[i];
		if (i != size - 1){
			std::cout << ",";  

		}else {
			std::cout << "]" << std::endl;
		}


	}


}

void printVector(int *&vec, int size){

	for (int i = 0 ; i < size; i++) {
		if (i == 0) {
			std::cout << "[";

		}
		std::cout << vec[i];
		if (i != size - 1){
			std::cout << ",";  

		}else {
			std::cout << "]" << std::endl;
		}


	}


}

void printVector(float *&vec, int size){

	for (int i = 0 ; i < size; i++) {
		if (i == 0) {
			std::cout << "[";

		}
		std::cout << vec[i];
		if (i != size - 1){
			std::cout << ",";  

		}else {
			std::cout << "]" << std::endl;
		}


	}


}


void printMatrix(float *&denseMatrix, int nrows, int ncols){
	
	int numElements = nrows * ncols; 

	for (int i = 0 ; i < numElements; i++){

		
		std::cout << std::fixed <<  std::setprecision(1)  <<  denseMatrix[i];
		
		if ((i+1) % ncols == 0){
			std::cout << "\n";
		}else{
			std::cout << "\t";
		}

	}
	
}
