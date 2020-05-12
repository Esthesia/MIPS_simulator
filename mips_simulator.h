/*
  CONTROL UNIT and INTER-STAGE STRUCTURE simulating register in MIPS
 */
typedef enum {FALSE, TRUE} bool;


typedef struct __BRANCH {
  bool prediction;
  unsigned int taken_address;
} BRANCH_INDI;

typedef struct __EX_CONTROL {
  bool regDst;
  bool ALUSrc;
  bool PCSrc;
  bool MemRead;
  bool MemWrite;
  bool regWrite;
  bool MemtoReg;
  bool ALUop_0;
  bool ALUop_1;
  bool stall; // stall to insert "NOP"
} EX_CON;

typedef struct __MEM_CONTROL {
  bool PCSrc;
  bool MemRead;
  bool MemWrite;
  bool regWrite;
  bool MemtoReg;
} MEM_CON;

typedef struct __WB_CONTROL {
  bool regWrite;
  bool MemtoReg;
} WB_CON;

typedef struct __IF_ID {
  bool ID_flushing;
  unsigned int IF_pc_num;
  int instruction;
} _IF_ID;

typedef struct __ID_EX {
  unsigned int ID_instruction; //DEBUGGING
  EX_CON ex_control;
  unsigned int ID_pc_num;
  int ID_op_code;
  int rs_value;
  int rt_value;
  int RS; // FOR HAZARD DETECTION ID/EX.RegisterRs
  int RT;
  int RD;
  int extension_num;
  bool jump_control;
  unsigned int jump_address;
  unsigned int prev_instruction;
} _ID_EX;

typedef struct __EX_MEM {
  unsigned int EX_instruction; //DEBUGGING
  MEM_CON mem_control;
  unsigned int EX_pc_num;
  int EX_op_code;
  int write_data; // In Save case, write data.
  int ALU_result;
  bool zero_flag;
  bool branch_control;
  int num_reg_to_write; // EX/MEM.RegisterRd in Data forwarding case
  /*
    For Load-Store non-stall data forwarding(check condition)
   */
  int RS;
  int immediate_num; // immediate NUM 16bits - can be BRANCH ADDRESS
} _EX_MEM;

typedef struct __MEM_WB {
  unsigned int MEM_instruction; //DEBUGGING
  WB_CON writeback_control;
  int size_of_IO;
  int mem_data;
  int ALU_result;
  int rd_num; // MEM/WB.RegisterRd in Data forwarding case
  /*
    FOR MEMORY I/O PRINTING
   */
   int R_W;
   int mem_address;
   bool MEM_IO_FLAG;
   int write_val;
} _MEM_WB;

typedef struct __status {
  bool no_operation;
  int cur_cycle;
  int cur_PC;
  unsigned cur_instruction;
  int cur_reg[32];
  // MEMORY I/O need to be implemented
} status;
