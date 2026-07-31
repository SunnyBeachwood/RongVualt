#include "vc_random.h"

#include <cerrno>
#include <stdexcept>
#include <sys/random.h>

namespace vc_core {

void FillSecureRandom(std::uint8_t* destination, std::size_t length) {
    if (length != 0 && destination == nullptr) throw std::invalid_argument("Random destination is null");
    std::size_t completed = 0;
    while (completed < length) {
        const ssize_t received = getrandom(destination + completed, length - completed, 0);
        if (received < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error("Android kernel random source failed");
        }
        if (received == 0) throw std::runtime_error("Android kernel random source made no progress");
        completed += static_cast<std::size_t>(received);
    }
}

}  // namespace vc_core
