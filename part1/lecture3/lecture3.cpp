#include <cstdint>
#include <cstdlib>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define OK       0
#define ERROR    1
#define MAX_SIZE 100

void CopyOpcodeIntoResult(char *destination, uint8_t opcode)
{
    switch (opcode)
    {
        case 0b100010:
        case 0b1011:
        {
            char opcodeString[] = "mov ";
            strcpy_s(destination, strlen(opcodeString) + 1, opcodeString);
        }
        break;

        default:
            printf("Invalid opcode\n");
    }
}

char *ReadFile(const char *filePath, size_t *size)
{
    // Open the file
    FILE *file = NULL;
    errno_t err = fopen_s(&file, filePath, "rb");
    if (err != 0)
    {
        return NULL;
    }

    // Determine the file size
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // Allocate memory for the file content
    char *buffer = (char *)malloc(fileSize);
    if (buffer == NULL)
    {
        fclose(file);
        return NULL;
    }

    // Read the file into the buffer
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize)
    {
        free(buffer);
        fclose(file);
        return NULL;
    }

    // Close the file
    fclose(file);

    // Store the size of the file
    if (size)
    {
        *size = fileSize;
    }

    // Return the buffer that contains file content
    return buffer;
}

const char sixteenBitRegisters[][3] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char eigthBitRegisters[][3] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
const char memoryMode[][128] =
    {"[bx + si]", "[bx + di]", "[bp + si]", "[bp + di]", "[si]", "[di]", "[DIRECT ADDRESS]", "[bx]"};
const char memoryModeDisplacement[][128] =
    {"[bx + si]", "[bx + di]", "[bp + si]", "[bp + di]", "[si]", "[di]", "[bp]", "[bx]"};

void GetReg(uint8_t w, uint8_t reg, char *output)
{
    if (w == 1)
    {
        strcpy_s(output, strlen(sixteenBitRegisters[reg]) + 1, sixteenBitRegisters[reg]);
    }
    else
    {
        strcpy_s(output, strlen(eigthBitRegisters[reg]) + 1, eigthBitRegisters[reg]);
    }
}

void GetRm(char *input, int32_t *offset, uint8_t mod, uint8_t w, uint8_t rm, char *output)
{
    switch (mod)
    {
        case 0b11:
        {
            if (w == 1)
            {
                strcpy_s(output, strlen(sixteenBitRegisters[rm]) + 1, sixteenBitRegisters[rm]);
            }
            else
            {
                strcpy_s(output, strlen(eigthBitRegisters[rm]) + 1, eigthBitRegisters[rm]);
            }
        }
        break;
        case 0b00:
        {
            strcpy_s(output, strlen(memoryMode[rm]) + 1, memoryMode[rm]);
        }
        break;
        case 0b01:
        {
            strcpy_s(output, strlen(memoryModeDisplacement[rm]) + 1, memoryModeDisplacement[rm]);

            uint8_t low8bitDisplacement = input[(*offset)++];
            if (low8bitDisplacement == 0)
            {
                break;
            }
            else
            {
                output[strlen(output) - 1] = '\0';
                sprintf(output, "%s + %d]", output, low8bitDisplacement);
            }
        }
        break;
        case 0b10:
        {
            strcpy_s(output, strlen(memoryModeDisplacement[rm]) + 1, memoryModeDisplacement[rm]);

            uint8_t low8bitDisplacement = input[(*offset)++];
            int8_t high8bitDisplacement = input[(*offset)++];
            int16_t wideDisplacement = (high8bitDisplacement << 8) | low8bitDisplacement;
            if (wideDisplacement == 0)
            {
                break;
            }
            else
            {
                output[strlen(output) - 1] = '\0';
                sprintf(output, "%s + %d]", output, wideDisplacement);
            }
        }
        break;
    }
}

enum Opcode
{
    mov_ModRegRm,
    mov_ImediateToReg,
    add_ModRegRm,
    add_ImediateToReg,
};

typedef struct
{
    Opcode opcode;
    char name[4];
} Instruction;

int32_t GetInstruction(const uint8_t firstByte, Instruction *instruction)
{
    memset(instruction, 0, sizeof(Instruction));
    switch ((firstByte >> 5) & 0b111)
    {
        case 0b101:
            switch ((firstByte >> 4) & 0b0001)
            {
                case 0b1:
                    instruction->opcode = mov_ImediateToReg;
                    strcpy_s(instruction->name, strlen("mov") + 1, "mov");
                    break;

                default:
                    return -1;
            }
            break;

        case 0b100:
            switch ((firstByte >> 2) & 0b000111)
            {
                case 0b010:
                    instruction->opcode = mov_ModRegRm;
                    strcpy_s(instruction->name, strlen("mov") + 1, "mov");
                    break;

                case 0b000:
                    instruction->opcode = add_ImediateToReg;
                    strcpy_s(instruction->name, strlen("add") + 1, "add");
                    break;

                default:
                    return -1;
            }
            break;

        case 0b000:
            switch ((firstByte >> 2) & 0b000111)
            {
                case 0b000:
                    instruction->opcode = add_ModRegRm;
                    strcpy_s(instruction->name, strlen("add") + 1, "add");
                    break;

                default:
                    return -1;
            }
            break;

        default:
            return -1;
    }

    return 0;
}

int32_t main(int32_t argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return ERROR;
    }

    size_t fileSize;
    char *input = ReadFile(argv[1], &fileSize);
    if (input == NULL)
    {
        printf("Error reading file: %s\n", argv[1]);
        return ERROR;
    }

    FILE *outputFile;
    errno_t err = fopen_s(&outputFile, "output.asm", "w");
    if (err != 0)
    {
        printf("Error opening file for output\n");
        return ERROR;
    }

    fprintf(outputFile, "bits 16\n\n");

    char result[MAX_SIZE];
    int32_t offset = 0;
    while (offset < fileSize)
    {
        uint8_t firstByte = input[offset++];
        if (input == NULL)
        {
            break;
        }

        Instruction instruction;
        if (GetInstruction(firstByte, &instruction) == -1)
        {
            printf("Invalid opcode\n");
            fclose(outputFile);
            return ERROR;
        }

        strcpy_s(result, strlen(instruction.name) + 1, instruction.name);
        strncat_s(result, MAX_SIZE, " ", 1);

        char source[128] = "\0";
        char destination[128] = "\0";
        if (instruction.opcode == mov_ImediateToReg)
        {
            uint8_t w = (firstByte >> 3) & 0b1;
            uint8_t reg = firstByte & 0b111;

            if (w == 1)
            {
                // Make sure that the low byte in unsigned, so that we can OR it with the high byte
                uint8_t lowByte = input[offset++];
                int8_t highByte = input[offset++];
                int16_t wideByte = (highByte << 8) | lowByte;
                strcpy_s(destination, strlen(sixteenBitRegisters[reg]) + 1, sixteenBitRegisters[reg]);
                sprintf(source, "%d", wideByte);
            }
            else
            {
                int8_t lowByte = input[offset++];
                strcpy_s(destination, strlen(eigthBitRegisters[reg]) + 1, eigthBitRegisters[reg]);
                sprintf(source, "%d", lowByte);
            }
        }
        else if (instruction.opcode == add_ImediateToReg)
        {
            uint8_t s = (firstByte >> 1) & 0b1;
            uint8_t w = firstByte & 0b1;

            uint8_t secondByte = input[offset++];
            uint8_t mod = (secondByte >> 6) & 0b11;
            uint8_t rm = secondByte & 0b111;

            if (w == 1)
            {
                if (s == 1)
                {
                    uint8_t lowByte = input[offset++];
                    strcpy_s(destination, strlen(sixteenBitRegisters[rm]) + 1, sixteenBitRegisters[rm]);
                    sprintf(source, "%d", lowByte);
                }
                else
                {
                    uint8_t lowByte = input[offset++];
                    strcpy_s(destination, strlen(sixteenBitRegisters[rm]) + 1, sixteenBitRegisters[rm]);
                    sprintf(source, "%d", lowByte);
                }
            }
            else
            {
                int8_t lowByte = input[offset++];
                strcpy_s(destination, strlen(eigthBitRegisters[rm]) + 1, eigthBitRegisters[rm]);
                sprintf(source, "%d", lowByte);
            }
        }
        else if (instruction.opcode == mov_ModRegRm || instruction.opcode == add_ModRegRm)
        {
            uint8_t w = firstByte & 0b1;

            uint8_t secondByte = input[offset++];
            uint8_t reg = (secondByte >> 3) & 0b111;
            uint8_t d = (firstByte >> 1) & 0b1;
            uint8_t mod = (secondByte >> 6) & 0b11;
            uint8_t rm = secondByte & 0b111;

            if (d == 1)
            {
                // Register is the destination field
                GetReg(w, reg, destination);
                GetRm(input, &offset, mod, w, rm, source);
            }
            else
            {
                // Register is the source field
                GetReg(w, reg, source);
                GetRm(input, &offset, mod, w, rm, destination);
            }
        }

        strncat_s(result, MAX_SIZE, destination, strlen(destination));
        strncat_s(result, MAX_SIZE, ", ", 2);
        strncat_s(result, MAX_SIZE, source, strlen(source));

        fprintf(outputFile, "%s\n", result);
    }

    fclose(outputFile);

    return 0;
}
