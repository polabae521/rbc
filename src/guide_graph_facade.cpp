// guide_graph_facade.cpp
#include "guide_graph_facade.h"

void GuideGraphFacade::setGuideGraph(vom::GuideGraph g) {
    guide_graph_ = g;
}

bool GuideGraphFacade::isHubVertex(int vid) {
    auto vert = guide_graph_.getVertex(vid);
    return vert && true/*vert->isHub()*/;
}

std::vector<int> GuideGraphFacade::getAllVertexIds() {
    std::vector<int> vids;
    for (const auto& kv : guide_graph_.getVerticesMap())
        vids.push_back(kv.first);
    return vids;
}
