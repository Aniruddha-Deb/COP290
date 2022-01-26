#include <iostream>
#include <vector>

// ? would defining a matrix type become overkill? I think matrix libraries do 
// this. This would be neat, because it would allow us to override operators 
// for matrices and vectors, and allow multiplication/equality comparisions.
// for now, macros is ok, along with an equal method
#define mat std::vector<std::vector<float>>
#define vec std::vector<float>
#define EPS 1e-6

float relu(float);
float rtw(float);

bool mat_eq(mat& m1, mat& m2);
bool vec_eq(vec& v1, vec& v2);

void activate(mat& M, float (*fun)(float));

mat fc_layer(mat& input, mat& weights, mat& bias);

float pool_avg(mat& input);
float pool_max(mat& input);

mat pool(mat& input, int stride, float (*poolfn)(mat& subpool));

vec normalize_sigmoid(vec& input);
vec normalize_softmax(vec& input);