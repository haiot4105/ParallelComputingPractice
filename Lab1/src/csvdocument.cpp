#include "csv_document/csvdocument.h"

#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>

std::string replaceAll(std::string str_, const std::string& from_, const std::string& to_) {
    size_t startPos = 0;
    while ((startPos = str_.find(from_, startPos)) != std::string::npos) {
        str_.replace(startPos, from_.length(), to_);
        startPos += to_.length();
    }
    return str_;
}

CsvCell::CsvCell(const std::string& data_) : m_data{data_} { }
CsvCell::CsvCell(const char* data_) : m_data{data_} { }
CsvCell::CsvCell(std::string&& data_) : m_data{std::move(data_)} { }
CsvCell::CsvCell(int value_) : m_data{std::to_string(value_)} { }
CsvCell::CsvCell(long long value_) : m_data{std::to_string(value_)} { }
CsvCell::CsvCell(unsigned long long value_) : m_data{std::to_string(value_)} { }
CsvCell::CsvCell(double value_) : m_data{std::to_string(value_)} { }

const std::string& CsvCell::toString() const {
    return m_data;
}

long long CsvCell::toLongLong(bool& success_) const {
    auto data = m_data.c_str();
    char* end{nullptr};
    auto n = std::strtoll(data, &end, 0);
    success_ = end > data;
    return success_ ? n : 0;
}

long long CsvCell::toLongLong(long long default_) const {
    bool success;
    auto v = toLongLong(success);
    return success ? v : default_;
}

unsigned long long CsvCell::toUnsignedLongLong(bool& success_) const {
    auto data = m_data.c_str();
    char* end{nullptr};
    auto n = std::strtoull(data, &end, 0);
    success_ = end > data;
    return success_ ? n : 0;
}

unsigned long long CsvCell::toUnsignedLongLong(unsigned long long default_) const {
    bool success;
    auto v = toUnsignedLongLong(success);
    return success ? v : default_;
}

double CsvCell::toDouble() const {
    auto data = m_data.c_str();
    char* end{nullptr};
    auto n = std::strtod(data, &end);
    return end > data ? n : std::numeric_limits<double>::quiet_NaN();
}

bool CsvCell::operator==(const CsvCell& other_) const {
    return m_data == other_.m_data;
}

size_t CsvRow::size() const {
    return m_cells.size();
}

void CsvRow::addCell(const CsvCell& cell_) {
    m_cells.emplace_back(cell_);
}

void CsvRow::addCell(CsvCell&& cell_) {
    m_cells.emplace_back(std::move(cell_));
}

CsvCell& CsvRow::operator[](size_t i_) {
    if (i_ >= m_cells.size()) {
        m_cells.resize(i_ + 1);
    }
    return m_cells[i_];
}

const CsvCell& CsvRow::operator[](size_t i_) const {
    return m_cells.at(i_);
}

CsvDocument::CsvDocument(char separator_) {
    setSeparator(separator_);
}

bool CsvDocument::load(const std::string& fileName_) {
    m_rows.clear();
    std::ifstream stream(fileName_);
    if (!stream.is_open()) {
        return false;
    }
    std::string line;
    while (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        CsvRow row;
        while (!lineStream.eof()) {
            char c;
            if (lineStream.peek() == '"') {
                std::ostringstream dataStream;
                lineStream.get(c);
                while (!lineStream.eof()) {
                    if (lineStream.peek() == '"') {
                        lineStream.get(c);
                        if (lineStream.peek() != '"') {
                            if ((lineStream.peek() != m_separator) && !lineStream.eof()) {
                                std::cout << "Parsing error: file '" << fileName_ << "', line "
                                          << m_rows.size() + 1 << ", position "
                                          << static_cast<size_t>(lineStream.tellg()) + 1 << std::endl;
                                continue;
                            } else {
                                lineStream.get(c);
                            }
                            break;
                        }
                    }
                    lineStream.get(c);
                    dataStream << c;
                }
                row.addCell(dataStream.str());
            } else {
                std::string data;
                std::getline(lineStream, data, m_separator);
                row.addCell(data);
            }
        }
        m_rows.emplace_back(std::move(row));
    }
    return true;
}

void CsvDocument::save(const std::string& fileName_) const {
    std::ofstream stream(fileName_);
    if (!stream.is_open()) {
        return;
    }
    for (const auto& row : m_rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            if (i != 0) {
                stream << m_separator;
            }
            auto data = row[i].toString();
            auto quoted = false;
            if ((data.find("\"") != std::string::npos) || (data.find(m_separator) != std::string::npos)) {
                quoted = true;
                data = replaceAll(data, "\"", "\"\"");
                stream << "\"";
            }
            stream << data;
            if (quoted) {
                stream << "\"";
            }
        }
        stream << "\n";
    }
}

void CsvDocument::setSeparator(char separator_) {
    if (separator_ != ',' && separator_ != ';') {
        return;
    }
    m_separator = separator_;
}

CsvRow& CsvDocument::operator[](size_t i_) {
    if (i_ >= m_rows.size()) {
        m_rows.resize(i_ + 1);
    }
    return m_rows[i_];
}

void CsvDocument::print() const {
    for (const auto& row : m_rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            std::cout << row[i].toString() << "\t";
        }
        std::cout << std::endl;
    }
}

size_t CsvDocument::size() const {
    return m_rows.size();
}

std::pair<size_t, size_t> CsvDocument::shape() const {
    size_t maxRowSize{0};
    for (const auto& row : m_rows) {
        maxRowSize = std::max(maxRowSize, row.size());
    }
    return {size(), maxRowSize};
}