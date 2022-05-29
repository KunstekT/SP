#include<math.h>
#include<stdexcept>
#include<time.h>
#include<Windows.h>
#include<process.h>
#include<iostream>


constexpr unsigned int MATRIX = 1032;
constexpr unsigned int THREADS = 1;

float getValue(int i, int j) {
    float value = 0;
    for (int k = 0; k <= i; k++) {
        value += k * sin(j) - j * cos(k);
    }
    return value;
}

float getSequential() {

    int** matrix = new int* [MATRIX];
    clock_t t1;
    clock_t t2;
    std::cout << "Sequential" << std::endl;
    t1 = clock();
    for (int i = 0; i < MATRIX; ++i) {
        matrix[i] = new int[MATRIX];
    }
    for (int row = 0; row < MATRIX; row++) {
        for (int col = 0; col < MATRIX; col++) {
            matrix[row][col] = getValue(row, col);
        }
    }
    t2 = clock();
    std::cout << "Time: " << t2 - t1 << " ms" << std::endl;
    float sum = 0;
    for (int row = 0; row < MATRIX; row++) {
        for (int col = 0; col < MATRIX; col++) {
            sum += matrix[row][col];
        }
    }
    std::cout << "Sum: " << sum << std::endl;
    delete[] matrix;
    return sum;
}

class Matrix {
private:
    unsigned _rows;
    unsigned _columns;
    float* _data;

public:
    Matrix(unsigned int rows, unsigned int columns) {
        _rows = rows;
        _columns = columns;
        if (rows == 0 || columns == 0) {
            throw std::invalid_argument("Rows and columns cannot be 0 or less.");
        }
        _data = new float[rows * columns];
    }

    ~Matrix() {
        delete[] _data;
    }

    float& operator() (unsigned row, unsigned column) {
        if (row >= _rows || column >= _columns) {
            throw std::out_of_range("Index out of range.");
        }
        return _data[_columns * row + column];
    }

    float operator() (unsigned row, unsigned col) const {
        if (row >= _rows || col >= _columns) {
            throw std::out_of_range("Index out of range.");
        }
        return _data[_columns * row + col];
    }
};


struct THREAD_PARAMS {
    Matrix* matrixPtr;
    unsigned int startRow;
    unsigned int rowCount;
    unsigned int columnCount;
};

unsigned __stdcall WorkerThread(PVOID param) {
    THREAD_PARAMS* params = (THREAD_PARAMS*)param;
    unsigned int row = params->startRow;
    unsigned int column = 0;
    for (row; row < params->startRow + params->rowCount; row++) {
        for (column = 0; column < params->columnCount; column++) {
            params->matrixPtr->operator()(row, column) = getValue(row, column);
        }
    }
    _endthreadex(0);
    return 0;
}



int main(int argc, char* argv[]) {
    //we can call getSequential() method if we want sequentially fill matrix

    clock_t t1;
    clock_t t2;
    t1 = clock();
    float sum = getSequential();
    t2 = clock();
    Matrix mat = Matrix(MATRIX, MATRIX);
    clock_t t3;
    clock_t t4;
    std::cout << "With threads" << std::endl;
    t3 = clock();

    HANDLE* hWork = new HANDLE[THREADS];
    THREAD_PARAMS* params = new THREAD_PARAMS[THREADS];
    unsigned int threadCounter = 0;
    unsigned int rowsPerThread = MATRIX / THREADS;
    for (threadCounter; threadCounter < THREADS; threadCounter++) {
        params[threadCounter] = THREAD_PARAMS{
            &mat,
            threadCounter * rowsPerThread,
            rowsPerThread,
            MATRIX
        };
        hWork[threadCounter] = (HANDLE)_beginthreadex(
            NULL,
            0,
            WorkerThread,
            &params[threadCounter],
            0,
            NULL
        );
    }

    WaitForMultipleObjects(
        THREADS,
        hWork,
        true,
        INFINITE
    );

    t4 = clock();
    std::cout << "Time: " << t4 - t3 << " ms" << std::endl;
    std::cout << "Omjer: " << (double)(t2 - t1) / (t4 - t3) << std::endl;


    float sum2 = 0;
    int row, column = 0;
    for (row = 0; row < MATRIX; row++) {
        for (column = 0; column < MATRIX; column++) {
            sum2 += mat(row, column);
        }
    }
    std::cout << "Sum: " << sum2 << std::endl;
    float tocnost = sum / sum2;
    if (tocnost < 0)
    {
        tocnost = sum2 / sum;
    }
    std::cout << "Tocnost[%]: " << abs(tocnost * 100) << "%" << std::endl;
    return 0;
}