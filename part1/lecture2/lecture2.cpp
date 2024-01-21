#include <cstdint>
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

void GetRm(FILE *inputFile, uint8_t mod, uint8_t w, uint8_t rm, char *output)
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

            uint8_t low8bitDisplacement = fgetc(inputFile);
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

            uint8_t low8bitDisplacement = fgetc(inputFile);
            int8_t high8bitDisplacement = fgetc(inputFile);
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

int32_t main(int32_t argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return ERROR;
    }

    FILE *inputFile;
    errno_t err = fopen_s(&inputFile, argv[1], "rb");
    if (err != 0)
    {
        printf("Error opening file: %s\n", argv[1]);
        return ERROR;
    }

    FILE *outputFile;
    err = fopen_s(&outputFile, "output.asm", "w");
    if (err != 0)
    {
        printf("Error opening file for output\n");
        fclose(inputFile);
        return ERROR;
    }

    fprintf(outputFile, "bits 16\n\n");

    char result[MAX_SIZE];

    while (true)
    {
        uint8_t firstByte = fgetc(inputFile);
        if (feof(inputFile))
        {
            break;
        }

        bool isMovImediateToRegister = (firstByte >> 4) == 0b1011;
        bool isMovRegisterToRegister = (firstByte >> 2) == 0b100010;
        if (!isMovRegisterToRegister && !isMovImediateToRegister)
        {
            printf("Invalid opcode\n");
            fclose(inputFile);
            fclose(outputFile);
            return ERROR;
        }

        if (isMovImediateToRegister)
        {
            char opcodeString[] = "mov ";
            strcpy_s(result, strlen(opcodeString) + 1, opcodeString);

            uint8_t w = (firstByte >> 3) & 0b1;
            uint8_t reg = firstByte & 0b111;

            char source[10];
            char destination[10];
            if (w == 1)
            {
                // Make sure that the low byte in unsigned, so that we can OR it with the high byte
                uint8_t lowByte = fgetc(inputFile);
                int8_t highByte = fgetc(inputFile);
                int16_t wideByte = (highByte << 8) | lowByte;
                strcpy_s(destination, strlen(sixteenBitRegisters[reg]) + 1, sixteenBitRegisters[reg]);
                sprintf(source, "%d", wideByte);
            }
            else
            {
                int8_t lowByte = fgetc(inputFile);
                strcpy_s(destination, strlen(eigthBitRegisters[reg]) + 1, eigthBitRegisters[reg]);
                sprintf(source, "%d", lowByte);
            }

            strncat_s(result, MAX_SIZE, destination, strlen(destination));
            strncat_s(result, MAX_SIZE, ", ", 2);
            strncat_s(result, MAX_SIZE, source, strlen(source));
        }

        if (isMovRegisterToRegister)
        {
            char opcodeString[] = "mov ";
            strcpy_s(result, strlen(opcodeString) + 1, opcodeString);

            uint8_t w = firstByte & 0b1;

            uint8_t secondByte = fgetc(inputFile);
            uint8_t reg = (secondByte >> 3) & 0b111;
            uint8_t d = (firstByte >> 1) & 0b1;
            uint8_t mod = (secondByte >> 6) & 0b11;
            uint8_t rm = secondByte & 0b111;

            char source[128] = "\0";
            char destination[128] = "\0";

            if (d == 1)
            {
                // Register is the destination field
                GetReg(w, reg, destination);
                GetRm(inputFile, mod, w, rm, source);
            }
            else
            {
                // Register is the source field
                GetReg(w, reg, source);
                GetRm(inputFile, mod, w, rm, destination);
            }

            strncat_s(result, MAX_SIZE, destination, strlen(destination));
            strncat_s(result, MAX_SIZE, ", ", 2);
            strncat_s(result, MAX_SIZE, source, strlen(source));
        }

        fprintf(outputFile, "%s\n", result);
    }

    fclose(inputFile);
    fclose(outputFile);

    return 0;
}
