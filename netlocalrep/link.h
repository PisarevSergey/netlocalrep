#pragma once

namespace link
{
  constexpr ULONG FSCTL_QUERY_LOCAL_NAME = CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS);

  struct send
  {
    ULONG_PTR cookie;
  };

#include <pshpack4.h>
  struct reply
  {
    unsigned __int32 size;
    wchar_t name[(PAGE_SIZE - sizeof(size))/2];
  };
#include <poppack.h>
  static_assert(PAGE_SIZE == sizeof(reply), "wrong size");
}
