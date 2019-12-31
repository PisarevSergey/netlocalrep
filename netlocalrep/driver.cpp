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

  class lanman_passthrough_driver : public tracing_driver
  {
  public:
    lanman_passthrough_driver(NTSTATUS* stat) : value_name(RTL_CONSTANT_STRING(L"{3A89299F-5C45-4CCC-A013-1170B95ADE10}")), value_created(false), lanman_fsctl_key(0)
    {
      UNICODE_STRING lanman_fsctl_path = RTL_CONSTANT_STRING(L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters\\FsctlAllowlist");
      OBJECT_ATTRIBUTES oa;
      InitializeObjectAttributes(&oa, &lanman_fsctl_path, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0);
      *stat = ZwOpenKey(&lanman_fsctl_key, KEY_SET_VALUE, &oa);
      if (NT_SUCCESS(*stat))
      {
        info_message(DRIVER, "ZwOpenKey success");

        *stat = ZwSetValueKey(lanman_fsctl_key, &value_name, 0, REG_DWORD, const_cast<ULONG*>(&link::FSCTL_QUERY_LOCAL_NAME), sizeof(link::FSCTL_QUERY_LOCAL_NAME));
        if (NT_SUCCESS(*stat))
        {
          value_created = true;
          info_message(DRIVER, "ZwSetValueKey success");
        }
        else
        {
          error_message(DRIVER, "ZwSetValueKey failed with status %!STATUS!", *stat);
        }
      }
      else
      {
        error_message(DRIVER, "ZwOpenKey failed with status %!STATUS!", *stat);
        lanman_fsctl_key = 0;
      }
    }

    ~lanman_passthrough_driver()
    {
      if (value_created)
      {
        ASSERT(lanman_fsctl_key);
        ZwDeleteValueKey(lanman_fsctl_key, &value_name);
      }

      if (lanman_fsctl_key)
      {
        ZwClose(lanman_fsctl_key);
      }
    }
  private:
    HANDLE lanman_fsctl_key;
    UNICODE_STRING value_name;
    bool value_created;
  };

  class fltmgr_filter_driver : public lanman_passthrough_driver
  {
  public:
    fltmgr_filter_driver(NTSTATUS* stat, PDRIVER_OBJECT win_driver) : filter(nullptr), cookie(reinterpret_cast<ULONG_PTR>(win_driver)), lanman_passthrough_driver(stat)
    {
      if (NT_SUCCESS(*stat))
      {
        FLT_REGISTRATION freg = { 0 };
        freg.Size = sizeof(freg);
        freg.Version = FLT_REGISTRATION_VERSION;
        freg.ContextRegistration = contexts::context_registration;
        freg.OperationRegistration = callbacks::operation_registration;
        freg.InstanceSetupCallback = attach;
        freg.FilterUnloadCallback = unload;

        *stat = FltRegisterFilter(win_driver, &freg, &filter);
        if (NT_SUCCESS(*stat))
        {
          info_message(DRIVER, "FltRegisterFilter success");
        }
        else
        {
          filter = nullptr;
          error_message(DRIVER, "FltRegisterFilter failed with status %!STATUS!", *stat);
        }

      }
    }

    ~fltmgr_filter_driver()
    {
      if (filter)
      {
        info_message(DRIVER, "unregistering driver");
        FltUnregisterFilter(filter);
        info_message(DRIVER, "driver unregistered");
        filter = nullptr;
      }
      else
      {
        info_message(DRIVER, "filter wasn't registered");
      }
    }

    ULONG_PTR get_cookie() const
    {
      return cookie;
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

    static NTSTATUS attach(
      _In_ PCFLT_RELATED_OBJECTS    FltObjects,
      _In_ FLT_INSTANCE_SETUP_FLAGS /*Flags*/,
      _In_ DEVICE_TYPE              VolumeDeviceType,
      _In_ FLT_FILESYSTEM_TYPE      /*VolumeFilesystemType*/
    )
    {
      info_message(DRIVER, "entering attach");

      NTSTATUS return_status(STATUS_FLT_DO_NOT_ATTACH);

      if ((FILE_DEVICE_DISK_FILE_SYSTEM    == VolumeDeviceType) ||
          (FILE_DEVICE_NETWORK_FILE_SYSTEM == VolumeDeviceType))
      {
        info_message(DRIVER, "this is local disk or network drive");

        contexts::instance* ic;
        NTSTATUS stat = get_driver()->allocate_context(FLT_INSTANCE_CONTEXT, sizeof(*ic), PagedPool, reinterpret_cast<PFLT_CONTEXT*>(&ic));
        if (NT_SUCCESS(stat))
        {
          info_message(DRIVER, "instance context allocated successfully");

          switch (VolumeDeviceType)
          {
          case FILE_DEVICE_NETWORK_FILE_SYSTEM:
            info_message(DRIVER, "this is network drive");
            ic->is_network_drive = true;
            break;
          case FILE_DEVICE_DISK_FILE_SYSTEM:
            info_message(DRIVER, "this is local drive");
            ic->is_network_drive = false;
            break;
          }

          stat = FltSetInstanceContext(FltObjects->Instance, FLT_SET_CONTEXT_KEEP_IF_EXISTS, ic, nullptr);
          if (NT_SUCCESS(stat))
          {
            info_message(DRIVER, "FltSetInstanceContext success");
            return_status = STATUS_SUCCESS;
          }
          else
          {
            error_message(DRIVER, "FltSetInstanceContext failed with status %!STATUS!", stat);
          }

          FltReleaseContext(ic);
        }
        else
        {
          error_message(DRIVER, "failed to allocate instance context with status %!STATUS!", stat);
        }
      }

      info_message(DRIVER, "exiting attach with status %!STATUS!", return_status);

      return return_status;
    }

  private:
    PFLT_FILTER filter;
    ULONG_PTR cookie;
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
