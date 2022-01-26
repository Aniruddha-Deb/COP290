#include "matio.hpp"
#include "nnmat.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

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

vec read_vector(string filename) {
    string line;

    cout << "reading matrix" << endl;

    ifstream source; 
    source.open(filename,ios::in);
    if (source) {
        stringstream buffer;
        buffer << source.rdbuf();
        int n;
        buffer >> n;
        vec V(n);
        for (int i=0; i<n; i++) {
            buffer >> V[i];
            cout << "read " << V[i] << endl;
        }
        cout << "Read vector" << endl;
        source.close();
        return V;
    }

    vec K;
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

void print_vector(vec& vector) {
    for (auto i : vector) {
        cout << i << " ";
    }
    cout << endl;
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

void print_vector_to_file(vec& vector, string filename) {

    ofstream out;
    out.open(filename, ios::out);
    if (out) {
        out << vector.size() << endl;
        for (int i=0; i<vector.size(); i++) {
            out << vector[i] << endl;
        }
    }
}