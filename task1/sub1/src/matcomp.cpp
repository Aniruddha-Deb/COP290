#include "matio.hpp"
#include "nnmat.hpp"
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

// for internal purposes only, so minimal debugging/error reporting.
int main(int argc, char** argv) {

    string type(argv[1]), loc1(argv[1]), loc2(argv[2]);
    if (type == "mat") {
        mat mat1 = read_matrix(loc1);
        mat mat2 = read_matrix(loc2);

        if (!mat_eq(mat1, mat2)) {
            cout << "Error: matrices are not equal!" << endl;
            return 1; // NZEC
        }
        else cout << "Compared matrices are equal" << endl;
    }
    else if (type == "vec") {
        vec vec1 = read_vector(loc1);
        vec vec2 = read_vector(loc2);

        if (!vec_eq(vec1, vec2)) {
            cout << "Error: vectors are not equal!" << endl;
            return 1; // NZEC
        }
        else cout << "Compared vectors are equal" << endl;        
    }


    return 0;
}