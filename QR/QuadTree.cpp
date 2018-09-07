#include "QuadTree.h"
#include <algorithm>
#include <unordered_map>
#include <utility>

#define MAX_INT 0xffff

bool sort_function(const tree::TreeNode* a, const tree::TreeNode* b) {
    if (a->x < b->x) {
        return true;
    }
    else if (a->x == b->x && a->y < b->y) {
        return true;
    }
    else {
        return false;
    }
}

namespace tree {
    std::vector<TreeNode*> TreeNode::sort_nodes(
        std::vector<TreeNode*> nodes,
        std::vector<std::pair<Coordinate, Coordinate>> &edges,
        float max_cos_angle,
        float max_dist_h,
        float max_dist_v,
        int w,
        int h) throw(std::exception) {
        int i, j, size = nodes.size();

        // gen quad tree
        for (i = 0; i < size; i++) {
            for (j = 0; j < size; j++) {
                if (i != j) {
                    bool result = false;
                    nodes[i]->insert_child(
                        nodes[j],
                        result, 
                        max_cos_angle,
                        max_dist_h,
                        max_dist_v
                    );
                    if (result && i < j) {
                        // TODO
                        edges.push_back(std::make_pair(nodes[i]->coor, nodes[j]->coor));
                    }
                }
            }
        }

        // collections
        std::vector<TreeNode*> left_vetex;
        std::vector<TreeNode*> top_vetex;
        std::vector<TreeNode*> bsd_queue;
        std::unordered_map<TreeNode*, int> vetex_counter;
        std::unordered_map<TreeNode*, bool> visited;

        // left edge
        for (i = 0; i < size; i++) {
            if (nodes[i]->children[LEFT] == nullptr) {
                left_vetex.push_back(nodes[i]);
            }
        }
        int left_vetex_size = left_vetex.size();

        // top edge
        for (i = 0; i < size; i++) {
            if (nodes[i]->children[UP] == nullptr) {
                top_vetex.push_back(nodes[i]);
            }
        }
        int top_vetex_size = top_vetex.size();

        // find quard tree root
        std::vector<tree::TreeNode*> roots;
        std::set_intersection(
            top_vetex.begin(), top_vetex.end(), 
            left_vetex.begin(), left_vetex.end(),
            std::back_inserter(roots)
        );
        if (roots.size() == 0) {
            throw std::exception("No top left node detected");
        }
        tree::TreeNode* root = roots[0];

        // bsd 
        bsd_queue.push_back(root);
        while (bsd_queue.size() > 0) {
            std::vector<TreeNode*> next_bsd_queue;
            while (bsd_queue.size() > 0) {
                auto node = bsd_queue.back();
                bsd_queue.pop_back();
                if (node->children[UP] != nullptr && !visited[node->children[UP]]) {
                    node->children[UP]->x = node->x;
                    node->children[UP]->y = node->y - 1;
                    visited[node->children[UP]] = true;
                    next_bsd_queue.push_back(node->children[UP]);
                }
                if (node->children[DOWN] != nullptr && !visited[node->children[DOWN]]) {
                    node->children[DOWN]->x = node->x;
                    node->children[DOWN]->y = node->y + 1;
                    visited[node->children[DOWN]] = true;
                    next_bsd_queue.push_back(node->children[DOWN]);
                }
                if (node->children[LEFT] != nullptr && !visited[node->children[LEFT]]) {
                    node->children[LEFT]->x = node->x - 1;
                    node->children[LEFT]->y = node->y;
                    visited[node->children[LEFT]] = true;
                    next_bsd_queue.push_back(node->children[LEFT]);
                }
                if (node->children[RIGHT] != nullptr && !visited[node->children[RIGHT]]) {
                    node->children[RIGHT]->x = node->x + 1;
                    node->children[RIGHT]->y = node->y;
                    visited[node->children[RIGHT]] = true;
                    next_bsd_queue.push_back(node->children[RIGHT]);
                }
            }
            bsd_queue = next_bsd_queue;
        }

        // normalize the node coordinates
        int min_x = 0xffff;
        int min_y = 0xffff;
        int d_x = 0;
        int d_y = 0;
        for (auto node : nodes) {
            if (node->x < min_x) {
                min_x = node->x;
            }
            if (node->y < min_y) {
                min_y = node->y;
            }
        }
        if (min_x < 0) {
            d_x = -min_x;
        }
        if (min_y < 0) {
            d_y = -min_y;
        }
        for (auto node : nodes) {
            node->x += d_x;
            node->y += d_y;
        }

        // sort and return nodes array
        std::sort(nodes.begin(), nodes.end(), sort_function);
        return nodes;
    }
}