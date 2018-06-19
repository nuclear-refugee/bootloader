# TARGET revboot0
revboot0_target = 'revboot0'
revboot0_csrc =  decode_msg.c revboot0.c serial.c stk500.c
revboot0_csrc += revlib\rev_spi.c
revboot0_csrc += revlib\ASA\ASA_spi.c
revboot0_csrc += revlib\ASA\ASA_general.c
revboot0_csrc += revprog.c asaprog.c
revboot0:
	make CSRC='$(revboot0_csrc)' TARGET='$(revboot0_target)' -f boot.make

# TARGET revboot
revboot_target = revboot
revboot_csrc   = revboot.c revprog.c asaprog.c
revboot:
	make CSRC='$(revboot_csrc)' TARGET='$(revboot_target)' -f boot.make

# TARGET page_prog
page_prog_csrc   = test\page_prog.c
page_prog_target = page_prog
page_prog:
	make CSRC='$(page_prog_csrc)' TARGET='$(page_prog_target)' -f boot.make

# TARGET testcmd
testcmd_csrc   = test\test_get_data.c getASAcmd.c
testcmd_target = testcmd
testcmd:
	gcc $(testcmd_csrc) -o $(testcmd_target)
