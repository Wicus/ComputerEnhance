#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
  uint8_t instruction;
  uint8_t d;
  uint8_t w;
  uint8_t mod;
  uint8_t reg;
  uint8_t rm;
} MovInstruction;

void decode(uint8_t firstByte, uint8_t secondByte, char *result, size_t resultSize);

const char *registerFieldsWide[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *registerFieldsLow[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  // Open the file for reading
  FILE *file;
  errno_t err = fopen_s(&file, argv[1], "rb");
  if (file == NULL) {
    printf("Error opening file: %s\n", argv[1]);
    return 1;
  }

  // Open the file for writing
  FILE* outFile;
  fopen_s(&outFile, "out.asm", "w");
  if (outFile == NULL) {
    printf("Error opening file for output\n");
    fclose(file);
    return 1;
  }

  fprintf(outFile, "bits 16\n\n");

  int firstByte, secondByte;
  char result[50];
  while ((firstByte = fgetc(file)) != EOF && (secondByte = fgetc(file)) != EOF) {
    decode(firstByte, secondByte, result, sizeof(result));
    fprintf(outFile, "%s\n", result);
  }

  fclose(file);
  fclose(outFile);

  return 0;
}

void decode(uint8_t firstByte, uint8_t secondByte, char *result, size_t resultSize) {
  MovInstruction movInstruction;

  movInstruction.instruction = (firstByte >> 2) & 0b00111111;
  movInstruction.d = (firstByte >> 1) & 0b00000001;
  movInstruction.w = firstByte & 0b00000001;
  movInstruction.mod = (secondByte >> 6) & 0b00000011;
  movInstruction.reg = (secondByte >> 3) & 0b00000111;
  movInstruction.rm = secondByte & 0b00000111;

  if (movInstruction.instruction != 0b100010) {
    strcpy_s(result, resultSize, "Invalid instruction");
    return;
  }

  strcpy_s(result, resultSize, "mov ");

  const char *source;
  const char *destination;

  if (movInstruction.d) {
    destination = movInstruction.w ? registerFieldsWide[movInstruction.reg] : registerFieldsLow[movInstruction.reg];
    source = movInstruction.w ? registerFieldsWide[movInstruction.rm] : registerFieldsLow[movInstruction.rm];
  } else {
    destination = movInstruction.w ? registerFieldsWide[movInstruction.rm] : registerFieldsLow[movInstruction.rm];
    source = movInstruction.w ? registerFieldsWide[movInstruction.reg] : registerFieldsLow[movInstruction.reg];
  }

  strncat_s(result, resultSize, destination, resultSize - strlen(result) - 1);
  strncat_s(result, resultSize, ", ", resultSize - strlen(result) - 1);
  strncat_s(result, resultSize, source, resultSize - strlen(result) - 1);
}

