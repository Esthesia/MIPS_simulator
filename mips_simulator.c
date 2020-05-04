#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"

#define signExtension(x) ((int32_t)(int16_t)x)
#define RTYPE 0

int reg[32];

typedef struct __status {
  int cur_cycle;
  int cur_PC;
  unsigned cur_instruction;
  int cur_reg[32];
  // MEMORY I/O need to be implemented
} status;

void WB(void);
void MEM(void);
void EX(void);
void ID(void);
void IF(status);
void print_status(status);

char *register_print(int reg_num, int reg_val){
  char print_form[8];
  int c, k;
  for(c = 7; c >=0; c--){
      k = reg_val >> c;
      if(k & 1)
          print_form[7-c] = '1';
      else
          print_form[7-c] = '0';
  }
  printf("[%d] %s\n", reg_num, reg_val);
}


void code_insertion(FILE *fp, int code[]){
    char instruction[10];
    char *pos;
    int i = 0;
    while(fgets(instruction, 10, fp) != NULL){
        unsigned int value = strtol(instruction, pos, 16);
        code[i] = value;
        i++;
    }
}

int code_execution(int code[], int mode, int program_counter){
    if(mode == 0){
      status cur_status;
      WB();
      MEM();
      EX();
      ID();
      IF(cur_status);
      int reg_iter;
      for(reg_iter = 0; reg_iter < 32; reg_iter++){
        cur_status.reg[reg_iter] = reg[reg_iter];
      }
      print_status(cur_status);
      return program_counter;
    }
    else if(mode == 1){
      status cur_status;
      WB();
      MEM();
      EX();
      ID();
      IF(,program_counter);
      print_status(cur_status);
      return program_counter;
    }
}


int main(int argc, char *argv[]){
  /*
    File reading for running instruction
   */
  unsigned int file_size;
  unsigned int number_of_instruction;
  int cycle_num;
  int mode;
  FILE *instruction_file;
  instruction_file = fopen(argv[1], "r");
  cycle_num = argv[2];
  mode = argv[3];
  fseek(instruction_file, 0, SEEK_END);
  file_size = ftell(instruction_file);
  number_of_instruction = file_size / 9;
  fseek(instruction_file, 0, SEEK_SET);
  /*
    file insertion to execute simualtor and file close
   */
  unsigned int *instruction = (unsigned int*)malloc(sizeof(unsigned int)*number_of_instruction);
  code_insertion(instruction_file, instruction);
  fclose(instruction_file);
  int register_iter;
  for(register_iter = 0; register_iter < 32; register_iter ++){
     reg[register_iter] = 0;
  }
  /*
    just for debugging
   */
  int i;
  for(i = 0; i < file_size; i++){
    printf("Cycle %d\n", i+1);
    printf("%08x\n\n", instruction[i]);
  }
  /*
  code execution
   */
  int j;
  int pc_init = 0;
  for(j = 0; j < cycle_num; j++){
    pc_init = code_execution(instruction, mode, pc_init);
  }
  /*
    free dynamic allocating memory.
   */
  free(instruction);
  return 0;
}

void print_status(status){
  printf("Cycle &d\n",status.cur_cycle);
  printf("PC: %04x\n",status.cur_PC);
  printf("Instruction: %08x\n\n",status.cur_instruction);
  printf("Registers:\n")
  int register_iter;
  for(register_iter = 0; register_iter < 32; register_iter ++){
    register_print(register_iter, status.cur_reg[register_iter]);
  }
  printf("Memory I/O: "); // not yet implemented
}

void IF(status){

}
