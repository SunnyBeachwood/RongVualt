#pragma once

#include <vector>

#include "fd_random_access.h"
#include "vc_request.h"

namespace vc_core {

/** Applies the VeraCrypt 1.26.29 keyfile pool algorithm to a password. */
SecureBytes ApplyVeraCryptKeyfiles(const SecureBytes& password, const std::vector<FdRandomAccess>& keyfiles);

}  // namespace vc_core
