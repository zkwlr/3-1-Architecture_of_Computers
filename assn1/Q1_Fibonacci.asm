.data

newline:
  .asciiz "\n"

str0:
  .asciiz "Enter a positive integer: "

str1:
  .asciiz "ERROR: received a negative integer!\n"

str2:
  .asciiz "INFO: fibonacci returned "

.text

################################################################################
# FIXME

# $a0: n (<1024)
fibonacci:
  # n<0 일때
  slt $t0, $a0, 0 
  beq $t0, 1, fibonacci_failure #if(n < 0) return $v0 = 0;

  beq $a0, $zero, fibonacci_default #if(n == 0) return $v0 = 1;
  # add $zero, $zero, $zero
  beq $a0, 1, fibonacci_default #else if(n == 1) return $v0 = 1;
  # add $zero, $zero, $zero
  # 중요!!! b 명령어에선 $ra가 바뀌지 않는다.(function call에서만 바뀜)
  # branch : 분기(메모리의 특정 위치로 점프)

  # fibonacci 자식 함수를 불러오기 전 스택에 공간을 할당하고(-4) $ra를 저장해야 한다.
  # s1 등에 저장하면 안되는 이유는 재귀된 함수가 계속 s1에 주소를 덮어써서 이전 저장해논
  # 주소를 잃기 때문이다.
  # 스택에 저장해야 연속적으로 공간을 -4씩 할당하면서 주소를 차례로 쌓고 다시 빼내면서
  # 불러올 수 있다.
  # $v1(함수 리턴값)도 마찬가지로 스택에 공간 할당해서 저장
  # 이는 재귀: 모든 함수가 caller면서 callee인 피보나치 함수의 특성 때문
  addi $a0, $a0, -1 #n-1
  addi $sp, $sp, -4 #스택에 공간 할당
  sw $ra, 0($sp) #스택에 $ra 저장
  jal fibonacci #fibonacci(n-1)
  lw $ra, 0($sp) #$ra 불러오기
  addi $sp, $sp, 4 #스택 공간 해제
  addi $a0, $a0, 1 #n 돌려놓기
  addi $sp, $sp, -4 #스택에 공간 할당
  sw $v1, 0($sp) # fibonacci(n-1) 값을 스택에 저장

  addi $a0, $a0, -2 #n-2
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  jal fibonacci #fibonacci(n-2)
  lw $ra, 0($sp)
  addi $sp, $sp, 4
  addi $a0, $a0, 2 #n 돌려놓기
  lw $t1, 0($sp) #fibonacci(n-1) 값 불러오기
  addi $sp, $sp, 4 #47번째 줄에서 할당한 스택 공간 해제

  add $v1, $v1, $t1 #fibonacci(n-1) + fibonacci(n-2)

return:
  jr $ra

fibonacci_failure:
  li $v0, 0
  jr $ra

fibonacci_default:
  li $v0, 1
  li $v1, 1
  jr $ra

# FIXME
################################################################################

.globl main
main:

  # print_string str0
  li $v0, 4
  la $a0, str0
  syscall

  # $t0 = read_int
  li $v0, 5
  syscall
  move $t0, $v0

  # $s0 = $ra; fibonacci($t0); $ra = $s0
  move $s0, $ra
  move $a0, $t0
  jal fibonacci
  move $ra, $s0

  # $t0 = $v0; $t1 = $v1
  move $t0, $v0
  move $t1, $v1

  # if ($t0 == 0) { goto main_failure }
  beq $t0, $zero, main_failure

main_success:

  # print_string str2
  li $v0, 4
  la $a0, str2
  syscall

  # print_int $t1
  li $v0, 1
  move $a0, $t1
  syscall

  # print_string newline
  li $v0, 4
  la $a0, newline
  syscall

  # goto main_return
  b main_return

main_failure:

  # print_string str1
  li $v0, 4
  la $a0, str1
  syscall

main_return:

  # return
  jr $ra

