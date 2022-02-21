PUBLIC Euler2
.data

.code
Euler2 Proc
push rbp		;save rbp register
mov rbp, rsp	;put rsp into rbp

mov rcx, rax	;put the argument passed to this function into rcx as a counter
mov rax, 1
mov rbx, 0		;rax will hold higher fib num, rbx will hold lower fib num

loop1:
mov rdx, rax	;temporarily store higher of the two fib nums in rdx
add rax, rbx	;add lower fib num to higher fib num, store in rax
jo oflow		;if the addition cuased overflow, jump to 'oflow'
mov rbx, rdx	;put the previously higher fib num (now lower) into rbx
dec rcx			;decrement counter
cmp rcx, 0
jg loop1		;if counter is still greather than zero, continue looping

endLoop1:
pop rbp
ret

oflow:
mov rax, 0		;set rax (return value) to zero
jmp endLoop1		;jump to the end of the loop to return with zero in rax

Euler2 endp
end