#include "error_boundary.hpp"

namespace statsig::internal {

Shareable<ErrorBoundary> ErrorBoundary::shareable_ = Shareable<ErrorBoundary>();

}