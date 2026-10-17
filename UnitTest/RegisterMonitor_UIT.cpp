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

void SetUpRegisters(std::array<Register, number_of_registers>& registers)
{
    // Device ID
    registers[0].value = 0x20100000;
    registers[0].flags.readable = true;
    registers[0].flags.writeable = false;

    // Device model
    registers[1].value = 0x00000001;
    registers[1].flags.readable = true;
    registers[1].flags.writeable = false;

    // Actual Speed
    registers[2].value = 0;
    registers[2].flags.readable = true;
    registers[2].flags.writeable = true;

    // Actual position
    registers[3].value = 0;
    registers[3].flags.readable = true;
    registers[3].flags.writeable = true;

    // Operation bit 
    registers[4].value = 0x00000101;
    registers[4].flags.readable = true;
    registers[4].flags.writeable = false;

    // Status bit
    registers[5].value = 0x0101;
    registers[5].flags.readable = false;
    registers[5].flags.writeable = false;
}

TEST(RegisterMonitorInit, ReturnsOkForValidRegisterList)
{
    std::array<Register, number_of_registers> registers{};
    std::array<uint32_t, number_of_registers> defaults{};
    std::array<uint32_t, number_of_registers> min_values{};
    std::array<uint32_t, number_of_registers> max_values{};

    EXPECT_EQ(
        RegMon_Init(
            registers.data(),
            defaults.data(),
            min_values.data(),
            max_values.data(),
            number_of_registers),
        REGMON_OK);
}

TEST(RegisterMonitorInit, ReturnsErrorForNullRegisterList)
{
    EXPECT_EQ(RegMon_Init(nullptr, nullptr, nullptr, nullptr, number_of_registers), REGMON_ERROR);
}

TEST(RegisterMonitorInit, ReturnsErrorForEmptyRegisterList)
{
    std::array<Register, number_of_registers> registers{};
    EXPECT_EQ(RegMon_Init(registers.data(), nullptr, nullptr, nullptr, 0), REGMON_ERROR);
}

class RegisterMonitorReadTest : public ::testing::Test
{
protected:
    std::array<Register, number_of_registers> registers{};

    void SetUp() override
    {
        SetUpRegisters(registers);
        ASSERT_EQ(
            RegMon_Init(registers.data(), nullptr, nullptr, nullptr, number_of_registers),
            REGMON_OK);
    }
};

TEST_F(RegisterMonitorReadTest, ReadsValueFromReadableRegister)
{
    uint32_t value = 0;

    EXPECT_EQ(RegMon_read_register(0, &value), REGMON_OK);
    EXPECT_EQ(value, 0x20100000);
}

TEST_F(RegisterMonitorReadTest, ReturnsErrorForNullDestination)
{
    EXPECT_EQ(RegMon_read_register(0, nullptr), REGMON_ERROR);
}

TEST_F(RegisterMonitorReadTest, ReturnsInvalidAddressForOutOfRangeRegister)
{
    uint32_t value = 0;

    EXPECT_EQ(RegMon_read_register(number_of_registers, &value), REGMON_INVALID_ADDRESS);
}

TEST_F(RegisterMonitorReadTest, ReturnsNotReadableForUnreadableRegister)
{
    uint32_t value = 0;

    EXPECT_EQ(RegMon_read_register(5, &value), REGMON_ADDRESS_NOT_READABLE);
}

class RegisterMonitorWriteTest : public ::testing::Test
{
protected:
    std::array<Register, number_of_registers> registers{};
    std::array<uint32_t, number_of_registers> min_values{};
    std::array<uint32_t, number_of_registers> max_values{};

    void SetUp() override
    {
        SetUpRegisters(registers);
        max_values.fill(0xFFFFFFFFu);
        ASSERT_EQ(
            RegMon_Init(
                registers.data(),
                nullptr,
                min_values.data(),
                max_values.data(),
                number_of_registers),
            REGMON_OK);
    }
};

TEST_F(RegisterMonitorWriteTest, WritesValueToWriteableRegister)
{
    constexpr uint32_t new_value = 1234;

    EXPECT_EQ(RegMon_write_register(2, new_value), REGMON_OK);
    EXPECT_EQ(registers[2].value, new_value);
}

TEST_F(RegisterMonitorWriteTest, ReturnsInvalidAddressForOutOfRangeRegister)
{
    EXPECT_EQ(
        RegMon_write_register(number_of_registers, 1),
        REGMON_INVALID_ADDRESS);
}

TEST_F(RegisterMonitorWriteTest, ReturnsNotWriteableForReadOnlyRegister)
{
    EXPECT_EQ(RegMon_write_register(0, 1), REGMON_ADDRESS_NOT_WRITEABLE);
}

TEST_F(RegisterMonitorWriteTest, RejectsValueBelowMinimum)
{
    min_values[2] = 10;

    EXPECT_EQ(RegMon_write_register(2, 9), REGMON_INVALID_VALUE);
    EXPECT_EQ(registers[2].value, 0);
}

TEST_F(RegisterMonitorWriteTest, RejectsValueAboveMaximum)
{
    max_values[2] = 20;

    EXPECT_EQ(RegMon_write_register(2, 21), REGMON_INVALID_VALUE);
    EXPECT_EQ(registers[2].value, 0);
}
