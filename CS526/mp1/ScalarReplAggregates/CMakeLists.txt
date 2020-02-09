add_llvm_library( LLVMSROAdaweis2 MODULE BUILDTREE_ONLY
  ScalarReplAggregates-daweis2.cpp

  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
