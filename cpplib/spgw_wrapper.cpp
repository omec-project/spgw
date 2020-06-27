extern "C"
{
    #include <stdio.h>
    #include "spgw_cpp_wrapper.h"
    int parse_config_wrapper(void)
    {
        printf("parse_config called in the library \n");
        return 0;
    }
}
