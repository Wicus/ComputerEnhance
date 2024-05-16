#include <cstdint>
#include <cstdio>
#include <exception>
#include <iomanip>
#include <memory>
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

struct Access
{
    std::string name = "";
    int32_t value = 0;
    operand_type type = Operand_None;
    union
    {
        uint16_t index = 0;
        uint16_t address;
    };
    Access() = default;
    Access(const operand_type type) : type(type)
    {
        if (type == Operand_Memory)
        {
            address = 0;
        }
        else
        {
            index = 0;
        }
    }
};

struct RegisterAccess
{
    std::vector<std::shared_ptr<Access> > registers = std::vector<std::shared_ptr<Access> >(1024);

    std::shared_ptr<Access> Get(uint16_t index)
    {
        if (registers.at(index) == NULL)
        {
            registers.at(index) = std::make_shared<Access>(Operand_Register);
            registers.at(index)->index = index;
        }
        return registers.at(index);
    }
};

struct MemoryAccess
{
    std::vector<std::shared_ptr<Access> > memory = std::vector<std::shared_ptr<Access> >(1024 * 1024);

    std::shared_ptr<Access> Get(uint16_t address)
    {
        if (memory.at(address) == NULL)
        {
            memory.at(address) = std::make_shared<Access>(Operand_Memory);
            memory.at(address)->address = address;
        }
        return memory.at(address);
    }
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

    RegisterAccess registerAccess;
    MemoryAccess memoryAccess;

    InstructionPointer ip;
    Flag flag;
    int clocks = 0;
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

        // Left hand side
        std::shared_ptr<Access> lhs;
        switch (decodedInstruction.Operands[0].Type)
        {
            case Operand_Register:
            {
                lhs = registerAccess.Get(decodedInstruction.Operands[0].Register.Index);
                lhs->name = Sim86_RegisterNameFromOperand(&decodedInstruction.Operands[0].Register);
                outputFile << lhs->name << ", ";
            }
            break;

            case Operand_Memory:
            {
                std::shared_ptr<Access> lhsReg = registerAccess.Get(decodedInstruction.Operands[0].Register.Index);
                uint16_t memAddress = decodedInstruction.Operands[0].Address.Displacement + lhsReg->value;
                lhs = memoryAccess.Get(memAddress);

                outputFile << lhs->name << " [" << lhsReg->name << "+"
                           << decodedInstruction.Operands[0].Address.Displacement << "], ";
                lhs->name = "word";
            }
            break;

            case Operand_Immediate:
            {
                lhs = std::make_shared<Access>(Operand_Immediate);
                lhs->value = decodedInstruction.Operands[0].Immediate.Value;
                outputFile << "$" << lhs->value << ", ";
            }
            break;

            default:
                printf("lhsOperandType: %d\n", lhs->type);
                throw std::runtime_error("First operand is not yet supported");
                break;
        }

        // Right hand side
        std::shared_ptr<Access> rhs;
        switch (decodedInstruction.Operands[1].Type)
        {
            case Operand_Register:
            {
                rhs = registerAccess.Get(decodedInstruction.Operands[1].Register.Index);
                outputFile << Sim86_RegisterNameFromOperand(&decodedInstruction.Operands[1].Register) << "; ";
            }
            break;


            case Operand_Memory:
            {
                rhs = memoryAccess.Get(decodedInstruction.Operands[1].Address.Displacement);
                outputFile << "[+" << rhs->address << "]; ";
            }
            break;

            case Operand_Immediate:
            {
                rhs = std::make_shared<Access>(Operand_Immediate);
                rhs->value = decodedInstruction.Operands[1].Immediate.Value;
                outputFile << rhs->value << "; ";
            }
            break;

            case Operand_None:
                break;

            default:
                printf("rhsOperandType: %d\n", rhs->type);
                throw std::runtime_error("Second operand is not yet supported");
                break;
        }

        // Perform the operation
        switch (decodedInstruction.Op)
        {
            case Op_mov:
                if (lhs->type == Operand_Register)
                {
                    lhs->value = rhs->value;
                }
                else if (lhs->type == Operand_Memory)
                {
                    lhs->value = rhs->value;
                }
                break;

            case Op_sub:
            {
                if (lhs->type == Operand_Register)
                {
                    lhs->value -= rhs->value;

                    flag.SetFlagBasedOnValue(lhs->value);
                }
            }
            break;

            case Op_add:
            {
                if (lhs->type == Operand_Register)
                {
                    lhs->value += rhs->value;

                    flag.SetFlagBasedOnValue(lhs->value);
                }
            }
            break;

            case Op_cmp:
                flag.SetFlagBasedOnValue(lhs->value - rhs->value);
                break;

            case Op_jne:
                if (flag.value != Flags::Zero)
                {
                    ip += lhs->value;
                }
                break;

            default:
                throw std::runtime_error("Unsupported operation");
        }

        if (lhs->type == Operand_Register && rhs->type == Operand_Immediate)
        {
            int cycles = 4;
            clocks += cycles;
            outputFile << "Clocks: + " << cycles << " = " << clocks << " | ";
        }
        else if (lhs->type == Operand_Register && rhs->type == Operand_Register)
        {
            int cycles = 2;
            clocks += cycles;
            outputFile << "Clocks: + " << cycles << " = " << clocks << " | ";
        }
        else if (lhs->type == Operand_Register && rhs->type == Operand_Memory)
        {
            int cycles = 8;
            int effectiveAddressCalculation = 6;
            clocks += cycles + effectiveAddressCalculation;
            outputFile << "Clocks: + " << cycles + effectiveAddressCalculation << " = " << clocks << " ( " << cycles
                       << " + " << effectiveAddressCalculation << "ea ) | ";
        }

        if (lhs->type == Operand_Register)
        {
            // Print register value before
            outputFile << lhs->name << ":0x" << std::hex << lhs->value << std::dec << " -> ";

            // Print register value after
            outputFile << "0x" << std::hex << lhs->value << std::dec << "; ";
        }

        // Print the instruction pointer
        outputFile << ip.GetChangeString();

        if (lhs->type == Operand_Register)
        {
            // Print the flags if there is a change
            outputFile << flag.GetFlagChangeString();
        }

        // End of intstruction
        outputFile << '\n';
    }

    outputFile << "\nFinal Registers\n";
    for (int i = 0; i < registerAccess.registers.size(); i++)
    {
        std::shared_ptr<Access> access = registerAccess.registers.at(i);
        if (access == NULL || access->value == 0)
        {
            continue;
        }

        outputFile << "    " << registerAccess.registers.at(i)->name << ": ";
        outputFile << "0x" << std::setfill('0') << std::setw(4) << std::hex << registerAccess.registers.at(i)->value
                   << std::dec << " (" << registerAccess.registers.at(i)->value << ")";
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
