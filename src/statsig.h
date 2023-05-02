#ifndef STATSIG_H
#define STATSIG_H

namespace statsig {
    void initialize(const std::string &sdk_key);

    void shutdown();

    bool check_gate(const std::string& gate_name, bool default_value = false);
}

#endif //STATSIG_H
