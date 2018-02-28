/**
 * @file main.c
 * @author LiYu
 * @date 2017.12.27
 * @brief STK500 主要邏輯處理，負責接收並處理船UART來之訊息。
 *
 * 接收從UART來之訊息封包，並解析成命令，在依照命令去執行相對應動作，個命令應對應動作請
 * 參照 AVR068.pdf (AVR068: STK500 Communication Protocol)。
 *
 */

#include "revlib/ASA/ASA_spi.h"
#include "revlib/ASA/ASA_general.h"
#include "revlib/rev_serial.h"
#include "command.h"
#include "stk500.h"

#define STATUS_START  0
#define STATUS_SEQ    1
#define STATUS_MSG1   2
#define STATUS_MSG2   3
#define STATUS_TOKEN  4
#define STATUS_DATA   5
#define STATUS_CHKSUM 6

#define STK_START 0x1B
#define STK_SEQ   0x01
#define STK_TOKEN 0x0E

/**
 * @brief STK500主要邏輯處理，負責接收並處理UART傳來之訊息。
 *
 * NOTE The current STK500 firmware can only handle messages with a message body
 * of maximum of 275 bytes.
 */
struct cmd {
    uint16_t bytes;
    uint8_t data[300];
};


Addres_t CurAddres; ///< Recording the current memory location to write.
uint8_t Sequence = 0; ///< Recording the current sequence of STK500 packet.

struct cmd GetCmd; ///< Save the command after decoding (function get_cmd).
struct cmd ResCmd; ///< The response command to be sent. It will be sent\
 by functino put_cmd.

/**
 * @brief Get one byte data from UART
 */
uint8_t serial_get() {
    while((UCSR0A&(1<<RXC0))==0)
        ;
	return UDR0;
}

/**
 * @brief Put one byte data to UART
 */
uint8_t serial_put(uint8_t data) {
    while((UCSR0A&(1<<UDRE0))==0)
        ;
    UDR0 = data;
    return 0;
}
#define RES_ERROR 3
/**
 * @brief Get a command from UART, will put in global var "GetCmd".
 *
 * 依照 STK500v2 的訊息封包格式進行解包，並解包成命令存放在全域變數 "GetCmd" 中。
 */
 uint8_t get_cmd() {
     uint8_t ch;
     uint8_t chksum = 0;
     ch = serial_get();
     // get STK500's START or ASSPROG's header to decode msg type

     // STK500 decoder
     if (ch == STK_START) {
         chksum ^= ch;

         //  Get Sequence
         ch = serial_get();
         chksum ^= ch;
         Sequence = ch;
         // TODO implement Sequence
         // NOTE
         // if (ch != Sequence+1)
         //     return 2;
         // Sequence++;

         // Get Sequence
         ch = serial_get();
         chksum ^= ch;
         GetCmd.bytes = ch;

         // Get Byte high
         ch = serial_get();
         chksum ^= ch;
         GetCmd.bytes = (GetCmd.bytes<<8) + ch;

         // Get Token
         ch = serial_get();
         chksum ^= ch;
         if (ch != STK_TOKEN)
             return RES_ERROR;

         // Get Msgbody (cmd of STK500)
         for (uint16_t i = 0; i < GetCmd.bytes; i++) {
             ch = serial_get();
             chksum ^= ch;
             GetCmd.data[i] = ch;
         }

         // Get chksum
         ch = serial_get();
         if (ch != chksum)
             return RES_ERROR;

         return 0;
     }
     // ASAPROG decoder
     else if (ch == 0xFC) {

         // Get Header
         for (uint8_t i = 0; i < 3; i++)
             ch = serial_get();
             if (ch != 0xFC)
                 return RES_ERROR;

         // Get Sequence
         if (serial_get() != 0x01)
             return RES_ERROR;

         // Get Byte high
         ch = serial_get();
         chksum += ch;
         GetCmd.bytes = (GetCmd.bytes<<8) + ch;

         // Get Byte low
         ch = serial_get();
         chksum += ch;
         if (ch != STK_TOKEN)
             return RES_ERROR;

         // Get Msgbody (exec binary)
         for (uint16_t i = 0; i < GetCmd.bytes; i++) {
             ch = serial_get();
             chksum += ch;
             GetCmd.data[i] = ch;
         }
         // Fill the page size (SPM_PAGESIZE) if the size is less then SPM_PAGESIZE
         // Otherwise will write trash data into memory
         for (uint16_t i = GetCmd.bytes; i < SPM_PAGESIZE; i++) {
             GetCmd.data[i] = 0;
         }

         // Get chksum
         ch = serial_get();
         if (ch != chksum)
             return RES_ERROR;

         return RES_ERROR;

     } else {
         return RES_ERROR;
     }
 }

/**
 * @brief Put a command "PutCmd" to UART.
 *
 * 依照 STK500v2 的訊息封包格式將全域變數 "PutCmd" 進行打包，並傳送到UART串列埠。
 */
uint8_t put_cmd(struct cmd* c) {
    uint8_t chksum=0;
    uint8_t ch;

    ch = STK_START;
    chksum ^= ch;
    serial_put(ch);

    ch = Sequence;
    chksum ^= ch;
    serial_put(ch);

    ch = c->bytes>>8;
    chksum ^= ch;
    serial_put(ch);

    ch = c->bytes&0xFF;
    chksum ^= ch;
    serial_put(ch);

    ch = STK_TOKEN;
    chksum ^= ch;
    serial_put(ch);

    for (uint16_t i = 0; i < c->bytes; i++) {
        ch = c->data[i];
        chksum ^= ch;
        serial_put(ch);
    }
    serial_put(chksum);
    return 0;
}

int main() {
    rev_serial_init();
    uint8_t get_cmd_res;
    uint8_t spires[4]; ///< 接收下達給待燒錄裝置的4byte spi命令
    while (1) {
        get_cmd_res = get_cmd();
        if (get_cmd_res) {
            /* TODO error out*/
        }
        // ***[ STK general command constants ]********************************
        switch (GetCmd.data[0]) {
            case CMD_SIGN_ON: {
                ResCmd.bytes = 11;
                ResCmd.data[0] = CMD_SIGN_ON;
                ResCmd.data[1] = STATUS_CMD_OK;
                ResCmd.data[2] = 8;
                ResCmd.data[3] = 'A';
                ResCmd.data[4] = 'V';
                ResCmd.data[5] = 'R';
                ResCmd.data[6] = 'I';
                ResCmd.data[7] = 'S';
                ResCmd.data[8] = 'P';
                ResCmd.data[9] = '_';
                ResCmd.data[10] = '2';
                put_cmd(&ResCmd);
                break;
            }
            case CMD_SET_PARAMETER: {
                uint8_t id = GetCmd.data[1];
                uint8_t value = GetCmd.data[2];
                uint8_t res = isp_set_param(id,value);
                if(res) {
                    ResCmd.bytes = 2;
                    ResCmd.data[0] = CMD_SET_PARAMETER;
                    ResCmd.data[1] = STATUS_CMD_FAILED;
                    put_cmd(&ResCmd);
                } else {
                    ResCmd.bytes = 2;
                    ResCmd.data[0] = CMD_SET_PARAMETER;
                    ResCmd.data[1] = STATUS_CMD_OK;
                    put_cmd(&ResCmd);
                }
                break;
            }
            case CMD_GET_PARAMETER: {
                uint8_t id = GetCmd.data[1];
                uint8_t value = 1;
                uint8_t res = isp_get_param(id, &value);
                if(res) {
                    ResCmd.bytes = 2;
                    ResCmd.data[0] = CMD_GET_PARAMETER;
                    ResCmd.data[1] = STATUS_CMD_FAILED;
                    put_cmd(&ResCmd);
                } else {
                    ResCmd.bytes = 3;
                    ResCmd.data[0] = CMD_GET_PARAMETER;
                    ResCmd.data[1] = STATUS_CMD_OK;
                    ResCmd.data[2] = value;
                    put_cmd(&ResCmd);
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
                CurAddres.ui8[0] = GetCmd.data[4];
                CurAddres.ui8[1] = GetCmd.data[3];
                CurAddres.ui8[2] = GetCmd.data[2];
                CurAddres.ui8[3] = GetCmd.data[1];
                ResCmd.bytes = 2;
                ResCmd.data[0] = CMD_LOAD_ADDRESS;
                ResCmd.data[1] = STATUS_CMD_OK;
                put_cmd(&ResCmd);
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
                spires[0] = spi_swap(GetCmd.data[ 8]);
                spires[1] = spi_swap(GetCmd.data[ 9]);
                spires[2] = spi_swap(GetCmd.data[10]);
                spires[3] = spi_swap(GetCmd.data[11]);
                ResCmd.data[0] = CMD_ENTER_PROGMODE_ISP;
                _delay_ms(1000);
                // if (GetCmd.data[6]!=spires[GetCmd.data[7]]) {
                //     ResCmd.bytes = 2;
                //     ResCmd.data[1] = STATUS_CMD_FAILED;
                //     put_cmd(&ResCmd);
                // } else if (0) {
                //     // TODO timeout
                //     ResCmd.data[1] = STATUS_CMD_TOUT;
                // } else {
                    ResCmd.bytes = 2;
                    ResCmd.data[1] = STATUS_CMD_OK;
                    put_cmd(&ResCmd);
                // }
                break;
            }
            case CMD_LEAVE_PROGMODE_ISP: {
                // 0 Command ID
                // 1 preDelay
                // 2 postDelay

                // _delay_ms(GetCmd.data[1]);
                isp_leave_progmode();
                // _delay_ms(GetCmd.data[2]);
                ResCmd.bytes = 2;
                ResCmd.data[0] = CMD_LEAVE_PROGMODE_ISP;
                ResCmd.data[1] = STATUS_CMD_OK;
                put_cmd(&ResCmd);
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

                spi_swap(GetCmd.data[3]);
                spi_swap(GetCmd.data[4]);
                spi_swap(GetCmd.data[5]);
                spi_swap(GetCmd.data[6]);
                // _delay_ms(GetCmd.data[1]);

                ResCmd.data[0] = CMD_CHIP_ERASE_ISP;
                if (GetCmd.data[6]!=spires[GetCmd.data[7]]) {
                    ResCmd.bytes = 2;
                    ResCmd.data[1] = STATUS_CMD_FAILED;
                    put_cmd(&ResCmd);
                } else {
                    ResCmd.bytes = 2;
                    ResCmd.data[1] = STATUS_CMD_OK;
                    put_cmd(&ResCmd);
                }

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
                uint16_t N = (GetCmd.data[1]<<8) + GetCmd.data[2];
                Addres_t tmpadd = CurAddres;
                // _delay_ms(10);
                for (uint16_t i = 0; i < N; i++) {
                    isp_load_flash(i&1, 0, CurAddres.ui8[0], GetCmd.data[10+i]);
                    if (i&1) {
                        CurAddres.ui32++;
                    }
                }
                isp_write_flash(tmpadd.ui8[1], tmpadd.ui8[0]);
                // _delay_ms(10);
                // _delay_ms(GetCmd.data[4]);

                ResCmd.bytes = 2;
                ResCmd.data[0] = CMD_PROGRAM_FLASH_ISP;
                ResCmd.data[1] = STATUS_CMD_OK;
                put_cmd(&ResCmd);
                break;
            }
            case CMD_READ_FLASH_ISP: {
                // 0 Command ID
                // 1 NumBytes 15:8
                // 2 NumBytes  7:0
                // 3 cmd1

                uint16_t N = (GetCmd.data[1]<<8) + GetCmd.data[2];
                for (uint16_t i = 0; i < N; i++) {
                    spi_swap(GetCmd.data[3] | (i&1)<<3 );
                    spi_swap(CurAddres.ui8[1]);
                    spi_swap(CurAddres.ui8[0]);
                    ResCmd.data[2+i] = spi_swap(0x00);
                    if (i&1) {
                        CurAddres.ui32++;
                    }
                    // _delay_ms(1);
                }
                ResCmd.data[0] = CMD_READ_FLASH_ISP;
                if ( 0 ) {
                    ResCmd.bytes = 2;
                    ResCmd.data[1] = STATUS_CMD_FAILED;
                    put_cmd(&ResCmd);
                } else {
                    ResCmd.bytes = 3+N;
                    ResCmd.data[1] = STATUS_CMD_OK;
                    ResCmd.data[2+N] = STATUS_CMD_OK;
                    put_cmd(&ResCmd);
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

                uint16_t N = (GetCmd.data[1]<<8) + GetCmd.data[2];
                // _delay_ms(10);
                for (uint16_t i = 0; i < N; i++) {
                    isp_write_eeprom(CurAddres.ui8[1], CurAddres.ui8[0], GetCmd.data[10+i]);
                    CurAddres.ui32++;
                    // _delay_ms(GetCmd.data[4]);
                }

                ResCmd.bytes = 2;
                ResCmd.data[0] = CMD_PROGRAM_EEPROM_ISP;
                ResCmd.data[1] = STATUS_CMD_OK;
                put_cmd(&ResCmd);
                break;
            }
            case CMD_READ_EEPROM_ISP: {
                // 0 Command ID
                // 1 NumBytes 15:8
                // 2 NumBytes  7:0
                // 3 cmd1

                uint16_t N = (GetCmd.data[1]<<8) + GetCmd.data[2];
                for (uint16_t i = 0; i < N; i++) {
                    spi_swap(GetCmd.data[3]);
                    spi_swap(CurAddres.ui8[1]);
                    spi_swap(CurAddres.ui8[0]);
                    ResCmd.data[2+i] = spi_swap(0x00);
                    CurAddres.ui32++;
                    // _delay_ms(1);
                }
                ResCmd.data[0] = CMD_READ_EEPROM_ISP;
                if ( 0 ) {
                    ResCmd.bytes = 2;
                    ResCmd.data[1] = STATUS_CMD_FAILED;
                    put_cmd(&ResCmd);
                } else {
                    ResCmd.bytes = 3+N;
                    ResCmd.data[1] = STATUS_CMD_OK;
                    ResCmd.data[2+N] = STATUS_CMD_OK;
                    put_cmd(&ResCmd);
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
                spi_swap(GetCmd.data[1]);
                spi_swap(GetCmd.data[2]);
                spi_swap(GetCmd.data[3]);
                spi_swap(GetCmd.data[4]);
                ResCmd.bytes = 3;
                ResCmd.data[0] = CMD_READ_OSCCAL_ISP;
                ResCmd.data[1] = STATUS_CMD_OK;
                ResCmd.data[2] = STATUS_CMD_OK;
                put_cmd(&ResCmd);
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
                spi_swap(GetCmd.data[2]);
                spi_swap(GetCmd.data[3]);
                spi_swap(GetCmd.data[4]);
                uint8_t res = spi_swap(GetCmd.data[5]);
                ResCmd.bytes = 4;
                ResCmd.data[0] = CMD_READ_OSCCAL_ISP;
                ResCmd.data[1] = STATUS_CMD_OK;
                ResCmd.data[2] = res;
                ResCmd.data[3] = STATUS_CMD_OK;
                put_cmd(&ResCmd);
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

                spires[0] = spi_swap(GetCmd.data[4]);
                spires[1] = spi_swap(GetCmd.data[5]);
                spires[2] = spi_swap(GetCmd.data[6]);
                spires[3] = spi_swap(GetCmd.data[7]);
                ResCmd.bytes = 7;
                ResCmd.data[0] = CMD_SPI_MULTI;
                ResCmd.data[1] = STATUS_CMD_OK;
                ResCmd.data[2] = spires[0];
                ResCmd.data[3] = spires[1];
                ResCmd.data[4] = spires[2];
                ResCmd.data[5] = spires[3];
                ResCmd.data[6] = STATUS_CMD_OK;
                put_cmd(&ResCmd);
                break;
            }

        }
    }
    return 0;
}
