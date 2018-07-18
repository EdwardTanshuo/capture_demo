#pragma once

#include "QuadTree.h"

#include <cpprest/http_client.h>
#include <vector>

struct Barcode {
	int left;
	int right;
	int top;
	int bottom;

	std::wstring text;

	Barcode(web::json::value& value) throw() {
		this->left = value[L"Left"].as_integer();
		this->right = value[L"Right"].as_integer();
		this->top = value[L"Top"].as_integer();
		this->bottom = value[L"Bottom"].as_integer();
		this->text = value[L"Text"].as_string();
	}

	tree::TreeNode to_tree_node() {
		return tree::TreeNode (
			(this->left + this->right) / 2, 
			(this->top + this->bottom) / 2, 
			std::string(this->text.begin(), this->text.end())
		);
	}
};


class BarcodeSorter {
private:
	float	_cos_angle;
	float	_max_distance;
	int		_w;
	int		_h;
public:
	BarcodeSorter(float cos_angle, float max_distance, int w, int h): 
		_cos_angle(cos_angle), 
		_max_distance(max_distance), 
		_w(w), 
		_h(h) {}

	~BarcodeSorter() {}

public:
	std::vector<std::string> process_barcodes(std::vector<Barcode> barcodes) {
		std::vector<tree::TreeNode*> nodes;
		for (auto barcode : barcodes) {
			auto node = barcode.to_tree_node();
			nodes.push_back(&node);
		}
		auto temp_array = tree::TreeNode::sort_nodes(nodes, _cos_angle, _max_distance, _w, _h);
		std::vector<std::string> result;
		for (auto iter : temp_array) {
			result.push_back(iter->barcode);
		}
		return result;
	}
};