#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>
#include <map>

#include "QuadTree.h"

namespace cluster {
    struct Coordinate {
        float x;
        float y;
    };

    struct ClusterNode {
    public:
        int x;
        int y;

        std::wstring barcode;
        Coordinate coor;

        ClusterNode(int x, int y, std::wstring barcode) : barcode(barcode) {
            coor.x = float(x);
            coor.y = float(y);
            this->x = 0;
            this->y = 0;
        };
    };

    class Cluster {
    public:
        Cluster() {

        }


    private:
        std::vector<ClusterNode> _cluster_nodes;
        std::map<std::pair<int, int>, int> _all_edges;
        std::vector<std::pair<std::pair<int, int>, int>> _added_endges;

        void gen_edges();
        void add_edges(int max_len);

        tree::TreeNode* gen_quadtree();
    };
}