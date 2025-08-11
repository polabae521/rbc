// guide_graph_facade.h
#pragma once
#include "guideGraph.h"
#include <vector>

class GuideGraphFacade {
private:
    vom::GuideGraph guide_graph_;
public:
    void setGuideGraph(vom::GuideGraph g);
    bool isHubVertex(int vid);
    std::vector<int> getAllVertexIds();
};
