#if !defined(SIMPLEWEB_ERROR_INCLUDED)
#define SIMPLEWEB_ERROR_INCLUDED


#include <stdio.h>
#include <stdlib.h>


namespace simpleweb {


void error_exit(const char *s)
{
    perror(s);
    exit(1);
}

    
} // namespace simpleweb


#endif // SIMPLEWEB_ERROR_INCLUDED
