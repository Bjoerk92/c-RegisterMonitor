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
constexpr int number_of_registers = 14;
constexpr int first_user_register = 6;

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
void SetUpRegisters(
    std::array<uint32_t, number_of_registers>& register_values,
    std::array<Register_flag_t, number_of_registers>& register_flags)
{
    // Device ID
    register_values[0] = 0x20100000;
    register_flags[0].readable = true;
    register_flags[0].writeable = false;

    // Device model
    register_values[1] = 0x00000001;
    register_flags[1].readable = true;
    register_flags[1].writeable = false;

    // Actual Speed
    register_values[2] = 0;
    register_flags[2].readable = true;
    register_flags[2].writeable = true;

    // Actual position
    register_values[3] = 0;
    register_flags[3].readable = true;
    register_flags[3].writeable = true;

    // Operation bit 
    register_values[4] = 0x00000101;
    register_flags[4].readable = true;
    register_flags[4].writeable = false;

    // Status bit
    register_values[5] = 0x0101;
    register_flags[5].readable = false;
    register_flags[5].writeable = false;

    // User registers
    for (int i = first_user_register; i < number_of_registers; ++i)
    {
        register_values[i] = 0;
        register_flags[i].readable = true;
        register_flags[i].writeable = true;
    }
}
}

// Register monitor initialization tests
// =============================================================================

// Initialization succeeds when given a non-null register list and non-zero size.
TEST(RegisterMonitorInit, ReturnsOkForValidRegisterList)
{
    std::array<uint32_t, number_of_registers> register_values{};
    std::array<Register_flag_t, number_of_registers> register_flags{};
    std::array<uint32_t, number_of_registers> defaults{};
    std::array<uint32_t, number_of_registers> min_values{};
    std::array<uint32_t, number_of_registers> max_values{};

    EXPECT_EQ(
        RegMon_Init(
            register_values.data(),
            defaults.data(),
            min_values.data(),
            max_values.data(),
            register_flags.data(),
            number_of_registers),
        REGMON_OK);
}

// Initialization rejects a null register list.
TEST(RegisterMonitorInit, ReturnsErrorForNullRegisterList)
{
    EXPECT_EQ(
        RegMon_Init(nullptr, nullptr, nullptr, nullptr, nullptr, number_of_registers),
        REGMON_INVALID_PARAM);
}

// Initialization rejects a register list with no entries.
TEST(RegisterMonitorInit, ReturnsErrorForEmptyRegisterList)
{
    std::array<uint32_t, number_of_registers> register_values{};
    std::array<Register_flag_t, number_of_registers> register_flags{};
    EXPECT_EQ(
        RegMon_Init(
            register_values.data(),
            nullptr,
            nullptr,
            nullptr,
            register_flags.data(),
            0),
        REGMON_INVALID_PARAM);
}

// Register read tests
// =============================================================================

// Provides a fresh, initialized register map for each single-register read test.
class RegisterMonitorReadTest : public ::testing::Test
{
protected:
    std::array<uint32_t, number_of_registers> register_values{};
    std::array<Register_flag_t, number_of_registers> register_flags{};

    // Reset the register map and initialize the monitor before every test.
    void SetUp() override
    {
        SetUpRegisters(register_values, register_flags);
        ASSERT_EQ(
            RegMon_Init(
                register_values.data(),
                nullptr,
                nullptr,
                nullptr,
                register_flags.data(),
                number_of_registers),
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

// All eight user registers are initialized as readable and writeable.
TEST_F(RegisterMonitorReadTest, UserRegistersAreReadableAndWriteable)
{
    for (int i = first_user_register; i < number_of_registers; ++i)
    {
        uint32_t value = static_cast<uint32_t>(i);

        EXPECT_EQ(RegMon_read_register(i, &value), REGMON_OK);
        EXPECT_EQ(value, 0);
        EXPECT_EQ(RegMon_write_register(i, static_cast<uint32_t>(i)), REGMON_OK);
        EXPECT_EQ(RegMon_read_register(i, &value), REGMON_OK);
        EXPECT_EQ(value, static_cast<uint32_t>(i));
    }
}

// Reading fails when the destination pointer is null.
TEST_F(RegisterMonitorReadTest, ReturnsErrorForNullDestination)
{
    EXPECT_EQ(RegMon_read_register(0, nullptr), REGMON_INVALID_PARAM);
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
    std::array<uint32_t, number_of_registers> register_values{};
    std::array<Register_flag_t, number_of_registers> register_flags{};

    // Reset the register map and initialize the monitor before every test.
    void SetUp() override
    {
        SetUpRegisters(register_values, register_flags);
        ASSERT_EQ(
            RegMon_Init(
                register_values.data(),
                nullptr,
                nullptr,
                nullptr,
                register_flags.data(),
                number_of_registers),
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
    register_values[5] = 0xABCD;
    register_flags[5].readable = true;
    std::array<uint32_t, 2> values{};

    EXPECT_EQ(RegMon_read_register_block(4, values.size(), values.data()), REGMON_OK);
    EXPECT_EQ(values[0], 0x00000101);
    EXPECT_EQ(values[1], 0xABCD);
}

// Reading a block fails if the destination buffer is null.
TEST_F(RegisterMonitorReadBlockTest, ReturnsErrorForNullDestination)
{
    EXPECT_EQ(RegMon_read_register_block(0, 1, nullptr), REGMON_INVALID_PARAM);
}

// Reading a block with zero registers is rejected.
TEST_F(RegisterMonitorReadBlockTest, ReturnsErrorForEmptyBlock)
{
    uint32_t value = 0;

    EXPECT_EQ(RegMon_read_register_block(0, 0, &value), REGMON_INVALID_PARAM);
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
    std::array<uint32_t, number_of_registers> register_values{};
    std::array<Register_flag_t, number_of_registers> register_flags{};
    std::array<uint32_t, number_of_registers> min_values{};
    std::array<uint32_t, number_of_registers> max_values{};

    // Reset the register map and initialize the monitor before every test.
    void SetUp() override
    {
        SetUpRegisters(register_values, register_flags);
        max_values.fill(0xFFFFFFFFu);
        ASSERT_EQ(
            RegMon_Init(
                register_values.data(),
                nullptr,
                min_values.data(),
                max_values.data(),
                register_flags.data(),
                number_of_registers),
            REGMON_OK);
    }
};

// Writing a permitted value updates a writeable register.
TEST_F(RegisterMonitorWriteTest, WritesValueToWriteableRegister)
{
    constexpr uint32_t new_value = 1234;

    EXPECT_EQ(RegMon_write_register(2, new_value), REGMON_OK);
    EXPECT_EQ(register_values[2], new_value);
}

// Writing a register configured with a callback invokes it with the written value.
TEST_F(RegisterMonitorWriteTest, InvokesConfiguredWriteCallback)
{
    constexpr uint8_t callback_number = 0;
    constexpr uint32_t new_value = 4321;
    write_callback_value = 0;
    write_callback_call_count = 0;

    register_flags[2].Write_callback = true;
    register_flags[2].callback_number = callback_number;
    ASSERT_EQ(RegMON_SetWriteCallback(callback_number, CaptureWriteCallback), REGMON_OK);

    EXPECT_EQ(RegMon_write_register(2, new_value), REGMON_OK);
    EXPECT_EQ(register_values[2], new_value);
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
    EXPECT_EQ(register_values[2], 0);
}

// Writing a value above the configured maximum is rejected without changing it.
TEST_F(RegisterMonitorWriteTest, RejectsValueAboveMaximum)
{
    max_values[2] = 20;

    EXPECT_EQ(RegMon_write_register(2, 21), REGMON_INVALID_VALUE);
    EXPECT_EQ(register_values[2], 0);
}

// Register block write tests
// =============================================================================

// Provides an initialized register map and permissive value limits for block writes.
class RegisterMonitorWriteBlockTest : public ::testing::Test
{
protected:
    std::array<uint32_t, number_of_registers> register_values{};
    std::array<Register_flag_t, number_of_registers> register_flags{};
    std::array<uint32_t, number_of_registers> min_values{};
    std::array<uint32_t, number_of_registers> max_values{};

    // Reset the register map and initialize the monitor before every test.
    void SetUp() override
    {
        SetUpRegisters(register_values, register_flags);
        max_values.fill(0xFFFFFFFFu);
        ASSERT_EQ(
            RegMon_Init(
                register_values.data(),
                nullptr,
                min_values.data(),
                max_values.data(),
                register_flags.data(),
                number_of_registers),
            REGMON_OK);
    }
};

// Writing a block updates each writable register with the corresponding value.
TEST_F(RegisterMonitorWriteBlockTest, WritesValuesToWriteableRegisterBlock)
{
    std::array<uint32_t, 2> values{1234, 5678};

    EXPECT_EQ(RegMON_write_register_block(2, values.size(), values.data()), REGMON_OK);
    EXPECT_EQ(register_values[2], values[0]);
    EXPECT_EQ(register_values[3], values[1]);
}

// A valid block write can include the final zero-based register index.
TEST_F(RegisterMonitorWriteBlockTest, WritesBlockEndingAtLastRegister)
{
    register_flags[4].writeable = true;
    register_flags[5].writeable = true;
    std::array<uint32_t, 2> values{1234, 5678};

    EXPECT_EQ(RegMON_write_register_block(4, values.size(), values.data()), REGMON_OK);
    EXPECT_EQ(register_values[4], values[0]);
    EXPECT_EQ(register_values[5], values[1]);
}

// Writing a block fails when the input values pointer is null.
TEST_F(RegisterMonitorWriteBlockTest, ReturnsErrorForNullValues)
{
    EXPECT_EQ(RegMON_write_register_block(2, 1, nullptr), REGMON_INVALID_PARAM);
}

// Writing a block with zero registers is rejected.
TEST_F(RegisterMonitorWriteBlockTest, ReturnsErrorForEmptyBlock)
{
    uint32_t value = 1;

    EXPECT_EQ(RegMON_write_register_block(2, 0, &value), REGMON_INVALID_PARAM);
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
    EXPECT_EQ(register_values[2], 0);
    EXPECT_EQ(register_values[3], 0);
}

// Register group test
// =============================================================================

// Provides an initialized register map and permissive value limits for block writes.
class RegisterMonitorGroupTest : public ::testing::Test
{
protected:
    std::array<uint32_t, number_of_registers> register_values{};
    std::array<Register_flag_t, number_of_registers> register_flags{};
    std::array<uint32_t, number_of_registers> min_values{};
    std::array<uint32_t, number_of_registers> max_values{};
    std::array<uint16_t, MAX_GROUP_SIZE> Group{
        6, 7, 8, 9, 10, 11, 12, 13
    };
    // Reset the register map and initialize the monitor before every test.
    void SetUp() override
    {
        SetUpRegisters(register_values, register_flags);
        max_values.fill(0xFFFFFFFFu);
        ASSERT_EQ(
            RegMon_Init(
                register_values.data(),
                nullptr,
                min_values.data(),
                max_values.data(),
                register_flags.data(),
                number_of_registers),
            REGMON_OK);
    }
};

TEST_F(RegisterMonitorGroupTest, NullPointer)
{
    ASSERT_EQ(RegMon_SetupGroup(nullptr, 0, Register_group_0), REGMON_INVALID_PARAM);
}

TEST_F(RegisterMonitorGroupTest, ZeroRegisters)
{
    ASSERT_EQ(RegMon_SetupGroup(this->Group.data(), 0, Register_group_0), REGMON_INVALID_PARAM);
}

TEST_F(RegisterMonitorGroupTest, ExceedRegisters)
{
    ASSERT_EQ(RegMon_SetupGroup(this->Group.data(), MAX_GROUP_SIZE + 1, Register_group_0), REGMON_GROUP_EXCEED_SIZE);
}

TEST_F(RegisterMonitorGroupTest, WriteReadGroup)
{
    RegMon_group_t ret_values = {};
    ret_values.group = Register_group_0; 

    RegMon_group_t values = {};
    values.group = Register_group_0; 
    values.reg_values[0] = 1;
    values.reg_values[1] = 1;
    values.reg_values[2] = 1;
    values.reg_values[3] = 1;
    values.reg_values[4] = 1;
    values.reg_values[5] = 1;
    values.reg_values[6] = 1;
    values.reg_values[7] = 1;
    
    ASSERT_EQ(RegMon_SetupGroup(this->Group.data(), this->Group.size(), Register_group_0), REGMON_OK);
    ASSERT_EQ(RegMON_WriteGroup(&values), REGMON_OK);
    ASSERT_EQ(RegMON_ReadGroup(&ret_values), REGMON_OK);

    EXPECT_EQ(ret_values.group, values.group);
    for (size_t i = 0; i < MAX_GROUP_SIZE; ++i)
    {
        EXPECT_EQ(ret_values.reg_values[i], values.reg_values[i]);
    }

}