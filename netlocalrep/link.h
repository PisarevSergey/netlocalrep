#pragma once

namespace link
{
  constexpr ULONG local_net_link_device = 0x8765;
  constexpr ULONG FSCTL_QUERY_LOCAL_NAME = CTL_CODE(link::local_net_link_device, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS);

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
