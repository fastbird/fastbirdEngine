#pragma once

inline int GetNumProcessors()
{
#if defined(_MSC_VER)
    SYSTEM_INFO SI;
    GetSystemInfo(&SI);
    return SI.dwNumberOfProcessors;
#else
    return 1;
#endif
}