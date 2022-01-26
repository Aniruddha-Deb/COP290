#include "nnmat.hpp"

#include <iostream>
#include <vector>
#include <limits>
#include <cmath>

float relu (float n) { return ((n > 0) ? n : 0); }
// reinvent the wheel - tanx function
float rtw  (float n) { return ((exp(n)-exp(-1*n))/(exp(n)+exp(-n))); }

void activate(mat& M, float (*fun)(float)) {
    for (int i=0; i<M.size(); i++) {
        for (int j=0; j<M[0].size(); j++) {
            M[i][j] = fun(M[i][j]);
        }
    }
}

bool mat_eq(mat& m1, mat& m2) {
    if (m1.size() != m2.size() || m1.size() == 0) return false;
    if (m1[0].size() != m2[0].size() || m1[0].size() == 0) return false;

    for (int i=0; i<m1.size(); i++) {
        for (int j=0; j<m1[0].size(); j++) {
            if (abs(m1[i][j] - m2[i][j]) > EPS) return false;
        }
    }

    return true;
}

bool vec_eq(vec& v1, vec& v2) {
    if (v1.size() != v2.size() || v1.size() == 0) return false;

    for (int i=0; i<v1.size(); i++) {
        if (abs(v1[i] - v2[i]) > EPS) return false;
    }

    return true;
}

mat fc_layer(mat& input, mat& weights, mat& bias) {
    mat M(bias.size(), vec(bias[0].size()));

    std::cout << "init M" << std::endl;

    for (int i=0; i<M.size(); i++) {
        for (int j=0; j<M[0].size(); j++) {
            for (int k=0; k<input[0].size(); k++) {
                M[i][j] += input[i][k] * weights[k][j];
            }
            M[i][j] += bias[i][j];
        }
    }

    std::cout << "Computed M" << std::endl;

    return M;
}

mat submatrix(mat& source, int x, int y, int r, int c) {
    mat submat(r, vec(c,0));

    for (int i=y; i<y+r; i++) {
        for (int j=x; j<c+x; j++) {
            submat[i-y][j-x] = source[i][j];
        }
    }

    return submat;
}

float pool_avg(mat& matrix) {
    float sum = 0;
    for (auto v : matrix) {
        for (auto i : v) {
            sum += i;
        }
    }

    return ((sum)/(matrix.size()*matrix[0].size()));
}

float pool_max(mat& matrix) {
    float max = -std::numeric_limits<float>::max();
    for (auto v : matrix) {
        for (auto i : v) {
            if (i > max) max = i;
        }
    }

    return max;
}

mat pool(mat& input, int stride, float (*poolfn)(mat& subpool)) {
    mat M(input.size()/stride, vec(input[0].size()/stride, 0));

    for (int i=0; i<M.size(); i++) {
        for (int j=0; j<M[0].size(); j++) {
            mat submat = submatrix(input,j*stride,i*stride,stride,stride);
            M[i][j] = poolfn(submat);
        }
    }

    return M;
}

vec normalize_sigmoid(vec& input) {
    vec output(input.size());
    for (int i=0; i<input.size(); i++) {
        output[i] = 1/(1+exp(-input[i]));
    }   
    return output;
}

vec normalize_softmax(vec& input) {
    float denom = 0;
    for (float i : input) {
        denom += exp(i);
    }

    vec output(input.size());
    for (int i=0; i<input.size(); i++) {
        output[i] = exp(input[i])/denom;
    }
    return output;
}