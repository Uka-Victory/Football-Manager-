// Facilities.cpp
#include "Facilities.hpp"
#include "Utils.hpp"

void Facilities::degrade(int years) {
    for (int i = 0; i < years; ++i) {
        int idx = Utils::randInt(0, 3);
        if (subLevels[idx] > 0) subLevels[idx]--;
        else {
            switch (idx) {
                case 0: if (training > 1) training--; break;
                case 1: if (academy > 1) academy--; break;
                case 2: if (stadium > 1) stadium--; break;
                case 3: if (infirmary > 1) infirmary--; break;
            }
        }
    }
}

json Facilities::toJson() const {
    return {{"training", training}, {"academy", academy},
            {"stadium", stadium}, {"infirmary", infirmary}};
}

Facilities Facilities::fromJson(const json& j) {
    Facilities f;
    f.training = j.value("training", 1);
    f.academy  = j.value("academy", 1);
    f.stadium  = j.value("stadium", 1);
    f.infirmary= j.value("infirmary", 1);
    return f;
}