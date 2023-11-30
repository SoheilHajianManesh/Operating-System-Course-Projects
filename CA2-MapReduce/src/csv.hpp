#ifndef CSV_HPP
#define CSV_HPP

#include <string>
#include <vector>


class Csv {
public:
    using Table = std::vector<std::vector<std::string>>;

    Csv(std::string filePath);

    int readCsv();
    const Table& getTable() const;

private:
    Table table;
    std::string filePath;
};

#endif