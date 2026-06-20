#pragma once

#include <iostream>
#include <cstdlib>

#ifdef NDEBUG                                                                                                                                                                                        
    #define ASSERT(cond, msg) ((void)0)                                                                                                                                                                
#else
    #define ASSERT(cond, msg) \
        do { \
            if (!(cond)) { \
                std::cerr \
                    << "Assertion failed: " << msg \
                    << " (File: " << __FILE__ \
                    << ", Line: " << __LINE__ << ")" \
                    << std::endl; \
                std::abort(); \
            } \
        } while (0)                                                                                                                                                                                       
#endif