.data

str:
  .asciiz "TestString0"
char:
  .asciiz "0"

log_str0:
  .asciiz "INFO: findIndex returned: "
log_newline:
  .asciiz "\n"

.text

################################################################################
# findIndex
#
# INPUTs
#   $a0: the starting memory address of str
#   $a1: the starting memory address of char
#
# OUTPUTs
#   $v0: a 32-bit signed integer indicating the index of the character in string
#
findIndex:
  # stack.push($ra) <-- modify this part depending w.r.t. your implementation
  addi $sp, $sp, -4
  sw $ra, 0($sp)

  # FIXME
  li $v0, -1 # return 초기값은 -1로 설정
  lb $t1, 0($a1) # char 값 t1에 저장
  li $t2, 0 # index를 표현할 변수 t2

loop:
  lb $t0, 0($a0) # 1byte ascii 문자인 str[i]의 값이 저장된 주소에서 값을 읽어 $t0에 저장
  beq $t0, $zero, end
  beq $t0, $t1, found
  addi $a0, $a0, 1 # str의 다음 주소(다음 문자가 저장된 주소)를 a0에 저장
  addi $t2, $t2, 1 # 다음 index 비교 전 index 1 증가
  b loop # loop

found: #return $v0 = index
  move $v0, $t2

end: # return $v0 = -1 , 조건 맞는 index 없을 시 found 거치지 않고 end로 이동

  # $ra = stack.pop() <-- modify this part depending w.r.t. your implementation
  lw $ra, 0($sp)
  addi $sp, $sp, 4

  jr $ra

################################################################################

.globl main
main:

  # stack.push($ra)
  addi $sp, $sp, -4
  sw $ra, 0($sp)

  # $t0 = findIndex(str, char)
  la $a0, str
  la $a1, char
  jal findIndex
  move $t0, $v0

  # print(log_str0, $t0, log_newline)
  li $v0, 4
  la $a0, log_str0
  syscall
  li $v0, 1
  move $a0, $t0
  syscall
  li $v0, 4
  la $a0, log_newline
  syscall

  # $ra = stack.pop()
  lw $ra, 0($sp)
  addi $sp, $sp, 4

  # return
  jr $ra

