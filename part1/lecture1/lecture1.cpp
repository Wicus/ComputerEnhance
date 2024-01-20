#include <cstdint>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OK    0
#define ERROR 1

int32_t Decode(uint8_t firstByte, uint8_t secondByte, char *result, size_t resultSize);

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

    int32_t firstByte, secondByte;
    char result[50];

    while (true)
    {
        firstByte = fgetc(inputFile);
        secondByte = fgetc(inputFile);
        if ((secondByte == EOF) || (secondByte == 0))
        {
            break;
        }

        if (Decode(firstByte, secondByte, result, sizeof(result)) != OK)
        {
            fprintf(stderr, "Error decoding opcode\n");
            fclose(inputFile);
            fclose(outputFile);
            return ERROR;
        }

        fprintf(outputFile, "%s\n", result);
    }

    fclose(inputFile);
    fclose(outputFile);

    return 0;
}

void CopyOpcodeIntoResult(char *destination, uint8_t opcode)
{
    switch (opcode)
    {
        case 0b100010:
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

int32_t Decode(uint8_t firstByte, uint8_t secondByte, char *result, size_t maxSize)
{
    uint8_t opcode = firstByte >> 2;
    uint8_t d = (firstByte >> 1) & 0b1;
    uint8_t w = firstByte & 0b1;
    uint8_t mod = (secondByte >> 6) & 0b11;
    uint8_t reg = (secondByte >> 3) & 0b111;
    uint8_t rm = secondByte & 0b111;

    CopyOpcodeIntoResult(result, opcode);

    char source[10];
    char destination[10];

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

    strncat_s(result, maxSize, destination, maxSize - strlen(result) - 1);
    strncat_s(result, maxSize, ", ", maxSize - strlen(result) - 1);
    strncat_s(result, maxSize, source, maxSize - strlen(result) - 1);

    return OK;
}
