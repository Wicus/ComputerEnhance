#include <exception>
#include <ostream>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include "sim86_shared.h"

#pragma comment(lib, "sim86_shared_debug.lib")

#define MAX_SIZE          100
#define ArrayCount(array) (sizeof(array) / sizeof(array[0]))

enum class Flags
{
    None,
    Zero,
    Sign
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
                return "ZF";
            case Flags::Sign:
                return "SF";
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
            std::string string = "; Flags:" + GetName(previousFlag) + "->" + GetName(currentFlag);
            previousFlag = currentFlag;
            return string;
        }
        else
        {
            return "";
        }
    }
};

static Flag flag;

struct RegisterValue
{
    std::string reg;
    int index;
    int value;
};

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

    outputFile << "bits 16\n\n";

    std::vector<RegisterValue> registerValues;
    int offset = 0;
    while (offset < fileContent.size())
    {
        instruction decodedInstruction;
        Sim86_Decode8086Instruction(fileContent.size() - offset,
                                    (unsigned char *)&fileContent[offset],
                                    &decodedInstruction);
        if (!decodedInstruction.Op)
        {
            throw std::runtime_error("Failed to decode instruction");
        }

        outputFile << Sim86_MnemonicFromOperationType(decodedInstruction.Op) << ' ';

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

        int rhsOperandType = decodedInstruction.Operands[1].Type;
        int rhsValue = 0;

        if (rhsOperandType == Operand_Register)
        {
            rhsValue = GetRegisterValueByIndex(registerValues, decodedInstruction.Operands[1].Register.Index);
            outputFile << Sim86_RegisterNameFromOperand(&decodedInstruction.Operands[1].Register) << "; ";
            std::cout << "rhs is a register\n";
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

        outputFile << registerValue.reg << ":0x" << std::hex << lhsValue << std::dec << " -> ";
        outputFile << "0x" << std::hex << registerValue.value << std::dec;
        outputFile << flag.GetFlagChangeString();
        outputFile << '\n';

        InsertRegisterValue(&registerValues, registerValue);
        offset += decodedInstruction.Size;
    }

    outputFile << "\nFinal Registers\n";
    for (const auto &registerValue : registerValues)
    {
        outputFile << "    " << registerValue.reg << ": ";
        outputFile << "0x" << std::hex << registerValue.value << std::dec << " (" << registerValue.value << ")";
        outputFile << '\n';
    }

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
