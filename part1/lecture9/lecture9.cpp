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
struct Register
{
    std::string name;
    uint16_t value;
    uint16_t index;
};
struct Memory
{
    std::string name;
    uint16_t value;
    uint16_t address;
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
static void SetRegister(std::vector<Register> *registers, const Register &reg);
static Register GetRegister(const std::vector<Register> &registers, uint16_t index);
static Memory GetMemory(const std::vector<Memory> &memory, uint16_t address);
static void SetMemory(std::vector<Memory> *memory, const Memory &mem);

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

    std::vector<Register> registers;
    std::vector<Memory> memory;

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

        // Print the left hand side
        int lhsValue = 0;
        int lhsOperandType = decodedInstruction.Operands[0].Type;
        switch (lhsOperandType)
        {
            case Operand_Register:
            {
                Register reg = GetRegister(registers, decodedInstruction.Operands[0].Register.Index);
                reg.name = Sim86_RegisterNameFromOperand(&decodedInstruction.Operands[0].Register);
                SetRegister(&registers, reg);

                lhsValue = reg.value;
                outputFile << reg.name << ", ";
            }
            break;

            case Operand_Memory:
            {
                Memory mem = GetMemory(memory, decodedInstruction.Operands[0].Address.Displacement);
                mem.name = "word";
                SetMemory(&memory, mem);

                lhsValue = mem.value;
                outputFile << mem.name << " [+" << mem.address << "], ";
            }
            break;

            case Operand_Immediate:
                lhsValue = decodedInstruction.Operands[0].Immediate.Value;
                outputFile << "$" << lhsValue << ", ";
                break;

            default:
                printf("lhsOperandType: %d\n", lhsOperandType);
                throw std::runtime_error("First operand is not yet supported");
                break;
        }

        // Print the right hand side and calculate the value
        int rhsOperandType = decodedInstruction.Operands[1].Type;
        int rhsValue = 0;
        if (rhsOperandType == Operand_Register)
        {
            rhsValue = GetRegister(registers, decodedInstruction.Operands[1].Register.Index).value;
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
                if (lhsOperandType == Operand_Register)
                {
                    Register reg = GetRegister(registers, decodedInstruction.Operands[0].Register.Index);
                    reg.value = rhsValue;
                    SetRegister(&registers, reg);
                }
                break;

            case Op_sub:
            {
                if (lhsOperandType == Operand_Register)
                {
                    Register reg = GetRegister(registers, decodedInstruction.Operands[0].Register.Index);
                    reg.value = lhsValue - rhsValue;
                    SetRegister(&registers, reg);

                    flag.SetFlagBasedOnValue(reg.value);
                }
            }
            break;

            case Op_add:
            {
                if (lhsOperandType == Operand_Register)
                {
                    Register reg = GetRegister(registers, decodedInstruction.Operands[0].Register.Index);
                    reg.value = lhsValue + rhsValue;
                    SetRegister(&registers, reg);

                    flag.SetFlagBasedOnValue(reg.value);
                }
            }
            break;

            case Op_cmp:
                flag.SetFlagBasedOnValue(lhsValue - rhsValue);
                break;

            case Op_jne:
                if (flag.value != Flags::Zero)
                {
                    ip += lhsValue;
                }
                break;

            default:
                throw std::runtime_error("Unsupported operation");
        }

        if (lhsOperandType == Operand_Register)
        {
            Register reg = GetRegister(registers, decodedInstruction.Operands[0].Register.Index);

            // Print register value before
            outputFile << reg.name << ":0x" << std::hex << lhsValue << std::dec << " -> ";

            // Print register value after
            outputFile << "0x" << std::hex << reg.value << std::dec << "; ";
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
    for (const auto &reg : registers)
    {
        if (reg.value == 0)
        {
            continue;
        }
        outputFile << "    " << reg.name << ": ";
        outputFile << "0x" << std::setfill('0') << std::setw(4) << std::hex << reg.value << std::dec << " ("
                   << reg.value << ")";
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

static Register GetRegister(const std::vector<Register> &registers, uint16_t index)
{
    for (const auto &entry : registers)
    {
        if (entry.index == index)
        {
            return entry;
        }
    }

    return Register({"", 0, index});
}

static void SetRegister(std::vector<Register> *registers, const Register &reg)
{
    for (auto &entry : *registers)
    {
        if (entry.index == reg.index)
        {
            entry = reg;
            return;
        }
    }

    registers->push_back(reg);
}

static Memory GetMemory(const std::vector<Memory> &memory, uint16_t address)
{
    for (const auto &entry : memory)
    {
        if (entry.address == address)
        {
            return entry;
        }
    }

    // TODO: Create memory
    return Memory({"", 0, address});
}

static void SetMemory(std::vector<Memory> *memory, const Memory &mem)
{
    for (auto &entry : *memory)
    {
        if (entry.address == mem.address)
        {
            entry = mem;
            return;
        }
    }

    memory->push_back(mem);
}
