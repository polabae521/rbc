// guide_graph_facade.h
#pragma once
#include <vector>
#include <string>
// #include "guideGraph.h" // Not available in this environment

class GuideGraphFacade {
public:
    GuideGraphFacade();
    ~GuideGraphFacade();
    void loadGraph(const std::string& file);
    std::vector<int> getPath(int from, int to) const;
private:
    void* graph_impl_;
};
