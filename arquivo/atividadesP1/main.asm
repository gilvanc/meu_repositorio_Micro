;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            
;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Initialize stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer
			mov.w	#2, R14
			mov.w	#3, R15

;-------------------------------------------------------------------------------
; Main loop here
;-------------------------------------------------------------------------------
MULT_unsigned: tst.w R14 ; b==0 ?
			jnz MULT_unsigned_else ; Se b n�o � zero, v� para o else
			clr.w R15 ; return 0
			ret
MULT_unsigned_else:
			push.w R15 ; Guarde a na pilha
			dec.w R14 ; b--
			call MULT_unsigned ; Calcule a*(b-1)
			pop.w R14 ; Recupere a na pilha
			add.w R14, R15 ; return a + a*(b-1)
			ret
                                            

;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack
            
;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
            
