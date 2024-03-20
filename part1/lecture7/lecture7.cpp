#include <cstdint>
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

struct RegisterValue
{
    std::string reg;
    int32_t index;
    int32_t value;
};

void Application(int argc, char *argv[]);
static std::vector<char> ReadFile(const std::string &filePath);
static int32_t GetRegisterValueByIndex(const std::vector<RegisterValue> &registerValues, int32_t index);
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
    std::cout << "8086 Instruction Instruction Encoding Count: " << table.EncodingCount << '\n';

    std::ofstream outputFile("output.txt");
    if (!outputFile)
    {
        throw std::runtime_error("Failed to open output file");
    }

    outputFile << "bits 16\n\n";

    std::vector<RegisterValue> registerValues;
    int32_t offset = 0;
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

        int32_t currentRegisterValue = 0;
        if (decodedInstruction.Operands[0].Type == Operand_Register)
        {
            registerValue.reg = Sim86_RegisterNameFromOperand(&decodedInstruction.Operands[0].Register);
            currentRegisterValue = GetRegisterValueByIndex(registerValues, registerValue.index);
            outputFile << registerValue.reg << ", ";
        }
        else
        {
            throw std::runtime_error("First operand must be a register");
        }

        if (decodedInstruction.Operands[1].Type == Operand_Register)
        {
            registerValue.value =
                GetRegisterValueByIndex(registerValues, decodedInstruction.Operands[1].Register.Index);
            outputFile << Sim86_RegisterNameFromOperand(&decodedInstruction.Operands[1].Register) << "; ";
        }
        else if (decodedInstruction.Operands[1].Type == Operand_Immediate)
        {
            registerValue.value = decodedInstruction.Operands[1].Immediate.Value;
            outputFile << registerValue.value << "; ";
        }

        outputFile << registerValue.reg << ":0x" << std::hex << currentRegisterValue << " -> ";
        outputFile << "0x" << registerValue.value << '\n';

        InsertRegisterValue(&registerValues, registerValue);
        offset += decodedInstruction.Size;
    }

    outputFile << "\nFinal Registers\n";
    for (const auto &registerValue : registerValues)
    {
        outputFile << "    " << registerValue.reg << ": ";
        outputFile << "0x" << std::hex << registerValue.value << " (" << std::dec << registerValue.value << ")\n";
    }
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

static int32_t GetRegisterValueByIndex(const std::vector<RegisterValue> &registerValues, int32_t index)
{
    if (index < 1)
    {
        throw std::runtime_error("Invalid register index");
    }

    // Does not exist yet, so the register value is 0.
    if (index > registerValues.size())
    {
        return 0;
    }

    // Adjust the index to start from 1
    int adjustedIndex = index - 1;

    return registerValues[adjustedIndex].value;
}

static void InsertRegisterValue(std::vector<RegisterValue> *registerValues, const RegisterValue &regValue)
{
    if (regValue.index < 1)
    {
        throw std::runtime_error("Invalid register index");
    }

    // Adjust the index to start from 1
    int32_t adjustedIndex = regValue.index - 1;

    if (adjustedIndex < registerValues->size())
    {
        registerValues->at(adjustedIndex) = regValue;
    }
    else
    {
        registerValues->push_back(regValue);
    }
}
