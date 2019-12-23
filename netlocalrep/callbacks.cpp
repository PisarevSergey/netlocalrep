#include "nlr_common.h"
#include "callbacks.tmh"

namespace
{
  NTSTATUS print_file_name(PFLT_CALLBACK_DATA Data)
  {
    PFLT_FILE_NAME_INFORMATION fni;
    NTSTATUS stat = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &fni);
    if (NT_SUCCESS(stat))
    {
      info_message(CALLBACKS, "file name %wZ", &fni->Name);
      FltReleaseFileNameInformation(fni);
    }

    return stat;
  }

  void print_local_name(PFLT_CALLBACK_DATA Data)
  {
    link::send snd;
    snd.cookie = get_driver()->get_cookie();

    link::reply* rpl = static_cast<link::reply*>(ExAllocatePoolWithTag(PagedPool, sizeof(*rpl), 'lpeR'));
    if (rpl)
    {
      RtlZeroMemory(rpl, sizeof(*rpl));

      ULONG returned(0);
      NTSTATUS stat = FltFsControlFile(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, link::FSCTL_QUERY_LOCAL_NAME, &snd, sizeof(snd), rpl, sizeof(*rpl), &returned);
      if (NT_SUCCESS(stat))
      {
        UNICODE_STRING local_name;
        local_name.Length = local_name.MaximumLength = static_cast<USHORT>(rpl->size);
        local_name.Buffer = rpl->name;
        info_message(CALLBACKS, "local name is %wZ", &local_name);
      }

      ExFreePool(rpl);
    }
  }
}

FLT_POSTOP_CALLBACK_STATUS callbacks::post_create(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS /*FltObjects*/, PVOID /*CompletionContext*/, FLT_POST_OPERATION_FLAGS Flags)
{
  if (0 == (Flags & FLTFL_POST_OPERATION_DRAINING))
  {
    if (STATUS_SUCCESS == Data->IoStatus.Status)
    {
      contexts::instance* ic(nullptr);
      NTSTATUS stat = FltGetInstanceContext(Data->Iopb->TargetInstance, reinterpret_cast<PFLT_CONTEXT*>(&ic));
      if (NT_SUCCESS(stat))
      {
        if (ic->is_network_drive)
        {
          stat = print_file_name(Data);
          if (NT_SUCCESS(stat))
          {
            print_local_name(Data);
          }
        }

        FltReleaseContext(ic);
      }

    }

  }

  return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS callbacks::pre_fs_control(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS /*FltObjects*/, PVOID* /*CompletionContext*/)
{
  FLT_PREOP_CALLBACK_STATUS fs_status(FLT_PREOP_SUCCESS_NO_CALLBACK);

  if (link::FSCTL_QUERY_LOCAL_NAME == Data->Iopb->Parameters.FileSystemControl.Buffered.FsControlCode)
  {
    if (KernelMode == Data->RequestorMode)
    {
      link::send* snd(nullptr);
      link::reply* rpl(nullptr);

      if ((Data->Iopb->Parameters.FileSystemControl.Buffered.InputBufferLength  >= sizeof(*snd)) &&
          (Data->Iopb->Parameters.FileSystemControl.Buffered.OutputBufferLength >= sizeof(*rpl)))
      {
        snd = static_cast<link::send*>(Data->Iopb->Parameters.FileSystemControl.Buffered.SystemBuffer);
        if (get_driver()->get_cookie() == snd->cookie)
        {
          contexts::instance* ic(nullptr);
          NTSTATUS stat = FltGetInstanceContext(Data->Iopb->TargetInstance, reinterpret_cast<PFLT_CONTEXT*>(&ic));
          if (NT_SUCCESS(stat))
          {
            if (false == ic->is_network_drive)
            {
              PFLT_FILE_NAME_INFORMATION fni;
              stat = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &fni);
              if (NT_SUCCESS(stat))
              {
                rpl = static_cast<link::reply*>(Data->Iopb->Parameters.FileSystemControl.Buffered.SystemBuffer);
                if (fni->Name.Length <= sizeof(rpl->name))
                {
                  rpl->size = fni->Name.Length;
                  RtlCopyMemory(rpl->name, fni->Name.Buffer, rpl->size);
                  Data->IoStatus.Status = STATUS_SUCCESS;
                  Data->IoStatus.Information = rpl->size;
                }
                else
                {
                  Data->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
                  Data->IoStatus.Information = 0;
                }
                fs_status = FLT_PREOP_COMPLETE;

                FltReleaseFileNameInformation(fni);
              }
            }

            FltReleaseContext(ic);
          }
        }
      }
    }
  }

  return fs_status;
}
