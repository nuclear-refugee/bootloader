
# rev bootloader

1. **bootloader with ASAPROG and STK500**  
  Use `make revboot0` to compile and burn revboot0.hex to your ASA_M128.  
  Then M128 can use ASAPROG and work as STK500 programmer in prog mode.  
  It takes 3218 bytes and starts at memory of 0xF000.  

2. **bootloader with only ASAPROG**  
  Use `make revboot` to compile and burn revboot.hex to your ASA_M128.  
  Then M128 can use in prog mode.  
  It takes 964 bytes and starts at memory of 0xF000.  

you can also download the hex in release.
