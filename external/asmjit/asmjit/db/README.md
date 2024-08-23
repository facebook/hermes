AsmJit Instruction Database
---------------------------

This is a database of instructions that is used by AsmJit to generate its internal database and also assembler implementations. This project started initially as AsmDB, but was merged to AsmJit later to make the maintenance easier. The database was created in a way so that each instruction definition would only need a single line in JSON data file. The data is then processed by architecture specific data readers that make the data canonical and ready for processing.

AsmJit database provides the following ISAs:

  * `isa_x86.json` - provides X86 instruction data (both 32-bit and 64-bit)
  * `isa_aarch32.json` - provides AArch32 instruction data (A32/T16/T32 encoding)
  * `isa_aarch64.json` - provides AArch64 instruction data (A64 encoding)
  * `isa_aarch64_sme.json` - provides AArch64 SME instruction data (work-in-progress)

To Be Documented
----------------

This project will be refactored and documented in the future.

License
-------

AsmJit database is dual licensed under Zlib (AsmJit license) or public domain. The database can be used for any purpose, not just by AsmJit.