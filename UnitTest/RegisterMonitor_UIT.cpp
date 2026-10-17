/**
 * @file RegisterMonitor_UIT.cpp
 * @author Anders (anders.bjoerk.unf@gmail.com)
 * @brief Unit tests for the Register Monitor API.
 * @version 0.1
 * @date 2026-10-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <gtest/gtest.h>
#include <array>
#include "RegisterMonitor.h"

namespace
{
// Number of registers shared by the test register maps.
constexpr int number_of_registers = 6;

// State captured by the write-callback test.
uint32_t write_callback_value = 0;
uint32_t write_callback_call_count = 0;

// Store the most recent callback value and count its invocations.
void CaptureWriteCallback(uint32_t value)
{
    write_callback_value = value;
    ++write_callback_call_count;
}

// Populate a register map with readable, writable, and restricted examples.
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
}

// Register monitor initialization tests
// =============================================================================

// Initialization succeeds when given a non-null register list and non-zero size.
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

// Initialization rejects a null register list.
TEST(RegisterMonitorInit, ReturnsErrorForNullRegisterList)
{
    EXPECT_EQ(RegMon_Init(nullptr, nullptr, nullptr, nullptr, number_of_registers), REGMON_ERROR);
}

// Initialization rejects a register list with no entries.
TEST(RegisterMonitorInit, ReturnsErrorForEmptyRegisterList)
{
    std::array<Register, number_of_registers> registers{};
    EXPECT_EQ(RegMon_Init(registers.data(), nullptr, nullptr, nullptr, 0), REGMON_ERROR);
}

// Register read tests
// =============================================================================

// Provides a fresh, initialized register map for each single-register read test.
class RegisterMonitorReadTest : public ::testing::Test
{
protected:
    std::array<Register, number_of_registers> registers{};

    // Reset the register map and initialize the monitor before every test.
    void SetUp() override
    {
        SetUpRegisters(registers);
        ASSERT_EQ(
            RegMon_Init(registers.data(), nullptr, nullptr, nullptr, number_of_registers),
            REGMON_OK);
    }
};

// Reading a readable register returns its current value.
TEST_F(RegisterMonitorReadTest, ReadsValueFromReadableRegister)
{
    uint32_t value = 0;

    EXPECT_EQ(RegMon_read_register(0, &value), REGMON_OK);
    EXPECT_EQ(value, 0x20100000);
}

// Reading fails when the destination pointer is null.
TEST_F(RegisterMonitorReadTest, ReturnsErrorForNullDestination)
{
    EXPECT_EQ(RegMon_read_register(0, nullptr), REGMON_ERROR);
}

// Reading fails when the register index is outside the configured list.
TEST_F(RegisterMonitorReadTest, ReturnsInvalidAddressForOutOfRangeRegister)
{
    uint32_t value = 0;

    EXPECT_EQ(RegMon_read_register(number_of_registers, &value), REGMON_INVALID_ADDRESS);
}

// Reading a register without its readable flag returns the appropriate error.
TEST_F(RegisterMonitorReadTest, ReturnsNotReadableForUnreadableRegister)
{
    uint32_t value = 0;

    EXPECT_EQ(RegMon_read_register(5, &value), REGMON_ADDRESS_NOT_READABLE);
}

// Register block read tests
// =============================================================================

// Provides a fresh, initialized register map for each block-read test.
class RegisterMonitorReadBlockTest : public ::testing::Test
{
protected:
    std::array<Register, number_of_registers> registers{};

    // Reset the register map and initialize the monitor before every test.
    void SetUp() override
    {
        SetUpRegisters(registers);
        ASSERT_EQ(
            RegMon_Init(registers.data(), nullptr, nullptr, nullptr, number_of_registers),
            REGMON_OK);
    }
};

// Reading a block returns each readable register value in index order.
TEST_F(RegisterMonitorReadBlockTest, ReadsValuesFromReadableRegisterBlock)
{
    std::array<uint32_t, 2> values{};

    EXPECT_EQ(RegMon_read_register_block(0, values.size(), values.data()), REGMON_OK);
    EXPECT_EQ(values[0], 0x20100000);
    EXPECT_EQ(values[1], 0x00000001);
}

// A valid block can include the final zero-based register index.
TEST_F(RegisterMonitorReadBlockTest, ReadsBlockEndingAtLastRegister)
{
    registers[5].value = 0xABCD;
    registers[5].flags.readable = true;
    std::array<uint32_t, 2> values{};

    EXPECT_EQ(RegMon_read_register_block(4, values.size(), values.data()), REGMON_OK);
    EXPECT_EQ(values[0], 0x00000101);
    EXPECT_EQ(values[1], 0xABCD);
}

// Reading a block fails if the destination buffer is null.
TEST_F(RegisterMonitorReadBlockTest, ReturnsErrorForNullDestination)
{
    EXPECT_EQ(RegMon_read_register_block(0, 1, nullptr), REGMON_ERROR);
}

// Reading a block with zero registers is rejected.
TEST_F(RegisterMonitorReadBlockTest, ReturnsErrorForEmptyBlock)
{
    uint32_t value = 0;

    EXPECT_EQ(RegMon_read_register_block(0, 0, &value), REGMON_ERROR);
}

// Reading fails if the requested block extends beyond the register list.
TEST_F(RegisterMonitorReadBlockTest, ReturnsInvalidAddressForOutOfRangeBlock)
{
    std::array<uint32_t, 2> values{};

    EXPECT_EQ(
        RegMon_read_register_block(number_of_registers - 1, values.size(), values.data()),
        REGMON_INVALID_ADDRESS);
}

// A block read fails if any register in the block is not readable.
TEST_F(RegisterMonitorReadBlockTest, ReturnsNotReadableWhenBlockContainsUnreadableRegister)
{
    std::array<uint32_t, 2> values{};

    EXPECT_EQ(
        RegMon_read_register_block(4, values.size(), values.data()),
        REGMON_ADDRESS_NOT_READABLE);
}

// Register write tests
// =============================================================================

// Provides an initialized register map and permissive value limits for write tests.
class RegisterMonitorWriteTest : public ::testing::Test
{
protected:
    std::array<Register, number_of_registers> registers{};
    std::array<uint32_t, number_of_registers> min_values{};
    std::array<uint32_t, number_of_registers> max_values{};

    // Reset the register map and initialize the monitor before every test.
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

// Writing a permitted value updates a writeable register.
TEST_F(RegisterMonitorWriteTest, WritesValueToWriteableRegister)
{
    constexpr uint32_t new_value = 1234;

    EXPECT_EQ(RegMon_write_register(2, new_value), REGMON_OK);
    EXPECT_EQ(registers[2].value, new_value);
}

// Writing a register configured with a callback invokes it with the written value.
TEST_F(RegisterMonitorWriteTest, InvokesConfiguredWriteCallback)
{
    constexpr uint8_t callback_number = 0;
    constexpr uint32_t new_value = 4321;
    write_callback_value = 0;
    write_callback_call_count = 0;

    registers[2].flags.Write_callback = true;
    registers[2].flags.callback_number = callback_number;
    ASSERT_EQ(RegMON_SetWriteCallback(callback_number, CaptureWriteCallback), REGMON_OK);

    EXPECT_EQ(RegMon_write_register(2, new_value), REGMON_OK);
    EXPECT_EQ(registers[2].value, new_value);
    EXPECT_EQ(write_callback_call_count, 1);
    EXPECT_EQ(write_callback_value, new_value);
}

// Writing fails when the register index is outside the configured list.
TEST_F(RegisterMonitorWriteTest, ReturnsInvalidAddressForOutOfRangeRegister)
{
    EXPECT_EQ(
        RegMon_write_register(number_of_registers, 1),
        REGMON_INVALID_ADDRESS);
}

// Writing to a register without its writeable flag returns an error.
TEST_F(RegisterMonitorWriteTest, ReturnsNotWriteableForReadOnlyRegister)
{
    EXPECT_EQ(RegMon_write_register(0, 1), REGMON_ADDRESS_NOT_WRITEABLE);
}

// Writing a value below the configured minimum is rejected without changing it.
TEST_F(RegisterMonitorWriteTest, RejectsValueBelowMinimum)
{
    min_values[2] = 10;

    EXPECT_EQ(RegMon_write_register(2, 9), REGMON_INVALID_VALUE);
    EXPECT_EQ(registers[2].value, 0);
}

// Writing a value above the configured maximum is rejected without changing it.
TEST_F(RegisterMonitorWriteTest, RejectsValueAboveMaximum)
{
    max_values[2] = 20;

    EXPECT_EQ(RegMon_write_register(2, 21), REGMON_INVALID_VALUE);
    EXPECT_EQ(registers[2].value, 0);
}

// Register block write tests
// =============================================================================

// Provides an initialized register map and permissive value limits for block writes.
class RegisterMonitorWriteBlockTest : public ::testing::Test
{
protected:
    std::array<Register, number_of_registers> registers{};
    std::array<uint32_t, number_of_registers> min_values{};
    std::array<uint32_t, number_of_registers> max_values{};

    // Reset the register map and initialize the monitor before every test.
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

// Writing a block updates each writable register with the corresponding value.
TEST_F(RegisterMonitorWriteBlockTest, WritesValuesToWriteableRegisterBlock)
{
    std::array<uint32_t, 2> values{1234, 5678};

    EXPECT_EQ(RegMON_write_register_block(2, values.size(), values.data()), REGMON_OK);
    EXPECT_EQ(registers[2].value, values[0]);
    EXPECT_EQ(registers[3].value, values[1]);
}

// A valid block write can include the final zero-based register index.
TEST_F(RegisterMonitorWriteBlockTest, WritesBlockEndingAtLastRegister)
{
    registers[4].flags.writeable = true;
    registers[5].flags.writeable = true;
    std::array<uint32_t, 2> values{1234, 5678};

    EXPECT_EQ(RegMON_write_register_block(4, values.size(), values.data()), REGMON_OK);
    EXPECT_EQ(registers[4].value, values[0]);
    EXPECT_EQ(registers[5].value, values[1]);
}

// Writing a block fails when the input values pointer is null.
TEST_F(RegisterMonitorWriteBlockTest, ReturnsErrorForNullValues)
{
    EXPECT_EQ(RegMON_write_register_block(2, 1, nullptr), REGMON_ERROR);
}

// Writing a block with zero registers is rejected.
TEST_F(RegisterMonitorWriteBlockTest, ReturnsErrorForEmptyBlock)
{
    uint32_t value = 1;

    EXPECT_EQ(RegMON_write_register_block(2, 0, &value), REGMON_ERROR);
}

// Writing fails when the requested block extends beyond the register list.
TEST_F(RegisterMonitorWriteBlockTest, ReturnsInvalidAddressForOutOfRangeBlock)
{
    std::array<uint32_t, 2> values{1, 2};

    EXPECT_EQ(
        RegMON_write_register_block(number_of_registers - 1, values.size(), values.data()),
        REGMON_INVALID_ADDRESS);
}

// A block write fails if any register in the block is not writable.
TEST_F(RegisterMonitorWriteBlockTest, ReturnsNotWriteableWhenBlockContainsReadOnlyRegister)
{
    std::array<uint32_t, 3> values{1, 2, 3};

    EXPECT_EQ(
        RegMON_write_register_block(2, values.size(), values.data()),
        REGMON_ADDRESS_NOT_WRITEABLE);
}

// If a block value is outside its limits, no registers in the block are changed.
TEST_F(RegisterMonitorWriteBlockTest, RejectsOutOfRangeValueWithoutChangingBlock)
{
    min_values[3] = 10;
    std::array<uint32_t, 2> values{1234, 9};

    EXPECT_EQ(
        RegMON_write_register_block(2, values.size(), values.data()),
        REGMON_INVALID_VALUE);
    EXPECT_EQ(registers[2].value, 0);
    EXPECT_EQ(registers[3].value, 0);
}
