#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_LEN 6

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, char *file)
{
  FILE *fp;
  fp = fopen(file, "r");

  if (fp == NULL)
  {
    fprintf(stderr, "ls8: error opening:  %s\n", file);
    exit(2);
  }

  char line[1024];
  int address = 0;

  while (fgets(line, 1024, fp) != NULL)
  {
    char *endptr;

    // converts str to number
    unsigned char val = strtoul(line, &endptr, 2);

    // prevents unnecessary lines being stored on ram
    if (endptr == line)
    {
      continue;
    }

    cpu->ram[address++] = val;
  }

  fclose(fp);
}

unsigned char cpu_ram_read(struct cpu *cpu, unsigned int pc)
{
  return cpu->ram[pc];
}

void cpu_ram_write(struct cpu *cpu, unsigned int instruction, unsigned int pc)
{
  cpu->ram[pc] = instruction;
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  switch (op)
  {
  case ALU_MUL:
    cpu->reg[regA] = cpu->reg[regA] * cpu->reg[regB];
    break;

  // TODO: implement more ALU ops
  case ALU_ADD:
    cpu->reg[regA] = cpu->reg[regA] + cpu->reg[regB];
    break;
  }
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; // True until we get a HLT instruction

  while (running)
  {
    // TODO
    // 1. Get the value of the current instruction (in address PC).
    int IR = cpu_ram_read(cpu, cpu->pc); // instruction register

    // 2. Figure out how many operands this next instruction requires
    int numOps = IR >> 6;

    // 3. Get the appropriate value(s) of the operands following this instruction
    int ops[4];
    for (int i = 0; i < numOps; i++)
    {
      ops[i] = cpu_ram_read(cpu, cpu->pc + 1 + i);
    }

    // 4. switch() over it to decide on a course of action.
    // 5. Do whatever the instruction should do according to the spec.
    switch (IR)
    {
    case LDI:
      cpu->reg[ops[0]] = ops[1];
      break;

    case PRN:
      printf("%d\n", cpu->reg[ops[0]]);
      break;

    case HLT:
      running = 0;
      break;

    case MUL:
      alu(cpu, ALU_MUL, ops[0], ops[1]);
      break;

    case ADD:
      alu(cpu, ALU_ADD, ops[0], ops[1]);
      break;

    case PUSH:
      // stack pointer = reg[7]
      if (cpu->reg[7] == 0)
      {
        cpu->reg[7] = 0xF4;
      }

      cpu->reg[7]--;
      cpu->ram[cpu->reg[7]] = cpu->reg[ops[0]];
      break;

    case POP:
      if (cpu->reg[7] == 0 || cpu->reg[7] == 0xF4)
      {
        fprintf(stderr, "error: no items in stack!\n");
        break;
      }

      cpu->reg[ops[0]] = cpu->ram[cpu->reg[7]];
      cpu->reg[7]++;
      break;

    case CALL:
      if (cpu->reg[7] == 0)
      {
        cpu->reg[7] = 0xF4;
      }

      cpu->reg[7]--;
      cpu->ram[cpu->reg[7]] = cpu->pc + 2;

      cpu->pc = cpu->reg[ops[0]] - 2;
      break;

    case RET:
      cpu->pc = cpu->ram[cpu->reg[7]];
      cpu->reg[7]++;
      cpu->pc -= 1 + numOps;
      break;

    case CMP:
      if (cpu->reg[ops[0]] == cpu->reg[ops[1]])
      {
        cpu->fl = 0b00000001;
      }
      // if op2 is greater set flag to 0b00000100
      else if (cpu->reg[ops[0]] < cpu->reg[ops[1]])
      {
        cpu->fl = 0b00000100;
      }
      // if op1 is greater set flag to 0b00000010
      else if (cpu->reg[ops[0]] > cpu->reg[ops[1]])
      {
        cpu->fl = 0b00000010;
      }
      break;

    case JMP:
      // Jump to the address stored in the given register
      // Set the PC to the address stored in the given register.
      cpu->pc = cpu->reg[ops[1]] - ops[0] - 1;
      break;

    case JEQ:
      // If `equal` flag is set (true),
      //jump to the address stored in the given register.
      if (cpu->fl == 00000001)
      {
        cpu->pc = cpu->reg[ops[1]] - ops[0] - 1;
      }
      break;

    case JNE:
      //If `E` flag is clear (false, 0),
      //jump to the address stored in the given
      //register.
      if ((cpu->fl & 0b00000001) == 0)
      {
        cpu->pc = cpu->reg[ops[1]] - ops[0] - 1;
      }
      break;

    default:
      printf("Unknown instruction at %d: %d\n", cpu->pc, IR);
      exit(1);
    }

    // 6. Move the PC to the next instruction.
    cpu->pc += 1 + numOps;
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  // TODO: Initialize the PC and other special registers
  cpu->pc = 0;
  memset(cpu->reg, 0, 8 * (sizeof(char)));
  memset(cpu->ram, 0, 256 * sizeof(char));
}
