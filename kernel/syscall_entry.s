.global syscall_entry
.global syscall_handler

syscall_entry:
  # Put 7th parameter on the stack
  push %rax

  # Call the C-land syscall handleer
  call syscall_handler

  # Take everything off the stack
  add $0x8, %rsp

  # Return from the interrupt handler
  iretq
