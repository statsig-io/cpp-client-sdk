#pragma once

// Mutex / Locks
#define WRITE_LOCK(rw_lock_) std::unique_lock<std::shared_mutex> lock(rw_lock_)
#define READ_LOCK(rw_lock_) std::shared_lock<std::shared_mutex> lock(rw_lock_)

// Optionals
#define UNWRAP_WITH_DEFAULT(opt, field, fallback) ((opt).has_value() ? (opt)->field : (fallback))
#define UNWRAP(opt, field) ((opt).has_value() ? (opt)->field : decltype((opt)->field)())
#define UNWRAP_OR(opt, fallback) ((opt).has_value() ? (opt).value() : (fallback))
