//
// Created by 赵先生 on 2021/6/29.
//

#ifndef RISC_V_PROGRAM_HPP
#define RISC_V_PROGRAM_HPP
#include "opReader.hpp"
#include "parser.hpp"
class program{
private:
    reader narrator;
    unsigned int reg[32];
    unsigned int pc;
    struct state{
        unsigned int pc = 0; // the pc of executed instruction
        // about ram operation
        bool ramFlag = false;
        unsigned int ramPos = 0u;//position
        bool ramRead = false;bool ramWrite = false;
        int size = 0;//bytes
        unsigned int data = 0u;
        bool sign = false; // sign/unsigned-extend
        // left-value register assignation
        bool regFlag = false;
        int rd = 0;
        unsigned int value = 0u;
        //pc of cpu modification
        bool pcFlag = false;
        unsigned int newpc = 0;
    };
public:
    program(): narrator(1000000),pc(0){
        narrator.initialize();
    }
private:
    void writeBack(state & codeState){
        if(codeState.regFlag) {
            if (codeState.rd != 0) reg[codeState.rd] = codeState.value;
        }
        if(codeState.pcFlag){
            pc = codeState.newpc;
        }
    }
    unsigned int IF(){
        unsigned int ans = narrator.readIns(pc);
        pc += 4;
        return ans;
    }
    parser ID(unsigned int code){
        return parser(code);
    }
    void execute(unsigned int code,parser & ID_code,state & codeState){
        opClass codeClass = ID_code.getClass();
        if(codeClass == LUI){
            int rd = ID_code.getrd();
            codeState.regFlag = true;
            codeState.rd = rd;
            codeState.value = ID_code.getimm();
            return;
        }
        if(codeClass == AUIPC){
            int rd = ID_code.getrd();
            codeState.regFlag = true;
            codeState.rd = rd;
            codeState.value = codeState.pc + ID_code.getimm();
            return;
        }
        if(codeClass == JAL){
            int rd = ID_code.getrd();
            codeState.regFlag = true;
            codeState.rd = rd;
            codeState.value = codeState.pc + 4;
            codeState.pcFlag = true;
            codeState.newpc = codeState.pc + ID_code.getimm();
            return;
        }
        if(codeClass == JALR){
            int rd = ID_code.getrd();
            codeState.regFlag = true;
            codeState.rd = rd;
            codeState.value = codeState.pc + 4;
            int rs1 = ID_code.getrs1();
            codeState.pcFlag = true;
            codeState.newpc = (reg[rs1]+ID_code.getimm()) & (~1);
            return;
        }
        if(codeClass == BEQ){
            int rs1 = ID_code.getrs1();
            int rs2 = ID_code.getrs2();
            if(reg[rs1] != reg[rs2]) return;
            codeState.pcFlag = true;
            codeState.newpc = codeState.pc + ID_code.getimm();
            return;
        }
        if(codeClass == BNE){
            int rs1 = ID_code.getrs1();
            int rs2 = ID_code.getrs2();
            if(reg[rs1] == reg[rs2]) return;
            codeState.pcFlag = true;
            codeState.newpc = codeState.pc + ID_code.getimm();
            return;
        }
        if(codeClass == BLT){
            int rs1 = ID_code.getrs1();
            int rs2 = ID_code.getrs2();
            if((int)reg[rs1] < (int)reg[rs2]){
                codeState.pcFlag = true;
                codeState.newpc = codeState.pc + ID_code.getimm();
            }
            return;
        }
        if(codeClass == BGE){
            int rs1 = ID_code.getrs1();
            int rs2 = ID_code.getrs2();
            if((int)reg[rs1] >= (int)reg[rs2]){
                codeState.pcFlag = true;
                codeState.newpc = codeState.pc + ID_code.getimm();
            }
            return;
        }
        if(codeClass == BLTU){
            int rs1 = ID_code.getrs1();
            int rs2 = ID_code.getrs2();
            if(reg[rs1] < reg[rs2]){
                codeState.pcFlag = true;
                codeState.newpc = codeState.pc + ID_code.getimm();
            }
            return;
        }
        if(codeClass == BGEU){
            int rs1 = ID_code.getrs1();
            int rs2 = ID_code.getrs2();
            if(reg[rs1] >= reg[rs2]){
                codeState.pcFlag = true;
                codeState.newpc = codeState.pc + ID_code.getimm();
            }
            return;
        }
        if(codeClass == LB){
            int rd = ID_code.getrd();
            int rs1 = ID_code.getrs1();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.ramFlag = true;codeState.ramRead = true;codeState.sign = true;codeState.size = 1;
            codeState.ramPos = reg[rs1] + ID_code.getimm();
            return;
        }
        if(codeClass == LH){
            int rd = ID_code.getrd();
            int rs1 = ID_code.getrs1();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.ramFlag = true;codeState.ramRead = true;codeState.sign = true;codeState.size = 2;
            codeState.ramPos = reg[rs1] + ID_code.getimm();
            return;
        }
        if(codeClass == LW){
            int rd = ID_code.getrd();
            int rs1 = ID_code.getrs1();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.ramFlag = true;codeState.ramRead = true;codeState.sign = true;codeState.size = 4;
            codeState.ramPos = reg[rs1] + ID_code.getimm();
            return;
        }
        if(codeClass == LBU){
            int rd = ID_code.getrd();
            int rs1 = ID_code.getrs1();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.ramFlag = true;codeState.ramRead = true;codeState.sign = false;codeState.size = 1;
            codeState.ramPos = reg[rs1] + ID_code.getimm();
            return;
        }
        if(codeClass == LHU){
            int rd = ID_code.getrd();
            int rs1 = ID_code.getrs1();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.ramFlag = true;codeState.ramRead = true;codeState.sign = false;codeState.size = 2;
            codeState.ramPos = reg[rs1] + ID_code.getimm();
            return;
        }
        if(codeClass == SB){
            int rs1 = ID_code.getrs1(),rs2 = ID_code.getrs2();
            codeState.ramFlag = true;codeState.ramWrite = true;codeState.size = 1;
            codeState.ramPos = reg[rs1] + ID_code.getimm();codeState.data = reg[rs2];
            return;
        }
        if(codeClass == SH){
            int rs1 = ID_code.getrs1(),rs2 = ID_code.getrs2();
            codeState.ramFlag = true;codeState.ramWrite = true;codeState.size = 2;
            codeState.ramPos = reg[rs1] + ID_code.getimm();codeState.data = reg[rs2];
            return;
        }
        if(codeClass == SW){
            int rs1 = ID_code.getrs1(),rs2 = ID_code.getrs2();
            codeState.ramFlag = true;codeState.ramWrite = true;codeState.size = 4;
            codeState.ramPos = reg[rs1] + ID_code.getimm();codeState.data = reg[rs2];
            return;
        }
        if(codeClass == ADDI){
            int rs1 = ID_code.getrs1();
            int rd = ID_code.getrd();
            codeState.regFlag = true;
            codeState.rd = rd;
            codeState.value = reg[rs1] + ID_code.getimm();
            return;
        }
        if(codeClass == SLTI){
            int rs1 = ID_code.getrs1(),rd = ID_code.getrd();
            codeState.regFlag = true;codeState.rd = rd;
            if((int)reg[rs1] < (int)ID_code.getimm()) codeState.value = 1;
            else codeState.value = 0;
            return;
        }
        if(codeClass == SLTIU){
            int rs1 = ID_code.getrs1(),rd = ID_code.getrd();
            codeState.regFlag = true;codeState.rd = rd;
            if(reg[rs1] < ID_code.getimm()) codeState.value = 1;
            else codeState.value = 0;
            return;
        }
        if(codeClass == XORI){
            int rs1 = ID_code.getrs1(),rd = ID_code.getrd();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.value = reg[rs1] ^ ID_code.getimm();
            return;
        }
        if(codeClass == ORI){
            int rs1 = ID_code.getrs1(),rd = ID_code.getrd();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.value = reg[rs1] | ID_code.getimm();
            return;
        }
        if(codeClass == ANDI){
            int rs1 = ID_code.getrs1(),rd = ID_code.getrd();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.value = reg[rs1] & ID_code.getimm();
            return;
        }
        if(codeClass == SLLI){
            int rs1 = ID_code.getrs1(),rd = ID_code.getrd(),shamt = ID_code.getShamt();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.value = reg[rs1] << shamt;
            return;
        }
        if(codeClass == SRLI){
            int rs1 = ID_code.getrs1(),rd = ID_code.getrd(),shamt = ID_code.getShamt();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.value = reg[rs1] >> shamt;
            return;
        }
        if(codeClass == SRAI){
            int rs1 = ID_code.getrs1(),rd = ID_code.getrd(),shamt = ID_code.getShamt();
            codeState.regFlag = true;codeState.rd = rd;
            unsigned int rs131 = reg[rs1] >> 31;
            codeState.value = reg[rs1] >> shamt;
            if(rs131 == 0u) return;
            if(rs131 == 1u){
                unsigned int tmp = 0;
                for(int i = 1;i <= shamt;++i){
                    tmp <<= 1;tmp += 1;
                }
                tmp <<= (32-shamt);
                codeState.value |= tmp;
            }
            return;
        }
        if(codeClass == ADD){
            int rs1 = ID_code.getrs1(),rs2 = ID_code.getrs2(),rd = ID_code.getrd();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.value = reg[rs1] + reg[rs2];
            return;
        }
        if(codeClass == SUB){
            int rs1 = ID_code.getrs1(),rs2 = ID_code.getrs2(),rd = ID_code.getrd();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.value = reg[rs1] - reg[rs2];
            return;
        }
        if(codeClass == SLL){
            int rs1 = ID_code.getrs1(),rs2 = ID_code.getrs2(),rd = ID_code.getrd();
            codeState.regFlag = true;codeState.rd = rd;
            unsigned int shamt = reg[rs2] & (0b11111u);
            codeState.value = reg[rs1] << shamt;
            return;
        }
        if(codeClass == SLT){
            int rs1 = ID_code.getrs1(),rs2 = ID_code.getrs2(),rd = ID_code.getrd();
            codeState.regFlag = true;codeState.rd = rd;
            if((int)reg[rs1] < (int)reg[rs2]) codeState.value = 1;
            else codeState.value = 0;
            return;
        }
        if(codeClass == SLTU){
            int rs1 = ID_code.getrs1(),rs2 = ID_code.getrs2(),rd = ID_code.getrd();
            codeState.regFlag = true;codeState.rd = rd;
            if(reg[rs1] < reg[rs2]) codeState.value = 1;
            else codeState.value = 0;
            return;
        }
        if(codeClass == XOR){
            int rs1 = ID_code.getrs1(),rs2 = ID_code.getrs2(),rd = ID_code.getrd();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.value = reg[rs1] ^ reg[rs2];
            return;
        }
        if(codeClass == SRL){
            int rs1 = ID_code.getrs1(),rs2 = ID_code.getrs2(),rd = ID_code.getrd();
            codeState.regFlag = true;codeState.rd = rd;
            unsigned int shamt = reg[rs2] & (0b11111u);
            codeState.value = reg[rs1] >> shamt;
            return;
        }
        if(codeClass == SRA){
            int rs1 = ID_code.getrs1(),rd = ID_code.getrd(),rs2 = ID_code.getrs2();
            codeState.regFlag = true;codeState.rd = rd;
            unsigned int shamt = reg[rs2] & (0b11111u);
            unsigned int rs131 = reg[rs1] >> 31;
            codeState.value = reg[rs1] >> shamt;
            if(rs131 == 0u) return;
            if(rs131 == 1u){
                unsigned int tmp = 0;
                for(int i = 1;i <= shamt;++i){
                    tmp <<= 1;tmp += 1;
                }
                tmp <<= (32-shamt);
                codeState.value |= tmp;
            }
            return;
        }
        if(codeClass == OR){
            int rs1 = ID_code.getrs1(),rs2 = ID_code.getrs2(),rd = ID_code.getrd();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.value = reg[rs1] | reg[rs2];
            return;
        }
        if(codeClass == AND){
            int rs1 = ID_code.getrs1(),rs2 = ID_code.getrs2(),rd = ID_code.getrd();
            codeState.regFlag = true;codeState.rd = rd;
            codeState.value = reg[rs1] & reg[rs2];
            return;
        }
    }
    void mem(state & codeState){
        if(!codeState.ramFlag) return;
        if(codeState.ramRead){
            codeState.data = codeState.value = narrator.readData(codeState.ramPos,codeState.size,codeState.sign);
        }
        if(codeState.ramWrite){
            narrator.writeData(codeState.ramPos,codeState.size,codeState.data);
        }
    }
public:
    void run(){
        while(true) {
            state codeState;
            codeState.pc = pc;
            unsigned opCode = IF(); // 1
            if(opCode == 0x0ff00513u){
                std::cout << (unsigned int)(reg[10] & (0b11111111u));
                break;
            }
//            if(opCode == 0x8067u){
//                std::cout << "debug";
//            }
            parser ID_code(ID(opCode)); // 2
            execute(opCode, ID_code, codeState);// 3
            mem(codeState); // 4
            writeBack(codeState); // 5
        }
    }
};
#endif //RISC_V_PROGRAM_HPP
