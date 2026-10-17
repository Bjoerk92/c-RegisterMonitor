/**
 * @file RegisterMonitor_UIT.cpp
 * @author Anders (anders.bjoerk.unf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2026-10-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <gtest/gtest.h>
#include <array>
#include "RegisterMonitor.h"

constexpr int number_of_registers = 6;

std::array<Register, number_of_registers> register_list{};

void SetUpRegisters()
{
    // Device ID
    register_list[0].value = 0x20100000;
    register_list[0].flags.readable = true;
    register_list[0].flags.writeable = false;

    // Device model
    register_list[1].value = 0x00000001;
    register_list[1].flags.readable = true;
    register_list[1].flags.writeable = false;

    // Actual Speed
    register_list[2].value = 0;
    register_list[2].flags.readable = true;
    register_list[2].flags.writeable = true;

    // Actual position
    register_list[3].value = 0;
    register_list[3].flags.readable = true;
    register_list[3].flags.writeable = true;

    // Operation bit 
    register_list[4].value = 0x00000101;
    register_list[4].flags.readable = true;
    register_list[4].flags.writeable = false;

    register_list[5].value = 0x0101;
    register_list[5].flags.readable = false;
    register_list[5].flags.writeable = false;
}


TEST(Init, nullptr)
{
    Regmon_status_e status = REGMON_OK;
    status = RegMon_Init(nullptr, nullptr, nullptr, nullptr, 0);
    EXPECT_EQ(status, REGMON_ERROR);
}

TEST(Init, ZeroSize)
{
    Regmon_status_e status = REGMON_OK;
    SetUpRegisters();
    status = RegMon_Init(register_list.data(), nullptr, nullptr, nullptr, 0);
    EXPECT_EQ(status, REGMON_ERROR);
}

TEST(Init, NoDefaults)
{
    Regmon_status_e status = REGMON_OK;
    SetUpRegisters();
    status = RegMon_Init(register_list.data(), nullptr, nullptr, nullptr, register_list.size());
    EXPECT_EQ(status, REGMON_OK);
}

TEST(Register, DeviceID)
{
    Regmon_status_e status = REGMON_OK;
    SetUpRegisters();
    status = RegMon_Init(register_list.data(), nullptr, nullptr, nullptr, register_list.size());
    EXPECT_EQ(status, REGMON_OK);

    uint32_t Device_ID = 0;
    status = RegMon_read_register(0, &Device_ID);
    EXPECT_EQ(status, REGMON_OK);
    EXPECT_EQ(Device_ID, 0x20100000);

    status = RegMon_write_register(0, 0);
    EXPECT_EQ(status, REGMON_ADDRESS_NOT_WRITEABLE);
}

TEST(Register, DeviceModel)
{
    Regmon_status_e status = REGMON_OK;
    SetUpRegisters();
    status = RegMon_Init(register_list.data(), nullptr, nullptr, nullptr, register_list.size());
    EXPECT_EQ(status, REGMON_OK);

    uint32_t DeviceModle = 0;
    status = RegMon_read_register(1, &DeviceModle);
    EXPECT_EQ(status, REGMON_OK);
    EXPECT_EQ(DeviceModle, 0x01);

    status = RegMon_write_register(0, 0);
    EXPECT_EQ(status, REGMON_ADDRESS_NOT_WRITEABLE);   
}

TEST(BitHandling, ReadBit)
{
    Regmon_status_e status = REGMON_OK;
    SetUpRegisters();
    status = RegMon_Init(register_list.data(), nullptr, nullptr, nullptr, register_list.size());
    EXPECT_EQ(status, REGMON_OK);

    uint8_t bitval = 0;
    status = RegMON_read_bit(4, 9, &bitval);
    EXPECT_EQ(bitval, 1);
    EXPECT_EQ(status, REGMON_OK);

    status = RegMON_read_bit(4,8,&bitval);
    EXPECT_EQ(bitval, 0);
    EXPECT_EQ(status, REGMON_OK);

    status = RegMON_read_bit(5,9,&bitval);
    EXPECT_EQ(bitval, 0);
    EXPECT_EQ(status, REGMON_ADDRESS_NOT_READABLE);
}