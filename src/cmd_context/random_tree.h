/*++
Copyright (c) 2026 Microsoft Corporation

Module Name:

    random_tree.h

Abstract:

    Controller for structured randomness. Can create "branches" of random generators,
    with each branch having overridable seeds. Leaves are infinite random generators.

Author:

    Sekoia (sekoiatree) 2026-01-04.

Revision History:

--*/
#pragma once

#include "params/rand_params.hpp"
#include "util/util.h"
#include "util/vector.h"

class random_tree {
public:
    class branch {
        unsigned           m_id{0};
        unsigned           m_seed{0};
        svector<unsigned>  m_seeds; // pre-generated child seeds derived via internal random_gen
        svector<branch*>   m_children; // sub-branches that have been generated


    public:
        branch() = delete;
        // Construct a branch, a desired seed and desired number of child seeds.
        // If set, the ID is used to look for a seed override in `gparams`.
        branch(unsigned seed, unsigned num_children, unsigned id = 0);

        // The seed that generated this branch. Generally not useful, but provided for completeness.
        [[nodiscard]] unsigned seed() const;
        // The number of child seeds. Generally not useful, but provided for completeness.
        [[nodiscard]] unsigned size() const;

        // Access the i'th child seed.
        unsigned operator[](unsigned i) const;

        // Override the seed at position i.
        void override_seed(unsigned i, unsigned new_seed);

        // Create a random_gen from the i'th seed.
        [[nodiscard]] random_gen mk_random_gen(unsigned i) const;

        // Create a new branch using the i'th seed as the parent seed.
        [[nodiscard]] branch *mk_branch(unsigned i, unsigned num_children);
    };

protected:
    using rand_param_accessor = unsigned (rand_params::*)() const;
    static constexpr std::pair<unsigned, rand_param_accessor> ID_TO_PARAM[] = {
    };
    static branch* root;

public:
    static constexpr unsigned HASH_SALT_IDX = 0;
    static constexpr unsigned ROOT_CHILDREN = 2;
    static bool init_root(unsigned seed);
    static branch& get_root();
};
