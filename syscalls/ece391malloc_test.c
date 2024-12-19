#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024

int main ()
{
    int32_t *ptr;
    int32_t cnt;
    uint8_t buf[BUFSIZE];

    ece391_fdputs(1, (uint8_t*)"Starting malloc_test\n");

    ece391_fdputs(1, (uint8_t*)"Enter the Test Number: (0): 4, (1): 32768, (2): 4194304\n");
    if (-1 == (cnt = ece391_read(0, buf, BUFSIZE-1)) ) {
        ece391_fdputs(1, (uint8_t*)"Can't read the number from keyboard.\n");
     return 3;
    }
    buf[cnt] = '\0';

    if ((ece391_strlen(buf) > 2) || ((ece391_strlen(buf) == 2) && ((buf[0] < '0') || (buf[0] > '2')))) {
        ece391_fdputs(1, (uint8_t*)"Wrong Choice!\n");
        return 0;
    } else {
        switch (buf[0]) {
	        case '0':
                if ( (ptr = (int32_t*)ece391_malloc(4)) != (int32_t*)0 ) {
                    *ptr = 1;
                    ece391_fdputs(1, (uint8_t*)"Malloc passed! (4) - ");
                    ece391_itoa(*ptr, buf, 10);
                    ece391_fdputs(1, buf);
                    ece391_fdputs(1, (uint8_t*)"\n");
                    return 0;
                }
                break;
            case '1':
                if ( (ptr = (int32_t*)ece391_malloc(32768)) != (int32_t*)0 ) {
                    *ptr = 2;
                    ece391_fdputs(1, (uint8_t*)"Malloc passed! (32768) - ");
                    ece391_itoa(*ptr, buf, 10);
                    ece391_fdputs(1, buf);
                    ece391_fdputs(1, (uint8_t*)"\n");
                    return 0;
                }
                break;
            case '2':
                if ( (ptr = (int32_t*)ece391_malloc(4194304)) != (int32_t*)0 ) {
                    *ptr = 3;
                    ece391_fdputs(1, (uint8_t*)"Malloc passed! (4194304) - ");
                    ece391_itoa(*ptr, buf, 10);
                    ece391_fdputs(1, buf);
                    ece391_fdputs(1, (uint8_t*)"\n");
                    return 0;
                }
                break;
        }
        ece391_fdputs(1, (uint8_t*)"Malloc failed!\n");
        return 1;
    }
}
