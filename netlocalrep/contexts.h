#pragma once

namespace contexts
{
  struct instance
  {
    bool is_network_drive;
  };

  static FLT_CONTEXT_REGISTRATION context_registration[] =
  {
    {FLT_INSTANCE_CONTEXT, 0, 0, sizeof(contexts::instance), 'xtcI'},
    {FLT_CONTEXT_END}
  };
}
