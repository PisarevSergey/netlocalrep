#include "nlr_common.h"
#include "main.tmh"

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT /*driver*/, PUNICODE_STRING)
{
  WPP_INIT_TRACING(0, 0);

  info_message(MAIN, "test");

  WPP_CLEANUP(0);

  return STATUS_UNSUCCESSFUL;
}
