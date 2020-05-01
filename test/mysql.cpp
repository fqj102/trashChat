#include <iostream>
#include "mysql_test.h"


int main(void)
{
    std::cout << "INIT" << std::endl;
    std::cout << DBSampleQuery(1) << std::endl;
    
    return 0;
}
