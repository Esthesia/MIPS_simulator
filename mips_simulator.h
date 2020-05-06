// typedef enum {false, true} bool;

typedef __EX_CONTROL {
  bool regDst;
  bool ALUSrc;
  bool PCSrc;
  bool MemRead;
  bool MemWrite;
  bool regWrite;
  bool MemtoReg;
} EX_CON;

typedef __MEM_CONTROL {
  bool PCSrc;
  bool MemRead;
  bool MemWrite;
  bool regWrite;
  bool MemtoReg;
} MEM_CON;

typedef __WB_CONTROL {
  bool regWrite;
  bool MemtoReg;
} WB_CON;

typedef struct __IF_ID {
  unsigned int IF_pc_num;
  int instruction;
} _IF_ID;

typedef struct __ID_EX {
  EX_CON ex_control;
  unsigned int ID_pc_num;
  int rs_value;
  int rt_value;
  int RT;
  int RD;
  int extension_num;
  bool jump_control;
  unsigned int jump_address; //
} _ID_EX;

typedef struct __EX_MEM {
  MEM_CON mem_control;
  unsigned int EX_pc_num;
  int ALU_result;
  bool zero_flag;
  int num_reg_to_write;
} _EX_MEM;

typedef struct __MEM_WB {
  WB_CON writeback_control;
  int mem_data;
  int ALU_result;
  int num_reg__to_wirte;
} _MEM_WB;

typedef struct __status {
  int cur_cycle;
  int cur_PC;
  unsigned cur_instruction;
  int cur_reg[32];
  // MEMORY I/O need to be implemented
} status;
