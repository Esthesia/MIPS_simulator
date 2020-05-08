;; Swap
; Mainly from ppt, but with minor changes (lw->lh, sw->sh)

lui $t5, 0x0123
ori $t5, $t5, 0x4567
lui $t6, 0x89ab
ori $t6, $t6, 0xcdef
lui $t7, 0xa0b1
ori $t7, $t7, 0xc2d3
lui $t8, 0x9988
ori $t8, $t8, 0x2211

ori $a0, $0, 0x100
ori $a1, $0, 0
ori $a2, $0, 4

j main
swap: sll $t1, $a1, 2   # $t1 = k * 4
      add $t1, $a0, $t1 # $t1 = v+(k*4)
                        #   (address of v[k])
      lw $t0, 0($t1)    # $t0 (temp) = v[k]
      lh $t2, 4($t1)    # $t2 = v[k+1]
      sh $t2, 0($t1)    # v[k] = $t2 (v[k+1])
      sw $t0, 4($t1)    # v[k+1] = $t0 (temp)
      jr $ra            # return to calling routine
main:
sw $t5, 0($a0)
sw $t6, 4($a0)
sw $t7, 8($a0)
sw $t8, 12($a0)
jal swap
addi $a1, $a1, 1
slt $s0, $a1, $a2
bne $s0, $zero, main
end:
beq $0, $0, end
