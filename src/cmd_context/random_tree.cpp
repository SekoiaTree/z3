/*++
Copyright (c) 2026 Microsoft Corporation

Module Name:

    random_tree.cpp

Abstract:

    Implementation for random_tree declarations.

Author:

    Sekoia (sekoiatree) 2026-01-04.

Revision History:

--*/

#include "cmd_context/random_tree.h"

#include "math/realclosure/realclosure.h"
#include "params/rand_params.hpp"

#include <random>

// ----- random_tree::branch -----
random_tree::branch::branch(unsigned seed, unsigned num_children, unsigned id): m_id(id), m_seed(seed) {
    m_seeds.resize(num_children);
    m_children.resize(num_children);

    if (id != 0) {
        rand_param_accessor accessor = nullptr;
        for (const auto &[other_id, other_accessor] : ID_TO_PARAM) {
            if (other_id == id) {
                accessor = other_accessor;
                break;
            }
        }

        if (accessor == nullptr) {
            m_id = 0; // We don't have an accessor; nothing below us will either
        } else {
            const auto new_seed = (rand_params().*accessor)();
            if (new_seed != 0) {
                m_seed = new_seed;
            }
        }
    }

    random_gen rng(m_seed);
    for (unsigned i = 0; i < num_children; ++i) {
        // Use internal random_gen to derive child seeds deterministically from parent seed.
        auto hi = static_cast<unsigned>(rng());
        auto lo = static_cast<unsigned>(rng());
        m_seeds[i] = (hi << 16) ^ lo;
    }

}

unsigned random_tree::branch::seed() const { return m_seed; }

unsigned random_tree::branch::size() const { return m_seeds.size(); }

unsigned random_tree::branch::operator[](unsigned i) const {
    SASSERT(i < size());
    return m_seeds[i];
}

void random_tree::branch::override_seed(unsigned i, unsigned new_seed) {
    SASSERT(i < size());
    m_seeds[i] = new_seed;
}

random_gen random_tree::branch::mk_random_gen(unsigned i) const {
    SASSERT(i < size());
    return {m_seeds[i]};
}

random_tree::branch *random_tree::branch::mk_branch(unsigned i, unsigned num_children) {
    SASSERT(i < size());

    if (m_children[i] == nullptr) {
        unsigned new_id = m_id;
        if (new_id != 0) {
            new_id <<= 4;
            new_id += i & 0xF;
        }
        m_children[i] = alloc(branch, m_seeds[i], num_children, new_id);
    }

    return m_children[i];
}

random_tree::branch* random_tree::root = nullptr;

bool random_tree::init_root(const unsigned seed) {
    if (root == nullptr) {
        root = alloc(branch, seed, ROOT_CHILDREN);
        return true;
    }
    return false;
}

random_tree::branch& random_tree::get_root() {
    if (root == nullptr) {
        // Get the global root seed that's been set
        const auto p = rand_params();
        auto seed = p.root();
        if (seed == 0) {
            // If it's set to 0, we pick a random seed given by the device.
            // random_device's constructor is called once at most, so the cost isn't too large.
            seed = std::random_device{}();
        }

        root = alloc(branch, seed, ROOT_CHILDREN, 1);
    }

    return *root;
}
