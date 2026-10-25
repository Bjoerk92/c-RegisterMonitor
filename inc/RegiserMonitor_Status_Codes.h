/**
 * @file RegiserMonitor_Errors.h
 * @author Anders Bjørk (anders.bjoerk.unf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2026-10-25
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once

/**
 * @brief Register monitor status codes.
 * 
 */
typedef enum {
    REGMON_OK = 0,
    REGMON_ERROR = -1,
    REGMON_INVALID_PARAM = -2,
    REGMON_INVALID_VALUE = -3, 
    REGMON_INVALID_ADDRESS = -4,
    REGMON_ADDRESS_NOT_WRITEABLE = -5,
    REGMON_ADDRESS_NOT_READABLE = -6,
    REGMON_MUTEX_ERROR = -7,
    REGMON_GROUP_EXCEED_SIZE = -8
}Regmon_status_e;
