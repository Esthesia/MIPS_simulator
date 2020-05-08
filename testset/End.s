;; End (endianness)

addi    $s0, $0, 100
addi    $t0, $0, 0
addi    $t1, $0, 7
L1:
sb      $t0, 0($s0)
beq     $t0, $t1, L2
addi    $t0, $t0, 1
addi    $s0, $s0, 1
j       L1
L2:
lw      $t2, 100($0)
lw      $t3, 104($0)
end:
b       end
