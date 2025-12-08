// Filename: mini_cpu.cpp
// Description: Cpp file for the mini cpu
// Author: Layla Johnson
// Date Modified: 12-7-25
//

#include <iostream>
#include <fstream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <sstream>

using namespace std;

// Program Counter
uint32_t PC = 0;

// Instruction Register
uint32_t IR = 0;

// 32 registers
int32_t regs[32];

// 4 KB byte-addressable memory
uint8_t memory[4096];

// Cycle counter
uint32_t cycle = 0;

bool regWrite = false;
uint32_t destReg = 0;
int32_t aluResult = 0;

bool memRead = false;
bool memWrite = false;
uint32_t memAddress = 0;
int32_t memData = 0;


void init_cpu() 
{
    PC = 0;
    IR = 0;
    cycle = 0;

    for (int i = 0; i < 32; i++) 
    {
        regs[i] = 0;
    }

    for (int i = 0; i < 4096; i++) 
    {
        memory[i] = 0;
    }
}

void load_program(const char* filename) 
{
    ifstream infile(filename);
    if (!infile) 
    {
        cout << "Error opening file: " << filename << endl;
        return;
    }

    uint32_t instruction;
    int addr = 0;

    while (infile >> hex >> instruction)
    {
        if (addr + 3 >= 4096) break;

        memory[addr]     = (instruction >> 24) & 0xFF;
        memory[addr + 1] = (instruction >> 16) & 0xFF;
        memory[addr + 2] = (instruction >> 8) & 0xFF;
        memory[addr + 3] = instruction & 0xFF;

        addr += 4;
    }

    infile.close();

}

uint32_t fetch()
{
    IR = (memory[PC] << 24) |
         (memory[PC + 1] << 16) |
         (memory[PC + 2] << 8) |
          memory[PC + 3];
    
    PC += 4;
    return IR;
        
}

void decode(uint32_t instr)
{
    uint32_t opcode = (instr >> 26) & 0x3F;
    uint32_t rs     = (instr >> 21) & 0x1F;
    uint32_t rt     = (instr >> 16) & 0x1F;
    uint32_t rd     = (instr >> 11) & 0x1F;
    uint32_t shamt  = (instr >> 6)  & 0x1F;
    uint32_t funct  =  instr        & 0x3F;
    int16_t imm =     instr        & 0xFFFF;

    cout << "Decoded instructions:" << endl;
    cout << " opcode = " << opcode << endl;
    cout << " rs = " << rs << endl;
    cout << " rt = " << rt << endl;
    cout << " rd = " << rd << endl;
    cout << " funct = " << funct << endl;
    cout << " imm = " << imm << endl;
}

void execute(uint32_t instr)
{
    regWrite = false;
    memRead = false;
    memWrite = false;

    uint32_t opcode = (instr >> 26) & 0x3F;
    uint32_t rs     = (instr >> 21) & 0x1F;
    uint32_t rt     = (instr >> 16) & 0x1F;
    uint32_t rd     = (instr >> 11) & 0x1F;
    uint32_t funct  =  instr        & 0x3F;
    int16_t imm =     instr        & 0xFFFF;

    cout << "Execute: "<< endl;

    if (opcode == 0)
    {
        if (funct == 0x20)
        {
            aluResult = regs[rs] + regs[rt];
            destReg = rd;
            regWrite = true;

            cout << " ADD: R" << rd << " = R" << rs << " + R" << rt << endl;
        }

        else if (funct == 0x22)
        {
            aluResult = regs[rs] - regs[rt];
            destReg = rd;
            regWrite = true;

            cout << " SUB: R" << rd << " = R" << rs << " - R" << rt << endl;
        }
        else
        {
            cout << " R type funct unknown: " << funct << endl;
        }
    }

    else if (opcode == 8)
    {
        aluResult = regs[rs] + imm;
        destReg = rt;
        regWrite = true;
        cout << " ADDI: R" << rt << " = R" << rs << " + " << imm << endl;
    }

    else if (opcode == 4)
    {
        cout << " BEQ: if R" << rs << " == R" << rt << endl;
        if (regs[rs] == regs[rt])
        {
            PC = PC + ((int32_t) imm << 2);
            cout << " Branch taken. New PC = " << PC << endl;
        }
        else
        {
            cout << " Branch not taken." << endl;
        }
    } 
    
    else if (opcode == 35)
    {
        memAddress = regs[rs] + imm;
        memRead = true;

        destReg = rt;
        regWrite = true;

        cout << " LW: R" << rt << " = MEM[" << memAddress << "]" << endl;
    }

    else if (opcode == 43)
    {
        memAddress = regs[rs] + imm;
        memData = regs[rt];
        memWrite = true;

        regWrite = false;

        cout << " SW: MEM[" << memAddress << "] = R" << rt << endl; 
    }
}

void write_back()
{
    if (regWrite)
    {
        if (destReg != 0)
        {
            regs[destReg] = aluResult;
        }

        regs[0] = 0;
        cout << " Write back: R" << destReg << " = " << aluResult << endl;
    }
}

void memory_access()
{
    if (memRead)
    {
        aluResult = (memory[memAddress] << 24) |
                    (memory[memAddress + 1] << 16) |
                    (memory[memAddress + 2] << 8) |
                     memory[memAddress + 3];
        cout << " Memory Read: " << aluResult << endl;
    }

    if (memWrite)
    {
        memory[memAddress] =     (memData >> 24) & 0xFF;
        memory[memAddress + 1] = (memData >> 16) & 0xFF;
        memory[memAddress + 2] = (memData >> 8)  & 0xFF;
        memory[memAddress + 3] =  memData        & 0xFF;

        cout << " Memory Write: " << memData << endl;
    }
}

void print_registers()
{
    cout << "Registers:" << endl;
    for (int i = 0; i < 32; i++)
    {
        cout << " R" << setw(2) << i << ": " << setw(8) << regs[i];

        if ((i + 1) % 4 == 0)
        cout << endl;
    }
}

int main(int argc, char* argv[])  
{
    init_cpu();

    if (argc < 2)
    {
        cout << "Usage: ./cpu <program.hex>" << endl;
        return 1;
    }

    load_program(argv[1]);

    cout << "Mini CPU Simulator Initialized\n";
    
    while (PC < 4096)
    {
        uint32_t currentPC = PC;
        uint32_t instr = fetch();

        if (instr == 0)
        {
            cout << "Program complete" << endl;
            break;
        }
        
        cycle++;

        cout << "\n==============================" << endl;
        cout << "Cycle " << dec << cycle << endl;
        cout << "PC = " << showbase << hex << currentPC << endl;
        cout << "IR = " << showbase << hex << instr << endl;
        
        decode(instr);
        execute(instr);
        memory_access();
        write_back();
        print_registers();
    }

    cout << "\n==== Final Mini CPU Results ====\n";
    print_registers();

    return 0;
}

