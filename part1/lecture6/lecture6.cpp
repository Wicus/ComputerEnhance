/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://computerenhance.com for more information

   ======================================================================== */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdio.h>

#include "sim86_shared.h"
#pragma comment(lib, "sim86_shared_debug.lib")

#define OK                0
#define ERROR             1
#define MAX_SIZE          100
#define ArrayCount(array) (sizeof(array) / sizeof(array[0]))

typedef struct RegisterValue
{
    char reg[100];
    int32_t value;
} RegisterValue;

static char *ReadFile(const char *filePath, size_t *size);
static int32_t GetRegisterValue(const char *name, RegisterValue *registerValues, int32_t size);

int32_t main(int32_t argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return ERROR;
    }

    size_t fileSize;
    unsigned char *input = (unsigned char *)ReadFile(argv[1], &fileSize);
    if (input == NULL)
    {
        printf("Error reading file: %s\n", argv[1]);
        return ERROR;
    }

    instruction_table table;
    Sim86_Get8086InstructionTable(&table);
    printf("8086 Instruction Instruction Encoding Count: %u\n", table.EncodingCount);

    FILE *outputFile;
    errno_t err = fopen_s(&outputFile, "output.txt", "w");
    if (err != 0)
    {
        printf("Error opening file for output\n");
        return ERROR;
    }

    fprintf(outputFile, "bits 16\n\n");

    RegisterValue registerValues[20] = {0};
    int32_t offset = 0;
    int32_t index = 0;
    while (offset < fileSize)
    {
        instruction decodedInstruction;
        unsigned char *source = &input[offset];
        Sim86_Decode8086Instruction(fileSize - offset, source, &decodedInstruction);
        if (!decodedInstruction.Op)
        {
            printf("Unrecognized instruction\n");
            break;
        }

        offset += decodedInstruction.Size;

        RegisterValue registerValue = {0};
        strncpy(registerValue.reg, Sim86_RegisterNameFromOperand(&decodedInstruction.Operands[0].Register), 100);
        int32_t currentRegisterValue = GetRegisterValue(registerValue.reg, registerValues, ArrayCount(registerValues));
        registerValue.value = decodedInstruction.Operands[1].Immediate.Value;

        registerValues[index++] = registerValue;

        fprintf(outputFile, "%s ", Sim86_MnemonicFromOperationType(decodedInstruction.Op));
        fprintf(outputFile, "%s, ", registerValue.reg);
        fprintf(outputFile, "%d; ", registerValue.value);
        fprintf(outputFile, "%s: 0x%x -> ", registerValue.reg, currentRegisterValue);
        fprintf(outputFile, "0x%x\n", registerValue.value);
    }

    fprintf(outputFile, "\nFinal Registers\n\n");

    for (int i = 0; i < ArrayCount(registerValues); i++)
    {
        if (registerValues[i].reg[0] == '\0')
        {
            break;
        }

        fprintf(outputFile, "    %s: ", registerValues[i].reg);
        fprintf(outputFile, "0x%04x (%d)\n", registerValues[i].value, registerValues[i].value);
    }

    return 0;
}

static char *ReadFile(const char *filePath, size_t *size)
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

static int32_t GetRegisterValue(const char *name, RegisterValue *registerValues, int32_t size)
{
    for (int i = 0; i < size; i++)
    {
        if (strncmp(name, registerValues[i].reg, 100) == 0)
        {
            return registerValues[i].value;
        }
    }

    return 0;
}
