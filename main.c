#include <stdio.h>
#include <stdint.h>
#include <signal.h>
/* unix only */
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

#define MEMORY_MAX (1 << 16)
uint16_t memory[MEMORY_MAX]; /*This is the memory, RAM with 65536 memory locations*/

enum
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,
    R_COND,
    R_COUNT
};

uint16_t reg[R_COUNT];

/* Instruction set*/
enum
{
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

enum
{
    FL_POS = 1 << 0, /* P */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2, /* N */
};

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}


void update_flags(uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) /* a 1 in the left-most bit indicates negative */
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

/* Process */
int main(int argc, const char* argv[]){

    if (argc < 2)
    {
        /* show usage string */
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }

    for (int j = 1; j < argc; ++j)
    {
        if (!read_image(argv[j]))
        {
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }

    /* SETUP*/



    reg[R_COND] = FL_ZRO;

    enum { PC_START = 0x3000 }; /* starting address of program */
    reg[R_PC] = PC_START; /* initialize the program counter */

    int running = 1;
    while (running)
    {
        uint16_t instr = mem_read(reg[R_PC]++); /* fetch the next instruction */
        uint16_t op = instr >> 12; /* extract the opcode at the 4 most significant bits */

        switch(op)
        {
            case OP_ADD:
                uint8_t sr1 = (instr >> 6) & 0x7; /* right shift by 6 bits and mask with 7 */ 
                uint8_t dr = (instr >> 9) & 0x7; /* right shift by 9 bits and mask with 7 */
                bool imm_flag = (instr >> 5) & 0x1; /* right shift by 4 bits and mask with 1*/
                
                if (imm_flag)
                {
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5); /* mask with 0x1F to get imm5 value and sign extend */
                    reg[dr] = reg[sr1] + imm5; /* add imm5 to sr1 and store in dr */
                }
                else
                {
                    uint8_t sr2 = instr & 0x7; /* mask by 0x7 to get 3 least significant bits */
                    reg[dr] = reg[sr1] + reg[sr2];
                }

                update_flags(dr);

                break;
            case OP_AND:
                @{AND}
                break;
            case OP_NOT:
                @{NOT}
                break;
            case OP_BR:
                @{BR}
                break;
            case OP_JMP:
                @{JMP}
                break;
            case OP_JSR:
                @{JSR}
                break;
            case OP_LD:
                @{LD}
                break;
            case OP_LDI:
                uint16_t pc_offset9 = instr & 0x1FF;
                uint16_t memory_address = sign_extend(pc_offset9, 9) + reg[R_PC];
                uint16_t data_address = mem_read(memory_address);
                reg[dr] = mem_read(data_address)
                update_flags(dr)
        
                break;
            case OP_LDR:
                @{LDR}
                break;
            case OP_LEA:
                @{LEA}
                break;
            case OP_ST:
                @{ST}
                break;
            case OP_STI:
                @{STI}
                break;
            case OP_STR:
                @{STR}
                break;
            case OP_TRAP:
                @{TRAP}
                break;
            case OP_RES:
            case OP_RTI:
            default:
                @{BAD OPCODE}
                break;
        }
    }
}