
page_prog_csrc   = page_prog.c
page_prog_asrc   =
page_prog_target = page_prog

page_prog:
	make CSRC=$(page_prog_csrc) TARGET=$(page_prog_target) -f boot.make

revboot_csrc   = revboot.c
revboot_asrc   =
revboot_target = revboot

revboot:
	make CSRC=$(revboot_csrc) TARGET=$(revboot_target) -f boot.make
