;; SU (signed vs unsigned)

lui     $s0, 0xffff
ori     $s0, $s0, 0xffff
ori     $s1, $0, 0x0001
slt     $t0, $s0, $s1
sltu    $t1, $s0, $s1
end:
b       end
