#include "quad_tree.h"
#include <unordered_map>
#include <utility>

namespace tree {	
	std::vector<TreeNode*> TreeNode::sort_nodes(const std::vector<TreeNode*> nodes, 
												float max_cos_angle, 
												float max_dist, 
												int w, 
												int h) {
		// gen quad tree
		int i, j, size = nodes.size();
		for (i = 0; i < size; i ++) {
			for (j = 0; j < size; j ++) {
				if (i != j) {
					bool result = false;
					nodes[i]->insert_child(nodes[j], result, max_cos_angle, max_dist);
				}
			}
		}
		
		std::vector<TreeNode*> left_vetex = std::vector<TreeNode*>();
		std::vector<TreeNode*> top_vetex = std::vector<TreeNode*>();
		std::unordered_map<TreeNode*, int> m_h;
		std::unordered_map<TreeNode*, int> m_v;
		
		// left edge
		for (i = 0; i < size; i ++) {
			if (nodes[i]->children[LEFT] == nullptr) {
				left_vetex.push_back(nodes[i]);
			}
		}
		int left_vetex_size = left_vetex.size();
		
		// top edge
		for (i = 0; i < size; i ++) {
			if (nodes[i]->children[UP] == nullptr) {
				top_vetex.push_back(nodes[i]);
			}
		}
		int top_vetex_size = top_vetex.size();
		
		// horizontal max lenth
		int temp_h_len[left_vetex_size];
		for (i = 0; i < left_vetex_size; i ++) {
			TreeNode* iter;
			int temp_len = 0;
			for (iter = left_vetex[0]; iter != nullptr; iter = iter->children[RIGHT]) {
				temp_len ++;
			}
			temp_h_len[i] = temp_len;
		}
		int max_h_len = *std::max_element(temp_h_len, temp_h_len + left_vetex_size);
		
		// calculate all the h_index of left edge nodes
		for (i = 0; i < left_vetex_size; i ++) {
			TreeNode* iter;
			int index = 0;
			for (iter = left_vetex[i]; iter != nullptr; iter = iter->children[RIGHT]) {
				if (index == 0) {
					m_h[iter] = max_h_len - temp_h_len[i];
				}
				if (iter->children[RIGHT] != nullptr) {
					m_h[iter->children[RIGHT]] = m_h[iter] + 1;
				}
				index ++;
			}
		}
		
		// vertical max lenth
		int temp_v_len[top_vetex_size];
		for (i = 0; i < top_vetex_size; i ++) {
			TreeNode* iter;
			int temp_len = 0;
			for (iter = top_vetex[0]; iter != nullptr; iter = iter->children[DOWN]) {
				temp_len ++;
			}
			temp_v_len[i] = temp_len;
		}
		int max_v_len = *std::max_element(temp_v_len, temp_v_len + top_vetex_size);
		
		// calculate all the v_index of left edge nodes
		for (i = 0; i < top_vetex_size; i ++) {
			TreeNode* iter;
			int index = 0;
			for (iter = top_vetex[i]; iter != nullptr; iter = iter->children[DOWN]) {
				if (index == 0) {
					m_v[iter] = max_v_len - temp_v_len[i];
				}
				if (iter->children[DOWN] != nullptr) {
					m_v[iter->children[DOWN]] = m_v[iter] + 1;
				}
				index ++;
			}
		}
		
		// generate the matrix
		TreeNode* matrix[w][h] = { nullptr };
		for (i = 0; i < size; i ++) {
			TreeNode* iter = nodes[i];
			matrix[m_h[iter]][m_v[iter]] = iter;
		}
		
		// generate the result
		std::vector<TreeNode*> result = std::vector<TreeNode*>();
		for(i = 0; i < w; i ++) {
			for(j = 0; j < h; j ++) {
				result.push_back(matrix[i][j]);
			}
		}
		
		return result;
	}
}