#include "nlr_common.h"

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT /*driver*/, PUNICODE_STRING)
{
  return STATUS_UNSUCCESSFUL;
}
