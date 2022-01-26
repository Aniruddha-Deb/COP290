#include "nnmat.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

mat read_matrix(string filename) {
    string line;

    cout << "reading matrix" << endl;

    ifstream source; 
    source.open(filename,ios::in);
    if (source) {
        stringstream buffer;
        buffer << source.rdbuf();
        int c, r;
        buffer >> c >> r;
        mat M(r,vec(c));
        for (int i=0; i<c; i++) {
            for (int j=0; j<r; j++) {
                buffer >> M[j][i];
                cout << "read " << M[j][i] << endl;
            }
        }
        cout << "Read matrix" << endl;
        source.close();
        return M;
    }

    mat K;
    return K;
}

void print_matrix(mat& matrix) {
    for (auto v : matrix) {
        for (float i : v) {
            cout << i << " ";
        }
        cout << endl;
    }
}

void print_matrix_to_file(mat& matrix, string filename) {

    ofstream out;
    out.open(filename, ios::out);
    if (out) {
        out << matrix[0].size() << endl;
        out << matrix.size() << endl;
        for (int i=0; i<matrix[0].size(); i++) {
            for (int j=0; j<matrix.size(); j++) {
                out << matrix[j][i] << endl;
            }
        }
    }
}

int main(int argc, char** argv) {

    cout << argc << endl;

    if (argc == 0) {
        cout << "Error: test requires a command and input/output files. \n type help for more" << endl;
        return 0;
    }

    string cmd(argv[1]);

    if (cmd == "help") {
        cout << "Usage:" << endl;
        cout << "./test fullyconnected inputmatrix.txt weightmatrix.txt biasmatrix.txt outputmatrix.txt" << endl;
        cout << "./test activation relu inputmatrix.txt outputmatrix.txt" << endl;
        cout << "./test activation tanh inputmatrix.txt outputmatrix.txt" << endl;
        cout << "./test pooling max inputmatrix.txt stride outputmatrix.txt" << endl;
        cout << "./test pooling average inputmatrix.txt stride outputmatrix.txt" << endl;
        cout << "./test probability softmax inputvector.txt outputvector.txt" << endl;
        cout << "./test probability sigmoid inputvector.txt outputvector.txt" << endl;
    }
    else if (cmd == "fullyconnected") {
        if (argc < 6) goto err;
        string input(argv[2]), weight(argv[3]), bias(argv[4]), output(argv[5]);
        mat inputmat = read_matrix(input);
        mat weightmat = read_matrix(weight);
        mat biasmat = read_matrix(bias);

        mat outputmat = fc_layer(inputmat, weightmat, biasmat);

        print_matrix(outputmat);
    }
    else if (cmd == "activation") {
        if (argc < 5) goto err;
        string type(argv[2]), input(argv[3]), output(argv[4]);
        mat inputmat = read_matrix(input);
        if (type == "relu") activate(inputmat, relu);
        else if (type == "tanh") activate(inputmat, tanh);
        else goto err;

        print_matrix(inputmat);
    }

    return 0;

err:
    cout << "that command does not exist. Type help for info" << endl;
    return 0;
}