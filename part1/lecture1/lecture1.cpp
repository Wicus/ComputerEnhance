#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <windows.h>

typedef struct {
  uint8_t instruction;
  uint8_t d;
  uint8_t w;
  uint8_t mod;
  uint8_t reg;
  uint8_t rm;
} MovInstruction;

std::string decode(uint8_t firstByte, uint8_t secondByte);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <filename>" << std::endl;
    return 1;
  }

  // Open the file for reading
  std::ifstream file(argv[1], std::ios::binary);
  if (!file.is_open()) {
    std::cout << "Error opening file: " << argv[1] << std::endl;
    return 1;
  }

  // Open the file for writing
  std::ofstream outFile("out.asm");
  if (!outFile.is_open()) {
    std::cout << "Error opening file: " << argv[1] << std::endl;
    return 1;
  }

  outFile << "bits 16" << std::endl << std::endl;

  do {
    uint8_t firstByte = file.get();
    uint8_t secondByte = file.get();

    std::string result = decode(firstByte, secondByte);

    outFile << result << std::endl;
  } while (file.peek() != EOF);

  file.close();
  outFile.close();

  return 0;
}

std::vector<std::string> registerFieldsWide = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
std::vector<std::string> registerFieldsLow = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

std::string decode(uint8_t firstByte, uint8_t secondByte) {
  std::string result = "";
  auto opcode = (firstByte & 0b11111100) >> 2;

  MovInstruction movInstruction;

  movInstruction.instruction = (firstByte & 0b11111100) >> 2; // 0b100010 = mov
  movInstruction.d = (firstByte & 0b00000010) >> 1; // 0b1 = reg is destination
  movInstruction.w = (firstByte & 0b00000001); // 0b1 = 16 bit, 0b0 = 8 bit
  movInstruction.mod = (secondByte & 0b11000000) >> 6; // 0b11 = register
  movInstruction.reg = (secondByte & 0b00111000) >> 3; // 3 bits of register (name)
  movInstruction.rm = (secondByte & 0b00000111); // 3 bits of register (name)

  // mov
  if (movInstruction.instruction == 0b100010) {
    result += "mov ";
  }

  // register to register
  if (movInstruction.mod == 0b11) {

    // reg is destination, rm the source
    if (movInstruction.d == 0b1) {
      // 16 bit
      if (movInstruction.w == 0b1) {
        result += registerFieldsWide[movInstruction.reg];
        result += ", ";
        result += registerFieldsWide[movInstruction.rm];
      }
      // 8 bit
      else {
        result += registerFieldsLow[movInstruction.reg];
        result += ", ";
        result += registerFieldsLow[movInstruction.rm];
      }
    }
    // rm is destination, reg is the source
    else {
      // 16 bit
      if (movInstruction.w == 0b1) {
        result += registerFieldsWide[movInstruction.rm];
        result += ", ";
        result += registerFieldsWide[movInstruction.reg];
      }
      // 8 bit
      else {
        result += registerFieldsLow[movInstruction.rm];
        result += ", ";
        result += registerFieldsLow[movInstruction.reg];
      }
    }
  }

  return result;
}
