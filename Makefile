revboot0_csrc   = ''
revboot0_asrc   = ''
revboot0_target = 'revboot0'
src = revlib\rev_serial.c decode_msg.c revboot0.c serial.c stk500.c
src += 'revlib/rev_spi.c'
src += 'revlib/ASA/ASA_spi.c'
src += 'revlib/ASA/ASA_general.c'
revboot0_csrc = '$(src)'
revboot0:
	make CSRC=$(revboot0_csrc) TARGET=$(revboot0_target) -f boot.make

revboot_csrc   = 'revboot.c revprog.c getASAcmd.c'
revboot_asrc   = ''
revboot_target = 'revboot'

revboot:
	make CSRC=$(revboot_csrc) TARGET=$(revboot_target) -f boot.make

page_prog_csrc   = 'test\page_prog.c'
page_prog_asrc   = ''
page_prog_target = 'page_prog'

page_prog:
	make CSRC=$(page_prog_csrc) TARGET=$(page_prog_target) -f boot.make

testcmd_csrc   = test\test_get_data.c getASAcmd.c
testcmd_asrc   =
testcmd_target = testcmd

testcmd:
	gcc $(testcmd_csrc) -o $(testcmd_target)
