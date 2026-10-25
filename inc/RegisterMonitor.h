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

#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"

#include "RegiserMonitor_Status_Codes.h"

#define MAX_WRITE_CALLBACK 8U
#define MAX_GROUP (Register_Group_3 + 1)
#define MAX_GROUP_SIZE 16

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
        uint32_t Write_callback : 1;            // does register have a callback
        uint32_t callback_number : 3;           // call back number that
        uint32_t reserved : 26;                 // Reserved flags
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

/**
 * @brief Define the number of register groups 
 * 
 */
typedef enum {
    Register_group_0 = 0,   // Register Group 0
    Register_Group_1,       // Register Group 1
    Register_Group_2,       // Register Group 2
    Register_Group_3        // Register Group 3
}RegMon_group_e;


typedef struct 
{   
    RegMon_group_e group;
    uint32_t reg_values[MAX_GROUP_SIZE];
}RegMon_group_t;

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

Regmon_status_e RegMon_SetupGroup(uint16_t* reg_num, size_t num_of_reg, RegMon_group_e group);
Regmon_status_e RegMON_ClearGroup(RegMon_group_e group);

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

Regmon_status_e RegMON_ReadGroup(RegMon_group_t* dst);
Regmon_status_e RegMON_WriteGroup(RegMon_group_t* src);

#ifdef __cplusplus
}
#endif