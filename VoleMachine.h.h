
#ifndef VOLEMACHINE_H
#define VOLEMACHINE_H

#include <bits/stdc++.h>
using namespace std;

class Register {
public:
    vector<string> reg = vector<string>(16, "00");

    string getCell(size_t index) {
        return reg[index];
    }

    void setCell(size_t index, int value) {
        if (index < reg.size()) {
            stringstream ss;
            ss << hex << uppercase << setw(2) << setfill('0') << (value & 0xFF);
            reg[index] = ss.str();
        }
    }

    void printRegisters() {
        cout << "Registers ===" << endl;
        for (size_t i = 0; i < 4; ++i) {
            for (size_t j = 0; j < 4; ++j) {
                cout << setw(4) << setfill('0') << reg[i * 4 + j] << " ";
            }
            cout << endl;
        }
    }
};

class Memory {
public:
    vector<string> memory = vector<string>(256, "00");

    string getCell(size_t index) {
        return memory[index];
    }

    void setCell(size_t index, int value) {
        if (index < memory.size()) {
            stringstream ss;
            ss << hex << uppercase << setw(2) << setfill('0') << (value & 0xFF);
            memory[index] = ss.str();
        }
    }

    void printMemory() {
        cout << "Memory ===" << endl;
        for (size_t i = 0; i < 16; ++i) {
            for (size_t j = 0; j < 16; ++j) {
                cout << setw(2) << memory[i * 16 + j] << " ";
            }
            cout << endl;
        }
    }
};

class ALU {
public:
    Register reg;
    static int hexToDec(const string& hexValue) {
        return stoi(hexValue, nullptr, 16);
    }

    static void ADD_Two_Complement(int R, int S, int T, Register& reg, ostream& outputStream) {
        int valueS = hexToDec(reg.getCell(S));
        int valueT = hexToDec(reg.getCell(T));
        int result = valueS + valueT;

        if (result > 255) result -= 256;
        if (result < 0) result += 256;

        reg.setCell(R, result);
        outputStream << "ADD_Two_Complement R" << R << " = R" << S << " + R" << T << ": " << result << endl;
    }

    static void ADD_Floating_Point(int R, int S, int T, Register& reg, ostream& outputStream) {
        float valueS = stof(reg.getCell(S));
        float valueT = stof(reg.getCell(T));
        float result = valueS + valueT;

        int intResult = static_cast<int>(result);
        reg.setCell(R, intResult);
        outputStream << "ADD_Floating_Point R" << R << " = R" << S << " + R" << T << ": " << result << endl;
    }
};

class CU {
public:
    Memory mem;
    Register reg;

    void LOAD(int R, size_t address, ostream& outputStream) {
        int value = ALU::hexToDec(mem.getCell(address));
        reg.setCell(R, value);
        outputStream << "LOAD R" << R << " = Memory[0x" << hex << uppercase << address << "] = " << value << endl;
    }

    void LOAD_IMM(int R, int value, ostream& outputStream) {
        reg.setCell(R, value);
        outputStream << "LOAD_IMM R" << R << " = " << value << endl;
    }

    void STORE(int R, size_t address, ostream& outputStream) {
        int value = ALU::hexToDec(reg.getCell(R));
        mem.setCell(address, value);
        outputStream << "STORE Memory[0x" << hex << uppercase << address << "] = R" << R << " = " << value << endl;
    }

    void MOVE(int R1, int R2, ostream& outputStream) {
        int value = ALU::hexToDec(reg.getCell(R2));
        reg.setCell(R1, value);
        outputStream << "MOVE R" << R1 << " = R" << R2 << " = " << value << endl;
    }

    void ADD(int R, int S, int T, ostream& outputStream, bool isFloatingPoint) {
        if (isFloatingPoint) {
            ALU::ADD_Floating_Point(R, S, T, reg, outputStream);
        } else {
            ALU::ADD_Two_Complement(R, S, T, reg, outputStream);
        }
    }

    void JUMP(int R, size_t address, ostream& outputStream, size_t& pc) {
        int value = ALU::hexToDec(reg.getCell(R));
        if (value == 0) {
            pc = address;
            outputStream << "Jumped to address " << address << " based on value in register R" << R << endl;
        } else {
            outputStream << "No jump, value in register R" << R << " is non-zero" << endl;
        }
    }

    static void HALT(ostream& outputStream) {
        outputStream << "Halting execution." << endl;
        exit(0);
    }

    void fetch(int& instruction, size_t& programCounter) {
        int instruction1 = ALU::hexToDec(mem.getCell(programCounter++));
        int instruction2 = ALU::hexToDec(mem.getCell(programCounter++));
        instruction = (instruction1 << 8) | instruction2;
    }

    void decode(int instruction, int& opcode, int& R, int& operand) {
        opcode = (instruction >> 12) & 0xF;
        R = (instruction >> 8) & 0xF;
        operand = instruction & 0xFF;
    }

    void executeInstruction(int instruction, size_t& programCounter, ostream& outputStream) {
        int opcode, R, operand;
        decode(instruction, opcode, R, operand);

        switch (opcode) {
            case 0x1:
                LOAD(R, operand, outputStream);
                break;
            case 0x2:
                LOAD_IMM(R, operand, outputStream);
                break;
            case 0x3:
                STORE(R, operand, outputStream);
                break;
            case 0x4:
                MOVE(R, operand, outputStream);
                break;
            case 0x5:
                ADD(R, (operand >> 4) & 0xF, operand & 0xF, outputStream, false);
                break;
            case 0x6:
                JUMP(R, operand, outputStream, programCounter);
                break;
            case 0xC:
                HALT(outputStream);
                return;
        }

        programCounter += 2;
    }
};
class CPU {
public:
    CU cu;
    size_t programCounter = 0;

    Memory mem;
    Register reg;

    CPU() : cu() {}

    void loadInstructions(const string& filename) {
        ifstream file(filename);
        if (file.is_open()) {
            string instruction;
            while (file >> instruction && programCounter < cu.mem.memory.size()) {
                int value = strtol(instruction.c_str(), nullptr, 16);

                cout << "Loading instruction: " << instruction << " (0x" << hex << uppercase << value << ")" << endl;

                if (programCounter + 1 >= cu.mem.memory.size()) {
                    cout << "Not enough memory to store instruction." << endl;
                    break;
                }

                cu.mem.setCell(programCounter++, value >> 8);
                cu.mem.setCell(programCounter++, value & 0xFF);

                    cu.reg.setCell(programCounter - 2, value >> 8);
                    cu.reg.setCell(programCounter - 2, value & 0xFF);
            }
            file.close();
        } else {
            cout << "Unable to open file: " << filename << endl;
        }
    }


    void executeStepByStep(ostream& outputStream) {
        while (programCounter < cu.mem.memory.size()) {
            int instruction;
            cu.fetch(instruction, programCounter);
            cu.executeInstruction(instruction, programCounter, outputStream);
        }
        cu.reg.printRegisters();
        cu.mem.printMemory();
    }

    void executeAllAtOnce(ostream& outputStream) {
        while (programCounter < cu.mem.memory.size()) {
            int instruction;
            cu.fetch(instruction, programCounter);
            cu.executeInstruction(instruction, programCounter, outputStream);
        }
        cu.reg.printRegisters();
        cu.mem.printMemory();
    }

};



#endif //VOLEMACHINE_H
