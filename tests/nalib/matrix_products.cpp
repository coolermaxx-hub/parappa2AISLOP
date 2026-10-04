#include "nalib/namatrix.h"

// Native product checks; also compile this file with the historical toolchain.
// Its R5900 integer MADD1 and address calculations cannot execute under the
// generic MIPS scalar runner used by matrix_layout.cpp.
static volatile int input = 1;

int main() {
    const int one = input;
    NaMATRIX<int, 2, 2> lhs(one, 2, 3, 4);
    NaMATRIX<int, 2, 2> rhs(5, 6, 7, 8);
    const NaMATRIX<int, 2, 2> product = lhs * rhs;
    if (product[0][0] != 23 || product[0][1] != 34 ||
        product[1][0] != 31 || product[1][1] != 46) return 1;

    NaVECTOR<int, 2> vector(2, 3);
    NaVECTOR<int, 2> applied = lhs * vector;
    if (applied[0] != 11 || applied[1] != 16) return 2;
    if (&NaMATRIX<int, 2, 2>::Apply(vector, lhs, vector) != &vector ||
        vector[0] != 11 || vector[1] != 16) return 3;

    NaMATRIX<int, 2, 2> leftAlias(lhs);
    if (&NaMATRIX<int, 2, 2>::Multiply(leftAlias, leftAlias, rhs) != &leftAlias) return 4;
    NaMATRIX<int, 2, 2> rightAlias(rhs);
    NaMATRIX<int, 2, 2>::Multiply(rightAlias, lhs, rightAlias);
    for (int column = 0; column < 2; column++) {
        if (leftAlias[column].Differs(product[column]) ||
            rightAlias[column].Differs(product[column])) return 5;
    }
    NaMATRIX<int, 2, 2>::Multiply(lhs, lhs, lhs);
    if (lhs[0][0] != 7 || lhs[0][1] != 10 ||
        lhs[1][0] != 15 || lhs[1][1] != 22) return 6;

    const float floatOne = one;
    NaMATRIX<float, 3, 3> matrix(floatOne, 2.0f, 3.0f,
                               4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 10.0f);
    NaVECTOR<float, 3> point(2.0f, -1.0f, 3.0f);
    NaMATRIX<float, 3, 3>::Apply(point, matrix, point);
    if (point[0] != 19.0f || point[1] != 23.0f || point[2] != 30.0f) return 7;
    NaMATRIX<float, 3, 3> permutation(0.0f, 0.0f, 1.0f,
                                    1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    NaMATRIX<float, 3, 3> permuted = matrix * permutation;
    if (permuted[0].Differs(matrix[2]) || permuted[1].Differs(matrix[0]) ||
        permuted[2].Differs(matrix[1])) return 8;

    // Four components do not imply float storage or the VU backend.
    NaMATRIX<int, 4, 4> integers(one, 2, 3, 4, 5, 6, 7, 8,
                                9, 10, 11, 12, 13, 14, 15, 16);
    NaVECTOR<int, 4> weights(2, -1, 0, 3);
    NaVECTOR<int, 4> weighted = integers * weights;
    if (weighted[0] != 36 || weighted[1] != 40 ||
        weighted[2] != 44 || weighted[3] != 48) return 9;

    // Rectangular storage uses rows for the output and columns for the input.
    NaMATRIX<int, 1, 4> row(one, 2, 3, 4);
    NaVECTOR<int, 1> scalar = row * weights;
    if (scalar[0] != 12) return 10;
    NaMATRIX<int, 1, 4> rowProduct = row * integers;
    if (rowProduct[0][0] != 30 || rowProduct[1][0] != 70 ||
        rowProduct[2][0] != 110 || rowProduct[3][0] != 150) return 11;
    NaMATRIX<int, 1, 4>::Multiply(row, row, integers);
    for (int column = 0; column < 4; column++) {
        if (row[column].Differs(rowProduct[column])) return 12;
    }
    NaMATRIX<int, 4, 1> column(one, 2, 3, 4);
    scalar[0] = 3;
    weighted = column * scalar;
    if (weighted[0] != 3 || weighted[1] != 6 ||
        weighted[2] != 9 || weighted[3] != 12) return 13;
    return 0;
}
