#include <cstddef>
#include "Cell.h"
#include <stdexcept>
#include <iostream>
#include <vector>
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


class AAAAA
{
public:
    int value;
    AAAAA(){}
    ~AAAAA(){}

    virtual  AAAAA &operator=(const AAAAA &other)
    {
        std::cout << "operator=" << std::endl;
        this->value = other.value;

        return *this;
    }
};

class BBBB : public AAAAA
{
public:
    int value1;

    BBBB &operator=(const BBBB &other)
    {
        std::cout << "BBBB operator=" << std::endl;
        this->value1 = other.value1;

        return *this;
    }
};
class CCCC : public AAAAA
{
public:
    int value1;

    CCCC &operator=(const CCCC &other) 
    {
        std::cout << "CCCC operator=" << std::endl;
        this->value1 = other.value1;

        return *this;
    }
};

void com(AAAAA *A1, AAAAA *A2)
{
    if (dynamic_cast<BBBB*>(A1)) 
    {
        BBBB *B1 = dynamic_cast<BBBB*>(A1);
        BBBB *B2 = dynamic_cast<BBBB*>(A2);

        *B1 = *B2;

    }
    else if(dynamic_cast<CCCC*>(A1))
    {
        CCCC *C1 = dynamic_cast<CCCC*>(A1);
        CCCC *C2 = dynamic_cast<CCCC*>(A2);

        *C1 = *C2;
    }
}

int main()
{
    int i ;
    for( i = 0; i < 10; i = 100)
    {
        printf("i = %d", i);
    }
    printf("i = %d", i);
    

    return 0;
}
