// a test for get_ASA_prog_cmd
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "test_revprog.h"
#include "../getASAcmd.h"

// uint8_t test_cmd[265];

int main() {
    printf("Start\n");
    uint8_t chksum = 0;
    uint16_t databytes = 256;
    test_cmd[5] = databytes>>8;
    test_cmd[6] = databytes;
    for (uint16_t i = 0; i < databytes; i++) {
        test_cmd[i+7] = i;
        chksum += i;
    }
    test_cmd[7+databytes+1] = chksum;

    uint8_t buf[256];
    uint16_t bytes;
    uint8_t res = 0;
    res = get_ASA_prog_cmd(buf,&bytes);
    printf("res = %d ,bytes = %d\n", res, bytes);

    for (uint16_t i = 0; i < bytes; i++) {
        printf("%3d ", buf[i]);
        if (!((i+1)%16)) {
            printf("\n");
        }
    }
    return 0;
}
