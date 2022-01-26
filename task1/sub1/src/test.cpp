#include "nnmat.hpp"
#include "matio.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

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
        print_matrix_to_file(outputmat, output);
    }
    else if (cmd == "activation") {
        if (argc < 5) goto err;
        string type(argv[2]), input(argv[3]), output(argv[4]);
        mat inputmat = read_matrix(input);
        if (type == "relu") activate(inputmat, relu);
        else if (type == "tanh") activate(inputmat, rtw);
        else goto err;

        print_matrix(inputmat);
        print_matrix_to_file(inputmat, output);
    }
    else if (cmd == "pooling") {
        if (argc < 6) goto err;
        string type(argv[2]), input(argv[3]), stride_str(argv[4]), output(argv[5]);
        int stride = stoi(stride_str);

        mat inputmat = read_matrix(input);
        float (*poolfn)(mat&);
        if (type == "max") poolfn = pool_max;
        else if (type == "average") poolfn = pool_avg;
        else goto err;
        mat outputmat = pool(inputmat, stride, poolfn);

        print_matrix(outputmat);
        print_matrix_to_file(outputmat, output);
    }
    else if (cmd == "probability") {
        if (argc < 5) goto err;
        string type(argv[2]), input(argv[3]), output(argv[4]);

        vec outputvec;
        vec inputvec = read_vector(input);
        if (type == "softmax") outputvec = normalize_softmax(inputvec);
        else if (type == "sigmoid") outputvec = normalize_sigmoid(inputvec);
        else goto err;

        print_vector(outputvec);
        print_vector_to_file(outputvec, output);
    }
    else goto err;

    return 0;

err:
    cout << "that command does not exist. Type help for info" << endl;
    return 0;
}