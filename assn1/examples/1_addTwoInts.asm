################################################################################

.data

prefix1: .asciiz "value1 ($t0): "
prefix2: .asciiz "value2 ($t1): "
prefix3: .asciiz "sum ($t2): "
newline: .asciiz "\n"

################################################################################

.text

main:

  # put two's comp. three into register 8
  ori $t0, $0, 0x3

  # print_string prefix1
  li $v0, 4
  la $a0, prefix1
  syscall
  # print_int $t0
  li $v0, 1
  move $a0, $t0
  syscall
  # print_string newline
  li $v0, 4
  la $a0, newline
  syscall

  # put two's comp. one into register 9
  ori $t1, $0, 0x1

  # print_string prefix2
  li $v0, 4
  la $a0, prefix2
  syscall
  # print_int $t1
  li $v0, 1
  move $a0, $t1
  syscall
  # print_string newline
  li $v0, 4
  la $a0, newline
  syscall

  # add registers 8 and 9, put result in 10
  addu $t2, $t0, $t1

  # print_string prefix3
  li $v0, 4
  la $a0, prefix3
  syscall
  # print_int $t2
  li $v0, 1
  move $a0, $t2
  syscall
  # print_string newline
  li $v0, 4
  la $a0, newline
  syscall

  # return
  jr $ra

################################################################################

