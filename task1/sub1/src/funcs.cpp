#include <iostream>
#include <vector>
#include <cmath>

using namespace std

#define mat vector<vector<float>>

inline float relu (float n) { return ((n > 0) ? n : 0); }

// M is in row major order: vectors of M contain rows and not columns
void activate_relu (mat& M) {
	for (int i=0; i<M.size(); i++) {
		for (int j=0; j<M[0].size(); j++) {
			M[i][j] = relu(M[i][j]);
		}
	}
}

void activate_tanh (mat& M) {
	for (int i=0; i<M.length(); i++) {
		for (int j=0; j<M[0].length(); j++) {
			M[i][j] = tanh(M[i][j]);
		}
	}
}

vector<vector<float>> fconn(vector<vector<int>>
