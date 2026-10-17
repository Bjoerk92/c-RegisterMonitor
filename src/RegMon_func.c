/**
 * @file RegMon_func.c
 * @author Anders (anders.bjoerk.unf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2026-10-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "RegMon.h"

#include "stdbool.h"
#include "stdint.h"

/*****************************************************************************/
/*                       Variables for Register setup                        */
/*****************************************************************************/

/**
 * Pointer to the register list. 
 */
static Register* register_list = NULL;

/**
 * Pointer to the registers default/factory values.
 */
static uint32_t* regmon_default_values = NULL;

/**
 * Pointer to the OEM registers default/factory values.
 * 
 */
static uint32_t* regmon_oem_default_values = NULL;

/**
 * Pointer to the registers minimum value
 */
static uint32_t* regmon_min_values;

/**
 * Pointer to the registers maximum value
 * 
 */
static uint32_t* regmon_max_values;

/**
 * number of register
 */
static uint32_t number_of_registers = 0;
                            
/*****************************************************************************/
/*                          Variables for Mutex usage                        */
/*****************************************************************************/
static RegMon_MutexCallback mutex_lock;
static RegMon_MutexCallback mutex_unlock;
static void* mutex_context;


/**
 * @brief Local function to mutex lock the register monitor
 * 
 */
static inline void RegMON_Lock()
{
    if (mutex_lock != NULL)
    {
        mutex_lock(mutex_context, 1);
    }
}

/**
 * @brief Local function to mutex unlock the register monitor
 * 
 */
static inline void RegMON_Unlock()
{
    if (mutex_unlock != NULL)
    {
        mutex_unlock(mutex_context, 1);
    }
}

/**
 * @brief Function to restore the register values to default.
 * 
 */
static void ResetToDefault()
{
    for (uint32_t i = 0; i < number_of_registers; i++)
    {
        register_list[i].value = regmon_default_values[i];
    }
}

static void ResetToOEM()
{
    for (uint32_t i = 0; i < number_of_registers; i++)
    {
        register_list[i].value = regmon_oem_default_values[i];
    }
}

/**
 * @brief Check if a given value is within min/max
 * 
 * @note we only check the value if the poiter to min and max are not NULL
 * 
 * @param reg       register we need
 * @param value     value to check 
 * @return true     value are with params
 * @return false    value are outside params
 */
static bool CheckValue(uint32_t reg, uint32_t value)
{
    if (regmon_min_values != NULL)
    {
        if (regmon_min_values[reg] > value)
        {
            return false;
        }
    }

    if (regmon_max_values != NULL)
    {
        if (regmon_max_values[reg] < value)
        {
            return false;   
        }
    }
    return true;
}

/**
 * @brief Check if a block of registers is readable.
 * 
 * @param reg           Start register
 * @param num_of_regs   number of registers in block
 * @return true         all registers is readable
 * @return false        1 or more registers is not readable
 */
static bool CheckReadableMulti(uint32_t reg, uint32_t num_of_regs)
{
    for (uint32_t i = 0; i < num_of_regs; i++)
    {
        if (!register_list[reg].flags.readable)
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief Register Monitor function to set the usage of RTOS Mutex's
 * 
 * @note If Mutex protection is required
 * 
 * @param lock      Function pointer to lock function 
 * @param unlock    Function pointer to unlock function
 * @param context   pointer to the mutex context
 * @return Regmon_status_e Status of operation. 
 */
Regmon_status_e RegMon_SetMutexCallbacks(
    RegMon_MutexCallback lock,
    RegMon_MutexCallback unlock,
    void* context)
{
    if (((lock == NULL) || (unlock == NULL)) && lock == unlock)
    {   // the function pointers cant be null or the same function!
        return REGMON_ERROR;
    }
    // all is okay set the function pointers.
    mutex_lock = lock;
    mutex_unlock = unlock;
    mutex_context = context;
    return REGMON_OK;
}

/**
 * @brief Initialize the register monitor.
 * 
 * @param ptr_list          Pointer to the register list
 * @param default_values    Pointer to default values
 * @param min               Pointer to minimum values
 * @param max               Pointer to Maximum values
 * @param list_size         number of registers.
 * @return Regmon_status_e 
 */
Regmon_status_e RegMon_Init(const Register* ptr_list,
                            const uint32_t* default_values,
                            const uint32_t* min,
                            const uint32_t* max,
                            const uint32_t list_size
)
{
    Regmon_status_e status = REGMON_OK;

    if (ptr_list == NULL || list_size == 0)
    {
        status = REGMON_ERROR;
    }
    else
    {   
        register_list = ptr_list;
        regmon_default_values = default_values;
        regmon_max_values = max;
        regmon_min_values = min;
        number_of_registers = list_size;
    }
    return status;
}

Regmon_status_e RegMON_default_list(void)
{
    Regmon_status_e status = REGMON_OK;
    RegMON_Lock();
    {
        if (regmon_default_values != NULL)
        {
            ResetToDefault();
        }
        else
        {
            status = REGMON_ERROR;
        }
    }
    RegMON_Unlock();
    return status;
}

Regmon_status_e RegMON_default_list_to_OEM(void)
{
    Regmon_status_e status = REGMON_OK;
    RegMON_Lock();
    {
        if (regmon_oem_default_values != NULL)
        {
            ResetToOEM();
        }
        else
        {
            status = REGMON_ERROR;
        }
    }
    RegMON_Unlock();
    return status;
}

/**
 * @brief Register monitor read register function
 * 
 * @param reg register number to read
 * @param dst pointer where to readout the register.
 * @return Regmon_status_e Status of operation. 
 */
Regmon_status_e RegMon_read_register(uint32_t reg, uint32_t* dst)
{
    Regmon_status_e status = REGMON_OK;
    RegMON_Lock();
    {
        if (reg < number_of_registers)
        {
            if (register_list[reg].flags.readable)
            {
                *dst = register_list[reg].value;
            }
            else
            {
                status = REGMON_ADDRESS_NOT_READABLE;
            }
        }
        else
        {
            status = REGMON_INVALID_ADDRESS;
        }

    }
    RegMON_Unlock();
    return status;
}

/**
 * @brief Register monitor function to write a register.
 * 
 * @param reg   register number to write to
 * @param value value to write
 * @return Regmon_status_e status of operation.
 */
Regmon_status_e RegMon_write_register(uint32_t reg, uint32_t value)
{
    Regmon_status_e status = REGMON_OK;
    RegMON_Lock();
    {
        if (reg < number_of_registers)
        {
            if (register_list[reg].flags.writeable)
            {
                if (CheckValue(reg, value))
                {
                    register_list[reg].value = value;
                }
                else
                {
                    status = REGMON_INVALID_VALUE;
                }
            }
            else
            {
                status = REGMON_ADDRESS_NOT_WRITEABLE;
            }
        }
        else
        {
            status = REGMON_INVALID_ADDRESS;
        }
    }
    RegMON_Unlock();
    return status;
}

/**
 * @brief Register monitor funcion to read a block of register.
 * 
 * @param reg           Register number to start from 
 * @param num_of_regs   number of register in block
 * @param dst           pointer to register buffer
 * @return Regmon_status_e Status of operation.
 */
Regmon_status_e RegMon_read_register_block(
    uint32_t reg,
    uint32_t num_of_regs,
    uint32_t* dst
)
{
    Regmon_status_e status = REGMON_OK;
    RegMON_Lock();
    {
        if ((reg + num_of_regs) < number_of_registers)
        {
            if (CheckReadableMulti(reg, num_of_regs))
            {
                for (uint32_t i = 0; i < num_of_regs; i++)
                {
                    dst[i] = register_list[reg + i].value; 
                }
            }
            else
            {
                status = REGMON_ADDRESS_NOT_READABLE;
            }
        }
        else
        {
            status = REGMON_INVALID_ADDRESS;
        }
    }
    RegMON_Unlock();
    return status;
}

