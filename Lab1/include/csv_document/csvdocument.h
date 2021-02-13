#ifndef CSVDOCUMENT_H
#define CSVDOCUMENT_H

#include <string>
#include <vector>

class CsvCell {
    public:
        CsvCell() = default;
        CsvCell(const CsvCell&) = default;
        CsvCell(CsvCell&&) = default;
        CsvCell& operator=(const CsvCell&) = default;
        CsvCell& operator=(CsvCell&&) = default;
        ~CsvCell() = default;

        CsvCell(const std::string& data_);
        CsvCell(const char* data_);
        CsvCell(std::string&& data_);
        CsvCell(int value_);
        CsvCell(long long value_);
        CsvCell(unsigned long long value_);
        CsvCell(double value_);

        const std::string& toString() const;
        long long toLongLong(bool& success_) const;
        long long toLongLong(long long default_ = 0) const;
        unsigned long long toUnsignedLongLong(bool& success_) const;
        unsigned long long toUnsignedLongLong(unsigned long long default_ = 0) const;
        double toDouble() const;

        bool operator==(const CsvCell&) const;
    private:
        std::string m_data;
};

class CsvRow {
    public:
        CsvRow() = default;
        CsvRow(const CsvRow&) = default;
        CsvRow(CsvRow&&) = default;
        CsvRow& operator=(const CsvRow&) = default;
        CsvRow& operator=(CsvRow&&) = default;
        ~CsvRow() = default;

        size_t size() const;
        void addCell(const CsvCell& cell_);
        void addCell(CsvCell&& cell_);
        CsvCell& operator[](size_t i_);
        const CsvCell& operator[](size_t i_) const;
    private:
        std::vector<CsvCell> m_cells;
};

class CsvDocument {
    public:
        CsvDocument() = default;
        explicit CsvDocument(char separator_);
        bool load(const std::string& fileName_);
        void save(const std::string& fileName_) const;
        void setSeparator(char separator_);
        CsvRow& operator[](size_t i_);
        void print() const;
        size_t size() const;
        std::pair<size_t, size_t> shape() const;
    private:
        std::vector<CsvRow> m_rows;
        char m_separator{','};
};

#endif // CSVDOCUMENT_H