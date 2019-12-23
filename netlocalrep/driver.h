#pragma once

class driver
{
public:
  virtual NTSTATUS start_filtering() = 0;
  virtual NTSTATUS allocate_context(FLT_CONTEXT_TYPE ContextType, SIZE_T ContextSize, POOL_TYPE PoolType, PFLT_CONTEXT* ReturnedContext) = 0;
  virtual ~driver() {}
  void __cdecl operator delete(void*) {}
};

NTSTATUS create_driver(PDRIVER_OBJECT win_driver);

driver* get_driver();