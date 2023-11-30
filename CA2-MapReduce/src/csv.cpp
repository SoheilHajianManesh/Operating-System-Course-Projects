#include "csv.hpp"
#include "ansi-color.hpp"
#include "consts.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

Csv::Csv(string filePath):filePath(filePath)
{
}


int Csv::readCsv() {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr <<ANSI_RED<< "Error: Could not open file " << ANSI_YEL << filePath << ANSI_RST << endl;
        return -1;
    }
    string line;
    while (getline(file, line)) {
        vector<string> row;
        istringstream iss(line);
        string token;
        while (getline(iss, token, ',')) {
            row.push_back(token);
        }
        table.push_back(row);
    }
    file.close();
    return 0;
}

const Csv::Table& Csv::getTable() const {
    return table;
}
