;; MIPS Simulator Checker (adopted dataforwarding)
    .text
    .code

    ;; [T-TYPE]
    ;; ADD
    addi $1, $0, 1 ;; $1 = 1
    addi $2, $0, 2 ;; $2 = 2

    add $3, $2, $1 ;; $3 = 3
    
    ;; SUB
    sub $4, $2, $1 ;; $4 = 1
    sub $5, $1, $2 ;; $5 = -1, signed?

    ;; AND
    and $6, $4, $1 ;; $6 = 1
    and $7, $2, $2 ;; $7 = 2

    ;; OR
    or $8, $2, $1 ;; $8 = 3

    ;; nor (not yet)
    nor $9, $2, $1 ;; $9 = 0xFFFFFFFC

    ;; sll, not in disassembler
    sll $10, $2, 1 ;; $10 = 4

    ;; srl, not in disassembler
    srl $11, $2, 1 ;; $11 = 1

    ;; slt
    slt $12, $5, $1 ;; $12 = $2 < $1 = 1

    ;; sltu
    sltu $13, $5, $1 ;; $13 = 1

    ;; jr (not yet)
    ADD $0, $0, $1 ;; error or nothing to do?


    addi $1, $0, 1
    addi $2, $0, 2
    add $3, $2, $1
    sub $4, $2, $1
    sub $5, $1, $2
    and $6, $4, $1
    and $7, $2, $2
    or $8, $2, $1
    nor $9, $2, $1
    sll $10, $2, 1
    srl $11, $2, 1
    slt $12, $5, $1
    sltu $13, $5, $1
    add $0, $0, $1





