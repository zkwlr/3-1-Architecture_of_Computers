################################################################################

.data

newline: .asciiz "\n"

################################################################################

.text

main:

  # initialize sum ($t0) to 0
  move $t0, $zero

  # initialize counter ($t1) to 0
  move $t1, $zero

loop:

  # increment counter ($t1) by 1
  addi $t1, $t1, 1

  # add counter ($t1) to sum ($t0)
  add $t0, $t0, $t1

  # if counter ($t1) < 10, goto loop
  blt $t1, 10, loop

  # print_int sum ($t0)
  li $v0, 1
  move $a0, $t0
  syscall

  # print_str newline 
  li $v0, 4
  la $a0, newline
  syscall

  # return
  jr $ra

################################################################################

