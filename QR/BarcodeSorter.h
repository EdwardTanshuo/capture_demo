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
	std::string well;

	Barcode(web::json::value& value) throw() {
		this->left = value[L"Left"].as_integer();
		this->right = value[L"Right"].as_integer();
		this->top = value[L"Top"].as_integer();
		this->bottom = value[L"Bottom"].as_integer();
		this->text = value[L"Text"].as_string();
		this->well = "Unknown";
	}

	tree::TreeNode* to_tree_node() {
		return new tree::TreeNode (
			(this->left + this->right) / 2, 
			(this->top + this->bottom) / 2, 
			this->text
		);
	}
};


class BarcodeSorter {
private:
	float	_cos_angle;
	float	_max_distance_h;
	float	_max_distance_v;
	int		_w;
	int		_h;
public:
	BarcodeSorter(float cos_angle, float max_distance_h, float max_distance_v, int w, int h):
		_cos_angle(cos_angle), 
		_max_distance_h(max_distance_h), 
		_max_distance_v(max_distance_v),
		_w(w), 
		_h(h) {}

	~BarcodeSorter() {}

public:
	std::vector<std::pair<std::string, std::wstring>> process_barcodes(std::vector<Barcode> barcodes) {
		std::vector<tree::TreeNode*> nodes;
		for (auto barcode : barcodes) {
			auto node = barcode.to_tree_node();
			nodes.push_back(node);
		}

		auto temp_array = tree::TreeNode::sort_nodes(nodes, _cos_angle, _max_distance_h, _max_distance_v, _w, _h);
		std::vector<std::pair<std::string, std::wstring>> result;
		for (auto iter : temp_array) {
			char prefix = iter->y + 'A';
			std::string well;
			well = well + std::string(1, prefix) + std::to_string(iter->x + 1);
			result.push_back(std::make_pair(well, iter->barcode));
		}

		for (auto node : nodes) {
			delete node;
		}

		return result;
	}

	static Barcode peek_barcode(std::vector<Barcode> barcodes, std::wstring text) {
		for (auto iter : barcodes) {
			if (iter.text == text) {
				return iter;
			}
		}
	}
};