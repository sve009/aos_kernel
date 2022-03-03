.global syscall

syscall:
  # Copy 7th param into %rax
  mov 0x8(%rsp), %rax  

  # Issue system call
  int $0x80

  # Go back to the program
  ret
