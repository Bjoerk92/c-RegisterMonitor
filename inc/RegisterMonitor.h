/**
 * @file RegMon.h
 * @author Anders (anders.bjoerk.unf@gmail.com)
 * @brief Interface file for Register monitor
 * @version 0.1
 * @date 2026-10-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once

#include "stdint.h"
#include "stdbool.h"

#define MAX_WRITE_CALLBACK 8U

/**
 * @brief Register monitor error codes.
 * 
 */
typedef enum {
    REGMON_OK = 0,
    REGMON_ERROR = -1,
    REGMON_INVALID_VALUE = -2, 
    REGMON_INVALID_ADDRESS = -3,
    REGMON_ADDRESS_NOT_WRITEABLE = -4,
    REGMON_ADDRESS_NOT_READABLE = -5,
    REGMON_MUTEX_ERROR = -6,
    REGMON_CYCLIC_READ_EXIDE_CAP = -7,
    REGMON_CYCLIC_WRITE_EXIDE_CAP = -8,
}Regmon_status_e;

/**
 * @brief Register monitors single register definition.
 * 
 */
typedef struct {
    volatile uint32_t value;                    // value of register
    
    /**
     * @brief Flags used to indicate registers operation and functions
     * 
     */
    struct
    {
        uint32_t readable : 1;                  // is register readable
        uint32_t writeable : 1;                 // is register writeble
        uint32_t cyclic_readable : 1;           // is register cyclic read-able
        uint32_t cyclic_writeable : 1;          // is register cyclic write-able
        uint32_t Write_callback : 1;            // does register have a callback
        uint32_t callback_number : 3;           // call back number that
        uint32_t reserved : 24;
    }flags;
}Register;

/**
 * @brief defines the mutex callback function used by the register monitor.
 * 
 */
typedef void (*RegMon_MutexCallback)(void* context, uint32_t timeout);

/**
 * @brief Defines a register callback function when it is written to. 
 * 
 */
typedef void (*RegMON_WriteCallback)(uint32_t value);

/*****************************************************************************/
/*                      Register monitor interface functions                 */
/*****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

Regmon_status_e RegMon_Init(
    Register* ptr_list,
    uint32_t* default_values,
    uint32_t* min,
    uint32_t* max,
    uint32_t list_size
);

Regmon_status_e RegMon_SetMutexCallbacks(
    RegMon_MutexCallback lock,
    RegMon_MutexCallback unlock,
    void* context
);

Regmon_status_e RegMON_SetWriteCallback(
    uint8_t callback_nr,
    RegMON_WriteCallback cb
);

Regmon_status_e RegMON_default_list(void);
Regmon_status_e RegMON_default_list_to_OEM(void);

Regmon_status_e RegMon_read_register(uint32_t reg_index, uint32_t* dst);
Regmon_status_e RegMon_write_register(uint32_t reg_index, uint32_t value);

Regmon_status_e RegMon_read_register_block(
    uint32_t reg_index,
    uint32_t num_of_regs,
    uint32_t* dst
);

Regmon_status_e RegMON_write_register_block(
    uint32_t reg_index,
    uint32_t num_of_regs,
    uint32_t* values
);

Regmon_status_e RegMON_read_bit(uint32_t reg_index, uint8_t bitnum, uint8_t* dst);
Regmon_status_e RegMON_write_bit(uint32_t reg_index, uint8_t bitnum, bool val);

#ifdef __cplusplus
}
#endif