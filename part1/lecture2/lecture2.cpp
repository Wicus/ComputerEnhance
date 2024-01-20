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

const char *sixteenBitRegisters[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *eigthBitRegisters[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

void GetReg(uint8_t mod, uint8_t w, uint8_t reg, char *output)
{
    switch (mod)
    {
        case 0b11:
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
        break;
    }
}

void GetRm(uint8_t mod, uint8_t w, uint8_t rm, char *output)
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
        int8_t firstByte = fgetc(inputFile);
        if (firstByte == EOF)
        {
            break;
        }

        bool isMovImediateToRegister = (firstByte >> 4) == 0b1011;
        bool isMovRegisterToRegister = (firstByte >> 2) == 0b100010;
        if (!isMovRegisterToRegister && !isMovImediateToRegister)
        {
            // TODO: debug
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
                int8_t secondByte = fgetc(inputFile);
                int8_t thirdByte = fgetc(inputFile);
                strcpy_s(destination, strlen(sixteenBitRegisters[reg]) + 1, sixteenBitRegisters[reg]);
                sprintf(source, "%d", ((uint16_t)secondByte + (uint16_t)thirdByte));
            }
            else
            {
                int8_t secondByte = fgetc(inputFile);
                strcpy_s(destination, strlen(eigthBitRegisters[reg]) + 1, eigthBitRegisters[reg]);
                sprintf(source, "%u", secondByte);
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

            int8_t secondByte = fgetc(inputFile);
            uint8_t reg = (secondByte >> 3) & 0b111;
            uint8_t d = (firstByte >> 1) & 0b1;
            uint8_t mod = (secondByte >> 6) & 0b11;
            uint8_t rm = secondByte & 0b111;

            char source[3];
            char destination[3];

            if (d == 1)
            {
                // Register is the destination field
                GetReg(mod, w, reg, destination);
                GetRm(mod, w, rm, source);
            }
            else
            {
                // Register is the source field
                GetReg(mod, w, reg, source);
                GetRm(mod, w, rm, destination);
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
