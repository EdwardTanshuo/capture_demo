#include "Cluster.h"

bool dist_ascending_order(std::pair<std::pair<int, int>, int> i, std::pair<std::pair<int, int>, int> j) { 
    return i.second < j.second; 
}

bool is_adjacent(std::pair<std::pair<int, int>, int> e, int max_dist) {
    return e.second <= max_dist;
}

std::vector<int> find_adjacent_node(std::vector<std::pair<std::pair<int, int>, int>> added_node, int node) {
    for (auto another_node : added_node) {
        
    }
    return std::vector<int>();
}


void cluster::Cluster::gen_edges() {
    int i, j;
    int len = this->_cluster_nodes.size();
    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            if (i < j) {
                int dist = DIST(this->_cluster_nodes[i].coor, this->_cluster_nodes[j].coor);
                this->_all_edges[std::make_pair(i, j)] = dist;
            }
        }
    }
}

void cluster::Cluster::add_edges(int max_len) {
    for (
        std::map<std::pair<int, int>, int>::iterator iter = this->_all_edges.begin();
        iter != this->_all_edges.end();
        ++iter
        ) {
        auto edge = std::make_pair(iter->first, iter->second);
        if (is_adjacent(edge, max_len))
            this->_added_edges.push_back(edge);
    }

    std::sort(this->_added_edges.begin(), this->_added_edges.end(), dist_ascending_order);
}

tree::TreeNode* cluster::Cluster::gen_quadtree() {
    return nullptr;
}