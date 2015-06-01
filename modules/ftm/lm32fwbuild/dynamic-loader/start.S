	.section	.text
	.align 4
	.global	_start
	.type	_start, @function
_start:
	mv gp, ra  // save old ra
	calli hack // puts hack into ra
hack:
	// restore ra (swap gp, ra):
	// original values ra=<hack>, gp=<ret>
	xor gp, gp, ra	// gp=<hack>^<ret>
	xor ra, ra, gp  // ra=<ret>
	xor gp, gp, ra  // gp=<hack>
	// fix gp to point to got
	sub gp, r0, gp
	addi gp, gp, gotofflo16(hack)
	sub gp, r0, gp
	bi entry
