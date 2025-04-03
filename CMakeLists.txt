cmake_minimum_required(VERSION 3.27.5)

project(PoLang)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)       # Set the C standard to C99
set(CMAKE_C_STANDARD_REQUIRED ON)  # Ensure the standard is used

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})

add_definitions(${LLVM_DEFINITIONS})


add_executable(PoLang
    source/main.c
    source/common.c
    source/file_io.c
    source/lexer.c
    source/string_view.c
    source/parser.c
    source/memory_arena.c
    source/llvm_converter.c
)

# What the f
target_link_libraries(PoLang LLVMWindowsManifest.lib LLVMXRay.lib LLVMLibDriver.lib LLVMDlltoolDriver.lib LLVMTelemetry.lib LLVMTextAPIBinaryReader.lib LLVMCoverage.lib LLVMLineEditor.lib LLVMNVPTXCodeGen.lib LLVMNVPTXDesc.lib LLVMNVPTXInfo.lib LLVMRISCVTargetMCA.lib LLVMRISCVDisassembler.lib LLVMRISCVAsmParser.lib LLVMRISCVCodeGen.lib LLVMRISCVDesc.lib LLVMRISCVInfo.lib LLVMWebAssemblyDisassembler.lib LLVMWebAssemblyAsmParser.lib LLVMWebAssemblyCodeGen.lib LLVMWebAssemblyUtils.lib LLVMWebAssemblyDesc.lib LLVMWebAssemblyInfo.lib LLVMBPFDisassembler.lib LLVMBPFAsmParser.lib LLVMBPFCodeGen.lib LLVMBPFDesc.lib LLVMBPFInfo.lib LLVMX86TargetMCA.lib LLVMX86Disassembler.lib LLVMX86AsmParser.lib LLVMX86CodeGen.lib LLVMX86Desc.lib LLVMX86Info.lib LLVMARMDisassembler.lib LLVMARMAsmParser.lib LLVMARMCodeGen.lib LLVMARMDesc.lib LLVMARMUtils.lib LLVMARMInfo.lib LLVMAArch64Disassembler.lib LLVMAArch64AsmParser.lib LLVMAArch64CodeGen.lib LLVMAArch64Desc.lib LLVMAArch64Utils.lib LLVMAArch64Info.lib LLVMOrcDebugging.lib LLVMOrcJIT.lib LLVMWindowsDriver.lib LLVMMCJIT.lib LLVMJITLink.lib LLVMInterpreter.lib LLVMExecutionEngine.lib LLVMRuntimeDyld.lib LLVMOrcTargetProcess.lib LLVMOrcShared.lib LLVMDWP.lib LLVMDebugInfoLogicalView.lib LLVMDebugInfoGSYM.lib LLVMOption.lib LLVMObjectYAML.lib LLVMObjCopy.lib LLVMMCA.lib LLVMMCDisassembler.lib LLVMLTO.lib LLVMPasses.lib LLVMHipStdPar.lib LLVMCFGuard.lib LLVMCoroutines.lib LLVMipo.lib LLVMVectorize.lib LLVMSandboxIR.lib LLVMLinker.lib LLVMInstrumentation.lib LLVMFrontendOpenMP.lib LLVMFrontendOffloading.lib LLVMFrontendOpenACC.lib LLVMFrontendHLSL.lib LLVMFrontendDriver.lib LLVMFrontendAtomic.lib LLVMExtensions.lib LLVMDWARFLinkerParallel.lib LLVMDWARFLinkerClassic.lib LLVMDWARFLinker.lib LLVMGlobalISel.lib LLVMMIRParser.lib LLVMAsmPrinter.lib LLVMSelectionDAG.lib LLVMCodeGen.lib LLVMTarget.lib LLVMObjCARCOpts.lib LLVMCodeGenTypes.lib LLVMCGData.lib LLVMIRPrinter.lib LLVMInterfaceStub.lib LLVMFileCheck.lib LLVMFuzzMutate.lib LLVMScalarOpts.lib LLVMInstCombine.lib LLVMAggressiveInstCombine.lib LLVMTransformUtils.lib LLVMBitWriter.lib LLVMAnalysis.lib LLVMProfileData.lib LLVMSymbolize.lib LLVMDebugInfoBTF.lib LLVMDebugInfoPDB.lib LLVMDebugInfoMSF.lib LLVMDebugInfoCodeView.lib LLVMDebugInfoDWARF.lib LLVMObject.lib LLVMTextAPI.lib LLVMMCParser.lib LLVMIRReader.lib LLVMAsmParser.lib LLVMMC.lib LLVMBitReader.lib LLVMFuzzerCLI.lib LLVMCore.lib LLVMRemarks.lib LLVMBitstreamReader.lib LLVMBinaryFormat.lib LLVMTargetParser.lib LLVMTableGen.lib LLVMSupport.lib LLVMDemangle.lib ntdll)
