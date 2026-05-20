int main(void) {
  asm("mov r0, #0 \n"
      "loop: \n"
        "add r0, r0, #1 \n"
        "cmp r0, #10 \n"
        "bne loop \n");
  
  return 0;
}
