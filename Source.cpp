#include <iostream>
#include <fstream>
#include <vector>

// thread management
#include <thread>
#include <mutex>

// time tracking
#include <chrono>

using namespace std;
using namespace std::chrono;

static mutex barrier;

#define MAX_SIZE 1000
#define MAX_THREAD 25

/* Fills the matix with random number or with zeros
* Inputs:
* @Mat: the matrix to fill
* @rows: number of rows
* @cols: number of columns
* @random: matrix to fill with random values or not
            if true: fill with random number between 1-2000 (default)
            if false: fill with all zeros
***********************************************************/
void initializeM(vector<vector<int>> &Mat, int rows, int cols, bool random = true)
{
    vector<int> row;
    for (int i = 0; i < rows; i++) {
        row.clear();
        for (int j = 0; j < cols; j++) {
            if (random)
                row.push_back(rand() % 2000); // Rnadom from 1-2000
            else
                row.push_back(0);
        }
        Mat.push_back(row);
    }
}


/* Prints the matix
* Inputs:
* @Mat: the matrix to fill
* @rows: number of rows
* @cols: number of columns
***********************************************************/
void printM(vector<vector<int>> &Mat, int rows, int cols)
{
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++)
            cout << Mat[i][j] << " ";
        cout << endl;
    }
    cout << endl;
}


/* Writes the matix to a file
* Inputs:
* @Mat: the matrix to fill
* @rows: number of rows
* @cols: number of columns
* @output: reference to output file where the matrix is dumped
***********************************************************/
void writeM(vector<vector<int>> &Mat, int rows, int cols, ofstream & output)
{
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++)
            output << Mat[i][j] << " ";
        output << endl;
    }
    output << endl;
}


/* Compare the numbers of two files(matrices) one-by-one
* Inputs:
* @file1: first file stream
* @file2: second file stream
* @return: true, if file content is same, return false
***********************************************************/
bool compareFiles(ifstream &file1, ifstream &file2)
{
    int fromFirst, fromSecond;
    bool isEqual = true;
    file1.in; file2.in;
    if (file1.is_open() && file2.is_open()) {
        while (file1 >> fromFirst && file2 >> fromSecond) { // only non-zeros
            if (fromFirst != fromSecond) {
                isEqual = false;
                break;
            }
        }
    }
    return isEqual;
}


/* Multiply two matrices(matrix01, matrix02)
* Inputs:
* @matrix01: first matrix
* @matrix02: second matrix
* @rowStart, rowEnd: range of the rows to be considered
* @colStart, colEnd: range of the columns to be considered
* @resultant: output matrix reference
************************************************************/
void multiplyM(vector<vector<int>> &matrix01,
    vector<vector<int>> &matrix02,
    int rowStart, int rowEnd,
    int colStart, int colEnd,
    vector<vector<int>> &resultant)
{
    for (int i = rowStart; i < rowEnd; i++)
        for (int j = colStart; j < colEnd; j++) {
            int res = 0;
            for (int k = 0; k < MAX_SIZE; k++) {
                res += matrix01[i][k] * matrix02[k][j];
            }
            resultant[i][j] = res;
        }
    //this_thread::sleep_for(10ms);

    // block other threads until this finishes
    lock_guard<mutex> blockthreads(barrier);
}


/*
1. Initialize matrix01 and matrix02 with random values between 1-2000
2. Dump values to matrix01.txt and matrix02
3. Create outputlog.txt to dump process times
4. Process regular multiplication. Dump result to withoutmt.txt.
5. Process multithreading multiplication. Dump result to withmt.txt.
6. Compare two results. Return true or false
*/
int main()
{
    vector<vector<int>> matrix01, matrix02, resultMT, resultNoMT;

    //*********************************************
    // MATRICES INITIALIZATION
    //*********************************************
    initializeM(matrix01, MAX_SIZE, MAX_SIZE); // random fill
    initializeM(matrix02, MAX_SIZE, MAX_SIZE); // random fill

    // resultNoMT: result matrix without multithreading
    initializeM(resultNoMT, MAX_SIZE, MAX_SIZE, false); // all zeros

    // resultMT: result matrix using multithreading
    initializeM(resultMT, MAX_SIZE, MAX_SIZE, false); // all zeros

    // Dump the matrix01 and matrix02 in file
    ofstream file;
    file.open("matrix01.txt");
    writeM(matrix01, MAX_SIZE, MAX_SIZE, file);
    file.close();
    //printM(matrix01, MAX_SIZE, MAX_SIZE);

    file.open("matrix02.txt");
    writeM(matrix02, MAX_SIZE, MAX_SIZE, file);
    file.close();
    //printM(matrix02, MAX_SIZE, MAX_SIZE);

    ofstream outputlog;
    outputlog.open("outputlog.txt");
    outputlog << "MATRIX SIZE: " << MAX_SIZE << endl;
    outputlog << "NUMBER OF THREADS: " << MAX_THREAD << endl;

    //*********************************************
    // MATRIX MULTIPLICATION: REGULAR
    //*********************************************
    outputlog << endl << "MATRIX MULTIPLICATION: REGULAR" << endl;
    auto starttime = system_clock::to_time_t(system_clock::now());
    //outputlog << "Started at: " << starttime << endl;

    // Regular matrix multiplication
    multiplyM(matrix01, matrix02, 0, MAX_SIZE, 0, MAX_SIZE, resultNoMT);

    auto endtime = system_clock::to_time_t(system_clock::now());
    //outputlog << "Finished at: " << endtime << endl;

    outputlog << "Time Taken (in seconds): " << (endtime - starttime) << endl;
    // Note: Printing takes time, DO NOT call it before time logging
    //printM(resultant, MAX_SIZE, MAX_SIZE);

    // Dump product to withoutMT.txt
    ofstream outFileMT, outFileNoMT;
    outFileNoMT.open("withoutmt.txt");
    writeM(resultNoMT, MAX_SIZE, MAX_SIZE, outFileNoMT);
    outFileNoMT.close();


    //*********************************************
    // MATRIX MULTIPLICATION: USING MULTITHREADING
    //*********************************************
    vector<thread> threads;
    int batch = MAX_SIZE / MAX_THREAD;
    int rowStart = 0, rowEnd = 0;
    int colStart = 0, colEnd = MAX_SIZE;

    outputlog << endl << "MATRIX MULTIPLICATION: USING MULTITHREADING" << endl;
    starttime = system_clock::to_time_t(system_clock::now());
    //outputlog << "Started at: " << starttime << endl;

    // thread creation
    for (int i = 0; i < MAX_THREAD; i++) {
        rowStart = i * batch;
        rowEnd = rowStart + batch;
        threads.push_back(thread(multiplyM, ref(matrix01), ref(matrix02),
            rowStart, rowEnd, colStart, colEnd, ref(resultMT)));
    }
    // thread wait
    for (auto &th : threads) th.join();

    endtime = system_clock::to_time_t(system_clock::now());
    outputlog << "Time Taken (in seconds): " << (endtime - starttime) << endl;
    //outputlog << "Finished at: " << endtime << endl;
    // Note: Printing takes time, DO NOT call it before time logging
    //printM(resultant, MAX_SIZE, MAX_SIZE);

    outFileMT.open("withmt.txt");
    writeM(resultMT, MAX_SIZE, MAX_SIZE, outFileMT);
    outFileMT.close();



    //*********************************************
    // COMPARE TWO RESULTS
    //*********************************************
    ifstream inFileMT, inFileNoMT;
    inFileMT.open("withmt.txt");
    inFileNoMT.open("withoutmt.txt");
    outputlog << endl << "Result comparison (1/0): ";
    outputlog << compareFiles(inFileMT, inFileNoMT); // method call
    inFileMT.close();
    inFileNoMT.close();
    outputlog.close();

    return 0;
}