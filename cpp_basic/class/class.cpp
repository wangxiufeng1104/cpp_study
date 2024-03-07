#include <cstddef>
#include "Cell.h"
#include <stdexcept>
#include <iostream>
class Spreadsheet
{
private:
    bool inRange(size_t value, size_t upper) const;

    size_t m_height {0};
    size_t m_width {0};
    Cell **m_cells {nullptr};
public:
    Spreadsheet(size_t width, size_t height) :
    m_width{width}, m_height{height}
    {
        std::cout << "start new cells" << std::endl;
        m_cells = new Cell*[m_width];
        for(size_t i {0}; i < m_width; i++){
            m_cells[i] = new Cell[m_height];
        }
    }
    void verifyCoordinate(size_t x, size_t y) const{
        if(x >= m_width){
            throw std::out_of_range{"must be less than "};
        }
        if(y >= m_height){
            throw std::out_of_range{"must be less than "};
        }
    }
    void setCellAt(size_t x, size_t y, const Cell& cell){
        verifyCoordinate(x, y);
        m_cells[x][y] = cell;
    }
    Cell& getCellAt(size_t x, size_t y){
        verifyCoordinate(x, y);
        return m_cells[x][y];
    }
    
    ~Spreadsheet(){
        std::cout << "start delete cells" << std::endl;
        for(size_t i {0}; i < m_width; i++){
            delete[] m_cells[i];
        }
        delete [] m_cells;
        m_cells = nullptr;
    }
};


int main()
{
    Spreadsheet s1{2,3}, s2{4,3};
    //s2 = s1;
    return 0;
}