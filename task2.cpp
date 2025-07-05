#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <sstream>
#include <chrono>

using namespace std;
using namespace std::chrono;

mutex fileMutex;

// RLE Compression
string compressBlock(const string& data) {
    string result = "";
    int n = data.length();
    for (int i = 0; i < n; ++i) {
        int count = 1;
        while (i + 1 < n && data[i] == data[i + 1]) {
            count++;
            i++;
        }
        result += data[i] + to_string(count);
    }
    return result;
}

// RLE Decompression
string decompressBlock(const string& data) {
    string result = "";
    for (size_t i = 0; i < data.length(); ) {
        char ch = data[i++];
        string countStr = "";
        while (i < data.length() && isdigit(data[i])) {
            countStr += data[i++];
        }
        int count = stoi(countStr);
        result.append(count, ch);
    }
    return result;
}

void compressFile(const string& inputFile, const string& outputFile, int numThreads) {
    ifstream inFile(inputFile);
    ofstream outFile(outputFile);
    vector<string> lines;
    string line;

    while (getline(inFile, line)) {
        lines.push_back(line);
    }

    vector<thread> threads;
    auto compressTask = [&](int start, int end) {
        string localOutput;
        for (int i = start; i < end; ++i) {
            localOutput += compressBlock(lines[i]) + "\n";
        }
        lock_guard<mutex> lock(fileMutex);
        outFile << localOutput;
    };

    int blockSize = lines.size() / numThreads;
    auto startTime = high_resolution_clock::now();

    for (int i = 0; i < numThreads; ++i) {
        int start = i * blockSize;
        int end = (i == numThreads - 1) ? lines.size() : start + blockSize;
        threads.emplace_back(compressTask, start, end);
    }

    for (auto& t : threads) t.join();

    auto endTime = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(endTime - startTime).count();

    cout << "✅ Compression completed in " << duration << " ms using " << numThreads << " threads.\n";
}

void decompressFile(const string& inputFile, const string& outputFile, int numThreads) {
    ifstream inFile(inputFile);
    ofstream outFile(outputFile);
    vector<string> lines;
    string line;

    while (getline(inFile, line)) {
        lines.push_back(line);
    }

    vector<thread> threads;
    auto decompressTask = [&](int start, int end) {
        string localOutput;
        for (int i = start; i < end; ++i) {
            localOutput += decompressBlock(lines[i]) + "\n";
        }
        lock_guard<mutex> lock(fileMutex);
        outFile << localOutput;
    };

    int blockSize = lines.size() / numThreads;
    auto startTime = high_resolution_clock::now();

    for (int i = 0; i < numThreads; ++i) {
        int start = i * blockSize;
        int end = (i == numThreads - 1) ? lines.size() : start + blockSize;
        threads.emplace_back(decompressTask, start, end);
    }

    for (auto& t : threads) t.join();

    auto endTime = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(endTime - startTime).count();

    cout << "✅ Decompression completed in " << duration << " ms using " << numThreads << " threads.\n";
}

int main() {
    string inputFile, outputFile;
    int choice, threads;

    cout << "\n=== Multithreaded File Compression Tool ===\n";
    cout << "1. Compress\n2. Decompress\nEnter your choice: ";
    cin >> choice;

    cout << "Enter input file name: ";
    cin >> inputFile;
    cout << "Enter output file name: ";
    cin >> outputFile;
    cout << "Enter number of threads: ";
    cin >> threads;

    if (choice == 1)
        compressFile(inputFile, outputFile, threads);
    else if (choice == 2)
        decompressFile(inputFile, outputFile, threads);
    else
        cout << "Invalid choice.\n";

    return 0;
}
