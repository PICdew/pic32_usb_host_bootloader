/*--------------------------------------------------------------------------
 * MPLAB XC Compiler -  PIC32MX250F128D linker script
 * 
 * This software is developed by Microchip Technology Inc. and its
 * subsidiaries ("Microchip").
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are 
 * met:
 * 
 * 1.      Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 * 2.      Redistributions in binary form must reproduce the above 
 *         copyright notice, this list of conditions and the following 
 *         disclaimer in the documentation and/or other materials provided 
 *         with the distribution.
 * 3.      Microchip's name may not be used to endorse or promote products
 *         derived from this software without specific prior written 
 *         permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY MICROCHIP "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL MICROCHIP BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING BUT NOT LIMITED TO
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWSOEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *-------------------------------------------------------------------------*/

/* Default linker script, for normal executables */

OUTPUT_FORMAT("elf32-tradlittlemips")
OUTPUT_ARCH(pic32mx)
ENTRY(_reset)
/*
 * Provide for a minimum stack and heap size
 * - _min_stack_size - represents the minimum space that must be made
 *                     available for the stack.  Can be overridden from
 *                     the command line using the linker's --defsym option.
 * - _min_heap_size  - represents the minimum space that must be made
 *                     available for the heap.  Must be specified on
 *                     the command line using the linker's --defsym option.
 */
EXTERN (_min_stack_size _min_heap_size)
PROVIDE(_min_stack_size = 0x400) ;

/*************************************************************************
 * Processor-specific object file.  Contains SFR definitions.
 *************************************************************************/
INPUT("processor.o")

/*************************************************************************
 * Processor-specific peripheral libraries are optional
 *************************************************************************/
OPTIONAL("libmchp_peripheral.a")
OPTIONAL("libmchp_peripheral_32MX250F128D.a")

/*************************************************************************
 * For interrupt vector handling
 *************************************************************************/
PROVIDE(_vector_spacing = 0x00000001);
_ebase_address = 0x9D01F000;

/*************************************************************************
 * Memory Address Equates
 * _ORIGINAL_RESET_ADDR           -- Hardware Reset Vector
 * _RESET_ADDR                    -- Application Reset Vector
 * _BEV_EXCPT_ADDR                -- Boot exception Vector
 * _DBG_EXCPT_ADDR                -- In-circuit Debugging Exception Vector
 * _DBG_CODE_ADDR                 -- In-circuit Debug Executive address
 * _DBG_CODE_SIZE                 -- In-circuit Debug Executive size
 * _GEN_EXCPT_ADDR                -- General Exception Vector
 *************************************************************************/
_ORIGINAL_RESET_ADDR           = 0xBFC00000;
_RESET_ADDR                    = 0x9D006000;
_BEV_EXCPT_ADDR                = 0x9D006380;
_DBG_EXCPT_ADDR                = 0x9D006480;
_DBG_CODE_ADDR                 = 0x9D006490;
_DBG_CODE_SIZE                 = 0x760;
_GEN_EXCPT_ADDR                = _ebase_address + 0x180;

/*************************************************************************
 * Memory Regions
 *
 * Memory regions without attributes cannot be used for orphaned sections.
 * Only sections specifically assigned to these regions can be allocated
 * into these regions.
 *
 * The Debug exception vector is located at 0x9FC00480.
 * The config_<address> sections are used to locate the config words at
 * their absolute addresses.
 *************************************************************************/
MEMORY
{
  kseg0_boot_mem              : ORIGIN = 0x9D006490, LENGTH = 0x0
  debug_exec_mem              : ORIGIN = 0x9D006490, LENGTH = 0x760
  kseg0_program_mem     (rx)  : ORIGIN = 0x9D007000, LENGTH = 0x1F000 - 0x7000
  exception_mem               : ORIGIN = 0x9D01F000, LENGTH = 0x1000
  
  startup_without_bootloader  : ORIGIN = 0xBFC00000, LENGTH = 0x8
  kseg1_boot_mem              : ORIGIN = 0x9D006000, LENGTH = 0x490
  config3                     : ORIGIN = 0xBFC00BF0, LENGTH = 0x4
  config2                     : ORIGIN = 0xBFC00BF4, LENGTH = 0x4
  config1                     : ORIGIN = 0xBFC00BF8, LENGTH = 0x4
  config0                     : ORIGIN = 0xBFC00BFC, LENGTH = 0x4
  kseg1_data_mem       (w!x)  : ORIGIN = 0xA0000000, LENGTH = 0x8000
  sfrs                        : ORIGIN = 0xBF800000, LENGTH = 0x100000
  configsfrs                  : ORIGIN = 0xBFC00BF0, LENGTH = 0x10
}

/*************************************************************************
 * Configuration-word sections. Map the config-pragma input sections to
 * absolute-address output sections.
 *************************************************************************/
SECTIONS
{
  .startup_without_bootloader _ORIGINAL_RESET_ADDR :
   {
       /* Creates a j _reset instruction at address _ORIGINAL_RESET_ADDR
        * _reset must be within the same 256MB boundary as the original reset
        * vector in order for this to work do to the limitations of the j instruction.
        */
       LONG( ((_reset >> 2) & 0x03FFFFFF) | 0x08000000);
       LONG(0);
   } > startup_without_bootloader

  .config_BFC00BF0 : {
    KEEP(*(.config_BFC00BF0))
  } > config3
  .config_BFC00BF4 : {
    KEEP(*(.config_BFC00BF4))
  } > config2
  .config_BFC00BF8 : {
    KEEP(*(.config_BFC00BF8
