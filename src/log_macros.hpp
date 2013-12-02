#ifdef __LOG__
#include <iostream>
#define INFO(x)        do {std::cout << x;} while(0)
#define INFO_LINE(x)   do {std::cout << x << std::endl;} while(0)
#else 
#define INFO(x)        do {} while(0) 
#define INFO_LINE(x)   do {} while(0) 
#endif
