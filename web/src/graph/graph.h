/* coding: utf-8 */
/**
 * nicopp
 *
 * Copyright 2015, PSI
 */
#pragma once

#include <vector>
#include <unordered_map>

namespace nicopp {

class Node final {
private:
	std::unordered_map<int, int> neighbors_;
	int selfLoops_;
	int degree_;
};

class Graph final{
private:
	std::vector<Node> nodes_;
public:
	Graph();
	~Graph() noexcept;
	Graph(Graph const&) = delete;
	Graph(Graph&&) = delete;
	Graph& operator= (Graph&&) = delete;
	Graph& operator= (Graph const&) = delete;
};

}
