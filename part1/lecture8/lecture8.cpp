#include <cstdint>
#include <exception>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include "sim86_shared.h"

#pragma comment(lib, "sim86_shared_debug.lib")

// Constants
#define MAX_SIZE          100
#define ArrayCount(array) (sizeof(array) / sizeof(array[0]))

// Types
enum class Flags
{
    None,
    Zero,
    Sign
};
struct RegisterValue
{
    std::string reg;
    int index;
    uint16_t value;
};
class Flag
{
private:
    Flags previous = Flags::None;

    std::string GetName(Flags flag)
    {
        switch (flag)
        {
            case Flags::Zero:
                return "Z";
            case Flags::Sign:
                return "S";
            default:
                return "";
        }
    }

public:
    Flags value = Flags::None;

    void SetFlagBasedOnValue(const uint16_t value)
    {
        if (value == 0)
        {
            this->value = Flags::Zero;
        }
        else if (value & 0x8000)
        {
            this->value = Flags::Sign;
        }
        else
        {
            this->value = Flags::None;
        }
    }

    std::string GetName()
    {
        return GetName(value);
    }

    std::string GetFlagChangeString()
    {
        if (previous != value)
        {
            std::string string = "; Flags:" + GetName(previous) + " -> " + GetName(value);
            previous = value;
            return string;
        }
        else
        {
            return "";
        }
    }
};

class InstructionPointer
{
private:
    uint16_t previous = 0;

public:
    uint16_t value = 0;

    InstructionPointer &operator+=(const uint16_t rhs)
    {
        previous = value;
        value += rhs;
        return *this;
    }

    std::string GetChangeString()
    {
        std::ostringstream oss;
        oss << "ip:0x" << std::hex << previous << " -> 0x" << value;
        return oss.str();
    }
};

// Function prototypes
void Application(int argc, char *argv[]);
static std::vector<char> ReadFile(const std::string &filePath);
static int GetRegisterValueByIndex(const std::vector<RegisterValue> &registerValues, int index);
static void InsertRegisterValue(std::vector<RegisterValue> *registerValues, const RegisterValue &regValue);


int main(int argc, char *argv[])
{
    try
    {
        Application(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}

void Application(int argc, char *argv[])
{
    if (argc < 2)
    {
        throw std::runtime_error("Usage: " + std::string(argv[0]) + " <filename>");
    }

    std::vector<char> fileContent = ReadFile(argv[1]);

    instruction_table table;
    Sim86_Get8086InstructionTable(&table);

    std::ofstream outputFile("output.txt");
    if (!outputFile)
    {
        throw std::runtime_error("Failed to open output file");
    }

    std::vector<RegisterValue> registerValues;

    InstructionPointer ip;
    Flag flag;
    while (ip.value < fileContent.size())
    {
        instruction decodedInstruction;
        Sim86_Decode8086Instruction(fileContent.size() - ip.value,
                                    (unsigned char *)&fileContent[ip.value],
                                    &decodedInstruction);
        if (!decodedInstruction.Op)
        {
            throw std::runtime_error("Failed to decode instruction");
        }

        // Move the instruction pointer
        ip += decodedInstruction.Size;

        // Print the mnemonic
        outputFile << Sim86_MnemonicFromOperationType(decodedInstruction.Op) << ' ';

        // Print the left hand side value
        RegisterValue registerValue;
        registerValue.index = decodedInstruction.Operands[0].Register.Index;
        int lhsValue = 0;
        int lhsOperandType = decodedInstruction.Operands[0].Type;
        if (lhsOperandType == Operand_Register)
        {
            registerValue.reg = Sim86_RegisterNameFromOperand(&decodedInstruction.Operands[0].Register);
            lhsValue = GetRegisterValueByIndex(registerValues, registerValue.index);
            outputFile << registerValue.reg << ", ";
        }
        else if (lhsOperandType == Operand_Immediate)
        {
            lhsValue = decodedInstruction.Operands[0].Immediate.Value;
            outputFile << "$" << lhsValue << ", ";
        }
        else
        {
            throw std::runtime_error("First operand must be a register");
        }

        // Print the right hand side value and calculate the result
        int rhsOperandType = decodedInstruction.Operands[1].Type;
        int rhsValue = 0;
        if (rhsOperandType == Operand_Register)
        {
            rhsValue = GetRegisterValueByIndex(registerValues, decodedInstruction.Operands[1].Register.Index);
            outputFile << Sim86_RegisterNameFromOperand(&decodedInstruction.Operands[1].Register) << "; ";
        }
        else if (rhsOperandType == Operand_Immediate)
        {
            rhsValue = decodedInstruction.Operands[1].Immediate.Value;
            outputFile << rhsValue << "; ";
        }

        switch (decodedInstruction.Op)
        {
            case Op_mov:
                registerValue.value = rhsValue;
                break;
            case Op_sub:
                registerValue.value = lhsValue - rhsValue;
                flag.SetFlagBasedOnValue(registerValue.value);
                break;
            case Op_cmp:
                flag.SetFlagBasedOnValue(lhsValue - rhsValue);
                break;
            case Op_add:
                registerValue.value = lhsValue + rhsValue;
                flag.SetFlagBasedOnValue(registerValue.value);
                break;
            case Op_jne:
                if (flag.value != Flags::Zero)
                {
                    printf("lhsValue: %d, rhsValue: %d\n", lhsValue, rhsValue);
                    printf("ip: %d\n", ip.value);
                    ip += lhsValue;
                }
                break;
            default:
                throw std::runtime_error("Unsupported operation");
        }

        if (lhsOperandType == Operand_Register)
        {
            // Print register values before and after
            outputFile << registerValue.reg << ":0x" << std::hex << lhsValue << std::dec << " -> ";
            outputFile << "0x" << std::hex << registerValue.value << std::dec << "; ";

            // Update the register values
            InsertRegisterValue(&registerValues, registerValue);
        }

        // Print the instruction pointer
        outputFile << ip.GetChangeString();

        if (lhsOperandType == Operand_Register)
        {
            // Print the flags if there is a change
            outputFile << flag.GetFlagChangeString();
        }

        // End of intstruction
        outputFile << '\n';
    }

    outputFile << "\nFinal Registers\n";
    for (const auto &registerValue : registerValues)
    {
        if (registerValue.value == 0)
        {
            continue;
        }
        outputFile << "    " << registerValue.reg << ": ";
        outputFile << "0x" << std::setfill('0') << std::setw(4) << std::hex << registerValue.value << std::dec << " ("
                   << registerValue.value << ")";
        outputFile << '\n';
    }

    outputFile << "    ip: 0x" << std::setfill('0') << std::setw(4) << std::hex << ip.value << std::dec << " ("
               << ip.value << ")" << '\n';
    outputFile << "Flags: " << flag.GetName() << '\n';
}


static std::vector<char> ReadFile(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file)
    {
        throw std::runtime_error("Unable to open file: " + filePath);
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(fileSize);
    if (!file.read(buffer.data(), fileSize))
    {
        throw std::runtime_error("Failed to read file: " + filePath);
    }

    return buffer;
}

static int GetRegisterValueByIndex(const std::vector<RegisterValue> &registerValues, int index)
{
    for (const auto &registerValue : registerValues)
    {
        if (registerValue.index == index)
        {
            return registerValue.value;
        }
    }

    return 0;
}

static void InsertRegisterValue(std::vector<RegisterValue> *registerValues, const RegisterValue &regValue)
{
    for (auto &registerValue : *registerValues)
    {
        if (registerValue.index == regValue.index)
        {
            registerValue = regValue;
            return;
        }
    }

    registerValues->push_back(regValue);
}
