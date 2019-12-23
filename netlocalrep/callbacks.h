#pragma once

namespace callbacks
{
  FLT_POSTOP_CALLBACK_STATUS post_create(
    _Inout_  PFLT_CALLBACK_DATA       Data,
    _In_     PCFLT_RELATED_OBJECTS    FltObjects,
    _In_opt_ PVOID                    CompletionContext,
    _In_     FLT_POST_OPERATION_FLAGS Flags
  );

  FLT_PREOP_CALLBACK_STATUS pre_fs_control(
    _Inout_ PFLT_CALLBACK_DATA    Data,
    _In_    PCFLT_RELATED_OBJECTS FltObjects,
    _Out_   PVOID* CompletionContext
    );

  static FLT_OPERATION_REGISTRATION operation_registration[] =
  {
    {IRP_MJ_CREATE             , 0, 0             , post_create},
    {IRP_MJ_FILE_SYSTEM_CONTROL, 0, pre_fs_control,           0},
    {IRP_MJ_OPERATION_END}
  };
}
