#ifndef PATCHMATCH_H
#define PATCHMATCH_H

#include "util.h"

class Patchmatcher
{
public:
    Patchmatcher();

    std::vector<std::pair<int, float>> errors;

    NNF_t patch_match();
};

#endif // PATCHMATCH_H
