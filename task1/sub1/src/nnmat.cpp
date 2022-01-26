#include <iostream>
#include <vector>
#include <cmath>

#define mat std::vector<std::vector<float>>
#define vec std::vector<float>

float relu (float n) { return ((n > 0) ? n : 0); }

void activate(mat& M, float (*fun)(float)) {
    for (int i=0; i<M.size(); i++) {
        for (int j=0; j<M[0].size(); j++) {
            M[i][j] = fun(M[i][j]);
        }
    }
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

mat pool(mat& input, int stride, float (*poolfn)(mat& subpool)) {
    mat M(input.size()/stride, vec(input[0].size()/stride, 0));

    for (int i=0; i<M.size(); i++) {
        for (int j=0; j<M[0].size(); j++) {
            mat submat = submatrix(input,i*stride,j*stride,stride,stride);
            M[i][j] = poolfn(submat);
        }
    }

    return M;
}

vec normalize_sigmoid(vec& input) {
    vec output(input.size());
    for (int i=0; i<input.size(); i++) {
        output[i] = 1/(1+exp(input[i]));
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