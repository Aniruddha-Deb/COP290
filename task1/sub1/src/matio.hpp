#include "nnmat.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

mat read_matrix(std::string filename);

vec read_vector(std::string filename);

void print_matrix(mat& matrix);

void print_vector(vec& vector);

void print_matrix_to_file(mat& matrix, std::string filename);

void print_vector_to_file(vec& vector, std::string filename);