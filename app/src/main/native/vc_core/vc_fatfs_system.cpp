#include <cstdlib>
#include <ctime>

extern "C" {
#include "ff.h"
}

extern "C" void* ff_memalloc(UINT size) {
    return std::malloc(size);
}

extern "C" void ff_memfree(void* pointer) {
    std::free(pointer);
}

extern "C" DWORD get_fattime(void) {
    const std::time_t now = std::time(nullptr);
    std::tm local_time {};
    if (now == static_cast<std::time_t>(-1) || localtime_r(&now, &local_time) == nullptr) return 0;
    const int year = local_time.tm_year + 1900;
    if (year < 1980 || year > 2107) return 0;
    return static_cast<DWORD>((year - 1980) << 25) |
           static_cast<DWORD>((local_time.tm_mon + 1) << 21) |
           static_cast<DWORD>(local_time.tm_mday << 16) |
           static_cast<DWORD>(local_time.tm_hour << 11) |
           static_cast<DWORD>(local_time.tm_min << 5) |
           static_cast<DWORD>(local_time.tm_sec / 2);
}
