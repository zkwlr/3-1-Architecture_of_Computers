################################################################################

.data

int: .word 4
greet: .asciiz "Hello, Im zkwlr\n"

################################################################################

.text

main:
    lw $t0, int
    la $t1, greet
    # print_int int
    move $a0, $t0
    li $v0, 1
    syscall

    move $a0, $t1
    li $v0, 4
    syscall

    li $v0 10
    syscall

################################################################################

