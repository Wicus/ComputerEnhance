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
    Flags previousFlag = Flags::None;
    Flags currentFlag = Flags::None;
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
    void Set(const int value)
    {
        if (value == 0)
        {
            currentFlag = Flags::Zero;
        }
        else if (value & 0x8000)
        {
            currentFlag = Flags::Sign;
        }
        else
        {
            currentFlag = Flags::None;
        }
    }

    std::string GetName()
    {
        return GetName(currentFlag);
    }

    std::string GetFlagChangeString()
    {
        if (previousFlag != currentFlag)
        {
            std::string string = "; Flags:" + GetName(previousFlag) + " -> " + GetName(currentFlag);
            previousFlag = currentFlag;
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

    InstructionPointer &operator-=(const uint16_t rhs)
    {
        previous = value;
        value -= rhs;
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
        if (decodedInstruction.Operands[0].Type == Operand_Register)
        {
            registerValue.reg = Sim86_RegisterNameFromOperand(&decodedInstruction.Operands[0].Register);
            lhsValue = GetRegisterValueByIndex(registerValues, registerValue.index);
            outputFile << registerValue.reg << ", ";
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
                flag.Set(registerValue.value);
                break;
            case Op_cmp:
                flag.Set(lhsValue - rhsValue);
                break;
            case Op_add:
                registerValue.value = lhsValue + rhsValue;
                flag.Set(registerValue.value);
                break;
            default:
                throw std::runtime_error("Unsupported operation");
        }

        // Print register values before and after
        outputFile << registerValue.reg << ":0x" << std::hex << lhsValue << std::dec << " -> ";
        outputFile << "0x" << std::hex << registerValue.value << std::dec << "; ";

        // Print the instruction pointer
        outputFile << ip.GetChangeString();

        // Print the flags if there is a change
        outputFile << flag.GetFlagChangeString();

        // End of intstruction
        outputFile << '\n';

        // Update the register values
        InsertRegisterValue(&registerValues, registerValue);
    }

    outputFile << "\nFinal Registers\n";
    for (const auto &registerValue : registerValues)
    {
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
