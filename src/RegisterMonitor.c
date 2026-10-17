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
#include "RegisterMonitor.h"

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
 * @param reg_index register we need
 * @param value     value to check 
 * @return true     value are with params
 * @return false    value are outside params
 */
static bool CheckValue(uint32_t reg_index, uint32_t value)
{
    if (regmon_min_values != NULL)
    {
        if (regmon_min_values[reg_index] > value)
        {
            return false;
        }
    }

    if (regmon_max_values != NULL)
    {
        if (regmon_max_values[reg_index] < value)
        {
            return false;   
        }
    }
    return true;
}

/**
 * @brief Check if a block of registers is readable.
 * 
 * @param reg_index     Start register
 * @param num_of_regs   number of registers in block
 * @return true         all registers is readable
 * @return false        1 or more registers is not readable
 */
static bool CheckReadableMulti(uint32_t reg_index, uint32_t num_of_regs)
{
    for (uint32_t i = 0; i < num_of_regs; i++)
    {
        if (!register_list[reg_index].flags.readable)
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief check if a block of registers is writeable.
 * 
 * @param reg_index     Start register
 * @param num_of_regs   number of registers in block
 * @return true         all registers is writable
 * @return false        1 or more registers is not writable
 */
static bool CheckWriteableMulti(uint32_t reg_index, uint32_t num_of_regs)
{
    for (uint32_t i = 0; i < num_of_regs; i++)
    {
        if (!register_list[reg_index].flags.writeable)
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief Function to check values going into a block
 * 
 * @param reg_index     First register in block
 * @param num_of_regs   number of register in block
 * @param values        pointer to values to check
 * @return true         All values are within params
 * @return false        1 or more values are outside params
 */
static bool CheckValueBlock(uint32_t reg_index, uint32_t num_of_regs, uint32_t* values)
{
    for (uint32_t i = 0; i < num_of_regs; i++)
    {
        if (regmon_min_values != NULL)
        {   
            if (regmon_min_values[reg_index + i] > values[i])
            {
                return false;
            }
        }

        if (regmon_max_values != NULL)
        {
            if (regmon_max_values[reg_index + i] < values[i])
            {
                return false;   
            }
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
Regmon_status_e RegMon_Init(Register* ptr_list,
                            uint32_t* default_values,
                            uint32_t* min,
                            uint32_t* max,
                            uint32_t list_size
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

/**
 * @brief Register Monitor function to reset registers to default values
 *  
 * @return Regmon_status_e status of operation. 
 */
Regmon_status_e RegMON_default_list(void)
{
    Regmon_status_e status = REGMON_OK;
    
    if (regmon_default_values != NULL)
    {
        RegMON_Lock();
        {
            ResetToDefault();
        }
        RegMON_Unlock();
    }
    else
    {
        status = REGMON_ERROR;
    }
    
    return status;
}

/**
 * @brief Register Monitor function to reset registers to OEM default values
 * 
 * @return Regmon_status_e status of operation
 */
Regmon_status_e RegMON_default_list_to_OEM(void)
{
    Regmon_status_e status = REGMON_OK;
    
    if (regmon_oem_default_values != NULL)
    {
        RegMON_Lock();
        {
            ResetToOEM();
        }
        RegMON_Unlock();
    }
    else
    {
        status = REGMON_ERROR;
    }
    return status;
}

/**
 * @brief Register monitor read register function
 * 
 * @param reg_index register number to read
 * @param dst pointer where to readout the register.
 * @return Regmon_status_e Status of operation. 
 */
Regmon_status_e RegMon_read_register(uint32_t reg_index, uint32_t* dst)
{
    Regmon_status_e status = REGMON_OK;
    if (dst != NULL)
    {

            if (reg_index < number_of_registers)
            {
                if (register_list[reg_index].flags.readable)
                {
                    RegMON_Lock();
                    {
                        *dst = register_list[reg_index].value;
                    }
                    RegMON_Unlock();
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
    else
    {
        status = REGMON_ERROR;
    }
    return status;
}

/**
 * @brief Register monitor function to write a register.
 * 
 * @param reg_index register number to write to
 * @param value value to write
 * @return Regmon_status_e status of operation.
 */
Regmon_status_e RegMon_write_register(uint32_t reg_index, uint32_t value)
{
    Regmon_status_e status = REGMON_OK;
    if (reg_index < number_of_registers)
    {
        if (register_list[reg_index].flags.writeable)
        {
            RegMON_Lock();
            {
                if (CheckValue(reg_index, value))
                {
                    register_list[reg_index].value = value;
                }
                else
                {
                    status = REGMON_INVALID_VALUE;
                }
            }
            RegMON_Unlock();
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
    return status;
}

/**
 * @brief Register monitor funcion to read a block of register.
 * 
 * @param reg_index     Register number to start from 
 * @param num_of_regs   number of register in block
 * @param dst           pointer to register buffer
 * @return Regmon_status_e Status of operation.
 */
Regmon_status_e RegMon_read_register_block(
    uint32_t reg_index,
    uint32_t num_of_regs,
    uint32_t* dst
)
{
    Regmon_status_e status = REGMON_OK;
    if (dst != NULL)
    {
        if ((reg_index + num_of_regs) < number_of_registers)
        {
            if (CheckReadableMulti(reg_index, num_of_regs))
            {
                RegMON_Lock();
                {
                    for (uint32_t i = 0; i < num_of_regs; i++)
                    {
                        dst[i] = register_list[reg_index + i].value; 
                    }
                }
                RegMON_Unlock();
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
    else
    {
        status = REGMON_ERROR;
    }
    return status;
}

Regmon_status_e RegMON_write_register_block(
    uint32_t reg_index,
    uint32_t num_of_regs,
    uint32_t* values
)
{
    Regmon_status_e status = REGMON_OK;
    if ((values != NULL) && (num_of_regs != 0))
    {
        if ((reg_index + num_of_regs) < number_of_registers)
        {
            if (CheckWriteableMulti(reg_index, num_of_regs))
            {
                RegMON_Lock();
                {
                    if (CheckValueBlock(reg_index, num_of_regs, values))
                    {
                        for (uint32_t i = 0; i < num_of_regs; i++)
                        {
                            register_list[reg_index + i].value = values[i];
                        }
                    }
                    else 
                    {
                        status = REGMON_INVALID_VALUE;
                    }
                }
                RegMON_Unlock();
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
    else
    {
        status = REGMON_ERROR;
    }
    return status;
}

/**
 * @brief Register Monitor function to read a single bit in a register.
 * 
 * @param reg_index Register to read from
 * @param bitnum    bit number to read
 * @param dst       pointer to buffer to readout bit
 * @return Regmon_status_e status of operation.
 */
Regmon_status_e RegMON_read_bit(uint32_t reg_index, uint8_t bitnum, uint8_t* dst)
{
    Regmon_status_e status = REGMON_OK;
    if ((dst != NULL) && (bitnum != 0) && (bitnum < 33))
    {
        if (reg_index < number_of_registers)
        {
            if (register_list[reg_index].flags.readable)
            {   
                RegMON_Lock();
                {
                    *dst = (register_list[reg_index].value >> (bitnum - 1)) & 0x01;
                }
                RegMON_Unlock();
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
    else
    {
        status = REGMON_ERROR;
    }
    return status;
}

/**
 * @brief Register monitor function to write a single bit in a register.
 * 
 * @param reg_index register to write to 
 * @param bitnum    bit number to write 
 * @param val       bit value to write
 * @return Regmon_status_e status of operation
 */
Regmon_status_e RegMON_write_bit(uint32_t reg_index, uint8_t bitnum, bool val)
{
    Regmon_status_e status = REGMON_OK;
    if (bitnum != 0 && bitnum < 33)
    {
        if (reg_index < number_of_registers)
        {
            if (register_list[reg_index].flags.writeable)
            {
                RegMON_Lock();
                {
                    if (val)
                    {
                        register_list[reg_index].value |= (1 << (bitnum - 1));
                    }
                    else
                    {
                        register_list[reg_index].value &= ~(1 << (bitnum -1));
                    }
                }
                RegMON_Unlock();
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
    else
    {
        status = REGMON_ERROR;
    }
    return REGMON_ERROR;
}