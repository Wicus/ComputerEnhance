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
const char memoryMode8BitDisplacement[][128] =
    {"[bx + si]", "[bx + di]", "[bp + si]", "[bp + di]", "[si]", "[di]", "[DIRECT ADDRESS]", "[bx]"};
const char memoryMode16BitDisplacement[][128] =
    {"[bx + si]", "[bx + di]", "[bp + si]", "[bp + di]", "[si]", "[di]", "[bp]", "[bx]"};

int DecodeOpcodeName(uint8_t byte, char *output)
{
    switch (byte >> 4)
    {
        case 0b1011:
            strncpy_s(output, 5, "mov ", 4);
            break;
    }

    switch (byte >> 2)
    {
        case 0b100010:
            strncpy_s(output, 5, "mov ", 4);
            break;
        case 0b0:
            strncpy_s(output, 5, "add ", 4);
            break;
        case 0b100000:
            strncpy_s(output, 5, "add ", 4);
            break;
    }

    switch (byte >> 1)
    {}

    return OK;
}

enum BitsType
{
    bitsType_Literal,
    bitsType_Destination,
    bitsType_Wide,
    bitsType_Mod,
    bitsType_Reg,
    bitsType_RM,
};

int instructions[][3] = {{bitsType_Literal, 6, 0b100010},
                         {bitsType_Destination, 1, 0b1},
                         {bitsType_Wide, 0, 0b1},
                         {bitsType_Mod, 2, 0b11},
                         {bitsType_Reg, 3, 0b111},
                         {bitsType_RM, 0, 0b111}};

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

        int instructionIndex = -1;
        for (int i = 0; i < sizeof(instructions) / sizeof(instructions[0]); i++)
        {
            int bitsLiteral = instructions[i][0];
            int numberOfBitsToShift = 8 - instructions[i][1];
            int bitsValue = instructions[i][2];
            int currentValue = firstByte >> numberOfBitsToShift;
            if (bitsValue == currentValue)
            {
                instructionIndex = i;
                break;
            }
        }

        if (instructionIndex == -1)
        {
            printf("Invalid opcode\n");
            fclose(inputFile);
            fclose(outputFile);
            return ERROR;
        }

        for (int i = 0; i < sizeof(instructions[instructionIndex]); i++)
        {
            int bits = instructions[instructionIndex][i];
            printf("bits: %d\n", bits);
        }

        break;

        if (firstByte >> 4 == 0b1011)
        {
            DecodeOpcodeName(firstByte, result);

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
        else if (firstByte >> 2 == 0b100000)
        {
            DecodeOpcodeName(firstByte, result);
            uint8_t s = firstByte >> 1 & 0b1;
            uint8_t w = firstByte & 0b1;

            uint8_t secondByte = fgetc(inputFile);
            uint8_t mod = (secondByte >> 6) & 0b11;
            uint8_t rm = secondByte & 0b111;

            char source[128] = "\0";
            char destination[128] = "\0";

            if (w == 1)
            {
                strcpy_s(destination, strlen(sixteenBitRegisters[rm]) + 1, sixteenBitRegisters[rm]);
            }
            else
            {
                strcpy_s(destination, strlen(eigthBitRegisters[rm]) + 1, eigthBitRegisters[rm]);
            }
            switch (mod)
            {
                // Register mode, no displacement
                case 0b11:
                {
                    if (w == 1)
                    {
                        if (s == 1)
                        {
                            int8_t lowData = fgetc(inputFile);
                            sprintf(source, "%d", lowData);
                        }
                        else
                        {
                            int8_t lowData = fgetc(inputFile);
                            sprintf(source, "%d", lowData);
                        }
                    }
                    else
                    {
                        int8_t lowData = fgetc(inputFile);
                        sprintf(source, "%d", lowData);
                    }
                }
                break;

                // Memory mode, no displacement
                case 0b00:
                {
                    if (w == 1)
                    {
                        if (s == 1)
                        {
                            int8_t lowData = fgetc(inputFile);
                            sprintf(source, "%d", lowData);
                        }
                        else
                        {
                            int8_t lowData = fgetc(inputFile);
                            sprintf(source, "%d", lowData);
                        }
                    }
                    else
                    {
                        int8_t lowData = fgetc(inputFile);
                        sprintf(source, "%d", lowData);
                    }
                }
                break;
                case 0b01:
                {
                    strcpy_s(source, strlen(memoryMode16BitDisplacement[rm]) + 1, memoryMode16BitDisplacement[rm]);

                    uint8_t lowDisplacement = fgetc(inputFile);
                    if (lowDisplacement == 0)
                    {
                        break;
                    }
                    else
                    {
                        source[strlen(source) - 1] = '\0';
                        sprintf(source, "%s + %d]", source, lowDisplacement);
                    }
                }
                break;
                case 0b10:
                {
                    strcpy_s(source, strlen(memoryMode16BitDisplacement[rm]) + 1, memoryMode16BitDisplacement[rm]);

                    uint8_t lowDisplacement = fgetc(inputFile);
                    int8_t highDisplacement = fgetc(inputFile);
                    int16_t fullDisplacement = (highDisplacement << 8) | lowDisplacement;
                    if (fullDisplacement == 0)
                    {
                        break;
                    }
                    else
                    {
                        source[strlen(source) - 1] = '\0';
                        sprintf(source, "%s + %d]", source, fullDisplacement);
                    }
                }
                break;
            }

            strncat_s(result, MAX_SIZE, destination, strlen(destination));
            strncat_s(result, MAX_SIZE, ", ", 2);
            strncat_s(result, MAX_SIZE, source, strlen(source));
        }
        else if (firstByte >> 2 == 0b100010 || firstByte >> 2 == 0b0)
        {
            DecodeOpcodeName(firstByte, result);
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
                if (w == 1)
                {
                    strcpy_s(destination, strlen(sixteenBitRegisters[reg]) + 1, sixteenBitRegisters[reg]);
                }
                else
                {
                    strcpy_s(destination, strlen(eigthBitRegisters[reg]) + 1, eigthBitRegisters[reg]);
                }
                switch (mod)
                {
                    case 0b11:
                    {
                        if (w == 1)
                        {
                            strcpy_s(source, strlen(sixteenBitRegisters[rm]) + 1, sixteenBitRegisters[rm]);
                        }
                        else
                        {
                            strcpy_s(source, strlen(eigthBitRegisters[rm]) + 1, eigthBitRegisters[rm]);
                        }
                    }
                    break;
                    case 0b00:
                    {
                        strcpy_s(source, strlen(memoryMode8BitDisplacement[rm]) + 1, memoryMode8BitDisplacement[rm]);
                    }
                    break;
                    case 0b01:
                    {
                        strcpy_s(source, strlen(memoryMode16BitDisplacement[rm]) + 1, memoryMode16BitDisplacement[rm]);

                        uint8_t lowDisplacement = fgetc(inputFile);
                        if (lowDisplacement == 0)
                        {
                            break;
                        }
                        else
                        {
                            source[strlen(source) - 1] = '\0';
                            sprintf(source, "%s + %d]", source, lowDisplacement);
                        }
                    }
                    break;
                    case 0b10:
                    {
                        strcpy_s(source, strlen(memoryMode16BitDisplacement[rm]) + 1, memoryMode16BitDisplacement[rm]);

                        uint8_t lowDisplacement = fgetc(inputFile);
                        int8_t highDisplacement = fgetc(inputFile);
                        int16_t fullDisplacement = (highDisplacement << 8) | lowDisplacement;
                        if (fullDisplacement == 0)
                        {
                            break;
                        }
                        else
                        {
                            source[strlen(source) - 1] = '\0';
                            sprintf(source, "%s + %d]", source, fullDisplacement);
                        }
                    }
                    break;
                }
            }
            else if (d == 0)
            {
                // Register is the source field
                if (w == 1)
                {
                    strcpy_s(source, strlen(sixteenBitRegisters[reg]) + 1, sixteenBitRegisters[reg]);
                }
                else
                {
                    strcpy_s(source, strlen(eigthBitRegisters[reg]) + 1, eigthBitRegisters[reg]);
                }
                switch (mod)
                {
                    case 0b11:
                    {
                        if (w == 1)
                        {
                            strcpy_s(destination, strlen(sixteenBitRegisters[rm]) + 1, sixteenBitRegisters[rm]);
                        }
                        else
                        {
                            strcpy_s(destination, strlen(eigthBitRegisters[rm]) + 1, eigthBitRegisters[rm]);
                        }
                    }
                    break;
                    case 0b00:
                    {
                        strcpy_s(destination,
                                 strlen(memoryMode8BitDisplacement[rm]) + 1,
                                 memoryMode8BitDisplacement[rm]);
                    }
                    break;
                    case 0b01:
                    {
                        strcpy_s(destination,
                                 strlen(memoryMode16BitDisplacement[rm]) + 1,
                                 memoryMode16BitDisplacement[rm]);

                        uint8_t lowDisplacement = fgetc(inputFile);
                        if (lowDisplacement == 0)
                        {
                            break;
                        }
                        else
                        {
                            destination[strlen(destination) - 1] = '\0';
                            sprintf(destination, "%s + %d]", destination, lowDisplacement);
                        }
                    }
                    break;
                    case 0b10:
                    {
                        strcpy_s(destination,
                                 strlen(memoryMode16BitDisplacement[rm]) + 1,
                                 memoryMode16BitDisplacement[rm]);

                        uint8_t lowDisplacement = fgetc(inputFile);
                        int8_t highDisplacement = fgetc(inputFile);
                        int16_t fullDisplacement = (highDisplacement << 8) | lowDisplacement;
                        if (fullDisplacement == 0)
                        {
                            break;
                        }
                        else
                        {
                            destination[strlen(destination) - 1] = '\0';
                            sprintf(destination, "%s + %d]", destination, fullDisplacement);
                        }
                    }
                    break;
                }
            }

            strncat_s(result, MAX_SIZE, destination, strlen(destination));
            strncat_s(result, MAX_SIZE, ", ", 2);
            strncat_s(result, MAX_SIZE, source, strlen(source));
        }
        else
        {
            printf("Invalid opcode\n");
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
