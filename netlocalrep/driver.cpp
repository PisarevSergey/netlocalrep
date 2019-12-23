#include "nlr_common.h"
#include "driver.tmh"

namespace
{
  class tracing_driver : public driver
  {
  public:
    tracing_driver()
    {
      WPP_INIT_TRACING(0, 0);
    }

    ~tracing_driver()
    {
      WPP_CLEANUP(0);
    }
  };

  class fltmgr_filter_driver : public tracing_driver
  {
  public:
    fltmgr_filter_driver(NTSTATUS* stat, PDRIVER_OBJECT win_driver) : filter(nullptr)
    {
      FLT_REGISTRATION freg = { 0 };
      freg.Size = sizeof(freg);
      freg.Version = FLT_REGISTRATION_VERSION;
      freg.ContextRegistration = contexts::context_registration;
      freg.FilterUnloadCallback = unload;

      *stat = FltRegisterFilter(win_driver, &freg, &filter);
    }

    ~fltmgr_filter_driver()
    {
      if (filter)
      {
        FltUnregisterFilter(filter);
        filter = nullptr;
      }
    }

    NTSTATUS allocate_context(FLT_CONTEXT_TYPE ContextType, SIZE_T ContextSize, POOL_TYPE PoolType, PFLT_CONTEXT* ReturnedContext)
    {
      return FltAllocateContext(filter, ContextType, ContextSize, PoolType, ReturnedContext);
    }

    NTSTATUS start_filtering()
    {
      return FltStartFiltering(filter);
    }

    static NTSTATUS unload(FLT_FILTER_UNLOAD_FLAGS)
    {
      info_message(DRIVER, "unloading");

      delete get_driver();

      return STATUS_SUCCESS;
    }

  private:
    PFLT_FILTER filter;
  };

  class top_driver : public fltmgr_filter_driver
  {
  public:
    top_driver(NTSTATUS* stat, PDRIVER_OBJECT win_driver) : fltmgr_filter_driver(stat, win_driver)
    {}

    void* __cdecl operator new(size_t, void* p) { return p; }
  };

  char driver_memory[sizeof(top_driver)];
}


NTSTATUS create_driver(PDRIVER_OBJECT win_driver)
{
  NTSTATUS stat = STATUS_SUCCESS;

  new(driver_memory) top_driver(&stat, win_driver);

  return stat;
}

driver* get_driver()
{
  return reinterpret_cast<driver*>(driver_memory);
}
