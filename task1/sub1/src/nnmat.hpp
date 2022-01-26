#include <iostream>
#include <vector>

#define mat std::vector<std::vector<float>>
#define vec std::vector<float>

float relu(float);

void activate(mat& M, float (*fun)(float));

mat fc_layer(mat& input, mat& weights, mat& bias);

float avg(mat& input);
float max(mat& input);

mat pool(mat& input, mat& stride);

vec normalize_sigmoid(vec& input);
vec normalize_softmax(vec& input);