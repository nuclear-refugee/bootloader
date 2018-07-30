#include "revlib/rev_DEVICE.h"
#include "revlib/rev_serial.h"
#include "revlib/rev_spi.h"
#include <avr/boot.h>
#include "decode_msg.h"
#include "serial.h"
#include "stk500.h"
#include "stk500_command.h"
#include "asaprog.h"
#include "revprog.h"

#define SPM_PAGESIZE 256

uint8_t do_asa_prog();

Addres_t CurAddres; ///< Recording the current memory location to write.
// uint8_t Sequence = 0; ///< Recording the current sequence of STK500 packet.
void delay_ms(uint16_t t) {
    for (uint16_t i = 0; i < t; i++) {
        _delay_ms(1);
    }
}

int main(void) {
    serial_init();

    uint8_t spires[4]; ///< 接收下達給待燒錄裝置的4byte spi命令
    uint8_t res = 0;

    do {
        res = get_msg();
    } while( res == RES_ERROR);

    if (res == RES_ASAPROG) {
        // res = do_asa_prog();
        __asm__ ("nop");
    } else if (res == RES_STK500) {
        __asm__ ("nop");
        do {
            __asm__ ("nop");
            // Start switch STK command ------------------------------------
            // ***[ STK general command constants ]********************************
            switch (MsgGet.data[0]) {
                case CMD_SIGN_ON: {
                    MsgRes.bytes = 11;
                    MsgRes.data[0] = CMD_SIGN_ON;
                    MsgRes.data[1] = STATUS_CMD_OK;
                    MsgRes.data[2] = 8;
                    MsgRes.data[3] = 'A';
                    MsgRes.data[4] = 'V';
                    MsgRes.data[5] = 'R';
                    MsgRes.data[6] = 'I';
                    MsgRes.data[7] = 'S';
                    MsgRes.data[8] = 'P';
                    MsgRes.data[9] = '_';
                    MsgRes.data[10] = '2';
                    put_msg_in_stk500(&MsgRes);
                    break;
                }
                case CMD_SET_PARAMETER: {
                    uint8_t id = MsgGet.data[1];
                    uint8_t value = MsgGet.data[2];
                    res = isp_set_param(id,value);
                    if(res) {
                        MsgRes.bytes = 2;
                        MsgRes.data[0] = CMD_SET_PARAMETER;
                        MsgRes.data[1] = STATUS_CMD_FAILED;
                        put_msg_in_stk500(&MsgRes);
                    } else {
                        MsgRes.bytes = 2;
                        MsgRes.data[0] = CMD_SET_PARAMETER;
                        MsgRes.data[1] = STATUS_CMD_OK;
                        put_msg_in_stk500(&MsgRes);
                    }
                    break;
                }
                case CMD_GET_PARAMETER: {
                    uint8_t id = MsgGet.data[1];
                    uint8_t value = 1;
                    res = isp_get_param(id, &value);
                    if(res) {
                        MsgRes.bytes = 2;
                        MsgRes.data[0] = CMD_GET_PARAMETER;
                        MsgRes.data[1] = STATUS_CMD_FAILED;
                        put_msg_in_stk500(&MsgRes);
                    } else {
                        MsgRes.bytes = 3;
                        MsgRes.data[0] = CMD_GET_PARAMETER;
                        MsgRes.data[1] = STATUS_CMD_OK;
                        MsgRes.data[2] = value;
                        put_msg_in_stk500(&MsgRes);
                    }
                    break;
                }
                case CMD_OSCCAL: {
                    // NOTE not implement
                    break;
                }
                case CMD_SET_DEVICE_PARAMETERS: {
                    // NOTE not implement
                    break;
                }
                case CMD_LOAD_ADDRESS: {
                    // MSB first
                    CurAddres.ui8[0] = MsgGet.data[4];
                    CurAddres.ui8[1] = MsgGet.data[3];
                    CurAddres.ui8[2] = MsgGet.data[2];
                    CurAddres.ui8[3] = MsgGet.data[1];
                    MsgRes.bytes = 2;
                    MsgRes.data[0] = CMD_LOAD_ADDRESS;
                    MsgRes.data[1] = STATUS_CMD_OK;
                    put_msg_in_stk500(&MsgRes);
                    break;
                }
                case CMD_FIRMWARE_UPGRADE: {
                    // NOTE not implement
                    break;
                }
                // ***[ STK ISP command constants ]********************************
                case CMD_ENTER_PROGMODE_ISP: {
                    // 0 Command ID
                    // 1 timeout
                    // 2 stabDelay
                    // 3 cmdexeDelay
                    // 4 synchLoops
                    // 5 byteDelay
                    // 6 pollValue
                    // 7 pollIndex
                    // 8 cmd1
                    // 9 cmd2
                    // 10 cmd3
                    // 11 cmd4
                    isp_enter_progmode();
                    spires[0] = spi_swap(MsgGet.data[ 8]);
                    spires[1] = spi_swap(MsgGet.data[ 9]);
                    spires[2] = spi_swap(MsgGet.data[10]);
                    spires[3] = spi_swap(MsgGet.data[11]);
                    MsgRes.data[0] = CMD_ENTER_PROGMODE_ISP;
                    _delay_ms(1000);
                    // if (MsgGet.data[6]!=spires[MsgGet.data[7]]) {
                    //     MsgRes.bytes = 2;
                    //     MsgRes.data[1] = STATUS_CMD_FAILED;
                    //     put_msg_in_stk500(&MsgRes);
                    // } else if (0) {
                    //     // TODO timeout
                    //     MsgRes.data[1] = STATUS_CMD_TOUT;
                    // } else {
                    MsgRes.bytes = 2;
                    MsgRes.data[1] = STATUS_CMD_OK;
                    put_msg_in_stk500(&MsgRes);
                    // }
                    break;
                }
                case CMD_LEAVE_PROGMODE_ISP: {
                    // 0 Command ID
                    // 1 preDelay
                    // 2 postDelay

                    // _delay_ms(MsgGet.data[1]);
                    isp_leave_progmode();
                    // _delay_ms(MsgGet.data[2]);
                    MsgRes.bytes = 2;
                    MsgRes.data[0] = CMD_LEAVE_PROGMODE_ISP;
                    MsgRes.data[1] = STATUS_CMD_OK;
                    put_msg_in_stk500(&MsgRes);
                    break;
                }
                case CMD_CHIP_ERASE_ISP: {
                    // 0 Command ID
                    // 1 eraseDelay
                    // 2 pollMethod
                    // 3 cmd1
                    // 4 cmd2
                    // 5 cmd3
                    // 6 cmd4
                    spi_swap(MsgGet.data[3]);
                    spi_swap(MsgGet.data[4]);
                    spi_swap(MsgGet.data[5]);
                    spi_swap(MsgGet.data[6]);

                    // TODO RDY/BSY command or delay
                    delay_ms(MsgGet.data[1]);

                    MsgRes.bytes = 2;
                    MsgRes.data[0] = CMD_CHIP_ERASE_ISP;
                    MsgRes.data[1] = STATUS_CMD_OK;
                    put_msg_in_stk500(&MsgRes);

                    break;
                }
                case CMD_PROGRAM_FLASH_ISP: {
                    // 0 Command ID
                    // 1 NumBytes 15:8
                    // 2 NumBytes  7:0
                    // 3 mode
                    // 4 delay
                    // 5 cmd1
                    // 6 cmd2
                    // 7 cmd3
                    // 8 poll1
                    // 9 poll2
                    // 10 ~ 10+N-1 Data (N bytes)

                    // mode A1 --> 1010 0001
                    // Page Mode
                    // Value polling
                    // Write page
                    uint16_t N = (MsgGet.data[1]<<8) + MsgGet.data[2];
                    Addres_t tmpadd = CurAddres;
                    // _delay_ms(10);
                    for (uint16_t i = 0; i < N; i++) {
                        isp_load_flash(i&1, 0, CurAddres.ui8[0], MsgGet.data[10+i]);
                        if (i&1) {
                            CurAddres.ui32++;
                        }
                    }
                    isp_write_flash(tmpadd.ui8[1], tmpadd.ui8[0]);
                    // _delay_ms(10);
                    // _delay_ms(MsgGet.data[4]);

                    MsgRes.bytes = 2;
                    MsgRes.data[0] = CMD_PROGRAM_FLASH_ISP;
                    MsgRes.data[1] = STATUS_CMD_OK;
                    put_msg_in_stk500(&MsgRes);
                    break;
                }
                case CMD_READ_FLASH_ISP: {
                    // 0 Command ID
                    // 1 NumBytes 15:8
                    // 2 NumBytes  7:0
                    // 3 cmd1

                    uint16_t N = (MsgGet.data[1]<<8) + MsgGet.data[2];
                    for (uint16_t i = 0; i < N; i++) {
                        spi_swap(MsgGet.data[3] | (i&1)<<3 );
                        spi_swap(CurAddres.ui8[1]);
                        spi_swap(CurAddres.ui8[0]);
                        MsgRes.data[2+i] = spi_swap(0x00);
                        if (i&1) {
                            CurAddres.ui32++;
                        }
                        // _delay_ms(1);
                    }
                    MsgRes.data[0] = CMD_READ_FLASH_ISP;
                    if ( 0 ) {
                        MsgRes.bytes = 2;
                        MsgRes.data[1] = STATUS_CMD_FAILED;
                        put_msg_in_stk500(&MsgRes);
                    } else {
                        MsgRes.bytes = 3+N;
                        MsgRes.data[1] = STATUS_CMD_OK;
                        MsgRes.data[2+N] = STATUS_CMD_OK;
                        put_msg_in_stk500(&MsgRes);
                    }
                    break;
                }
                case CMD_PROGRAM_EEPROM_ISP: {
                    // 0 Command ID
                    // 1 NumBytes 15:8
                    // 2 NumBytes  7:0
                    // 3 mode
                    // 4 delay
                    // 5 cmd1
                    // 6 cmd2
                    // 7 cmd3
                    // 8 poll1
                    // 9 poll2
                    // 10 ~ 10+N-1 Data (N bytes)

                    uint16_t N = (MsgGet.data[1]<<8) + MsgGet.data[2];
                    // _delay_ms(10);
                    for (uint16_t i = 0; i < N; i++) {
                        isp_write_eeprom(CurAddres.ui8[1], CurAddres.ui8[0], MsgGet.data[10+i]);
                        CurAddres.ui32++;
                        // _delay_ms(MsgGet.data[4]);
                    }

                    MsgRes.bytes = 2;
                    MsgRes.data[0] = CMD_PROGRAM_EEPROM_ISP;
                    MsgRes.data[1] = STATUS_CMD_OK;
                    put_msg_in_stk500(&MsgRes);
                    break;
                }
                case CMD_READ_EEPROM_ISP: {
                    // 0 Command ID
                    // 1 NumBytes 15:8
                    // 2 NumBytes  7:0
                    // 3 cmd1

                    uint16_t N = (MsgGet.data[1]<<8) + MsgGet.data[2];
                    for (uint16_t i = 0; i < N; i++) {
                        spi_swap(MsgGet.data[3]);
                        spi_swap(CurAddres.ui8[1]);
                        spi_swap(CurAddres.ui8[0]);
                        MsgRes.data[2+i] = spi_swap(0x00);
                        CurAddres.ui32++;
                        // _delay_ms(1);
                    }
                    MsgRes.data[0] = CMD_READ_EEPROM_ISP;
                    if ( 0 ) {
                        MsgRes.bytes = 2;
                        MsgRes.data[1] = STATUS_CMD_FAILED;
                        put_msg_in_stk500(&MsgRes);
                    } else {
                        MsgRes.bytes = 3+N;
                        MsgRes.data[1] = STATUS_CMD_OK;
                        MsgRes.data[2+N] = STATUS_CMD_OK;
                        put_msg_in_stk500(&MsgRes);
                    }
                    break;
                }
                case CMD_PROGRAM_FUSE_ISP:
                case CMD_PROGRAM_LOCK_ISP: {
                    // 0 Command ID
                    // 1 cmd1
                    // 2 cmd2
                    // 3 cmd3
                    // 4 cmd4
                    spi_swap(MsgGet.data[1]);
                    spi_swap(MsgGet.data[2]);
                    spi_swap(MsgGet.data[3]);
                    spi_swap(MsgGet.data[4]);
                    MsgRes.bytes = 3;
                    MsgRes.data[0] = CMD_READ_OSCCAL_ISP;
                    MsgRes.data[1] = STATUS_CMD_OK;
                    MsgRes.data[2] = STATUS_CMD_OK;
                    put_msg_in_stk500(&MsgRes);
                    break;
                }
                case CMD_READ_FUSE_ISP:
                case CMD_READ_LOCK_ISP:
                case CMD_READ_SIGNATURE_ISP:
                case CMD_READ_OSCCAL_ISP: {
                    // 0 Command ID
                    // 1 RetAddr
                    // 2 cmd1
                    // 3 cmd2
                    // 4 cmd3
                    // 5 cmd4
                    spi_swap(MsgGet.data[2]);
                    spi_swap(MsgGet.data[3]);
                    spi_swap(MsgGet.data[4]);
                    res = spi_swap(MsgGet.data[5]);
                    MsgRes.bytes = 4;
                    MsgRes.data[0] = CMD_READ_OSCCAL_ISP;
                    MsgRes.data[1] = STATUS_CMD_OK;
                    MsgRes.data[2] = res;
                    MsgRes.data[3] = STATUS_CMD_OK;
                    put_msg_in_stk500(&MsgRes);
                    break;
                }
                case CMD_SPI_MULTI: {
                    // 0 Command ID
                    // 1 NumTx
                    // 2 NumRx
                    // 3 RxStartAddr
                    // 4 cmd1
                    // 5 cmd2
                    // 6 cmd3
                    // 7 cmd4

                    spires[0] = spi_swap(MsgGet.data[4]);
                    spires[1] = spi_swap(MsgGet.data[5]);
                    spires[2] = spi_swap(MsgGet.data[6]);
                    spires[3] = spi_swap(MsgGet.data[7]);
                    MsgRes.bytes = 7;
                    MsgRes.data[0] = CMD_SPI_MULTI;
                    MsgRes.data[1] = STATUS_CMD_OK;
                    MsgRes.data[2] = spires[0];
                    MsgRes.data[3] = spires[1];
                    MsgRes.data[4] = spires[2];
                    MsgRes.data[5] = spires[3];
                    MsgRes.data[6] = STATUS_CMD_OK;
                    put_msg_in_stk500(&MsgRes);
                    break;
                }
            }
            // End switch STK command --------------------------------------
            __asm__ ("nop");
            get_msg();
            // TODO error out
        } while(1);
    } else {
        return 0;
    }

}
