#include "nlr_common.h"
#include "main.tmh"

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT win_driver, PUNICODE_STRING)
{
  NTSTATUS stat = create_driver(win_driver);
  if (NT_SUCCESS(stat))
  {
    info_message(MAIN, "create_driver success");
    stat = get_driver()->start_filtering();
    if (NT_SUCCESS(stat))
    {
      info_message(MAIN, "start_filtering success");
    }
    else
    {
      error_message(MAIN, "start_filtering failed with status %!STATUS!", stat);
    }
  }

  if (!NT_SUCCESS(stat))
  {
    delete get_driver();
  }

  return stat;
}
