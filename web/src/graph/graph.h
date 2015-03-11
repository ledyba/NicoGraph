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
	Node* parent_;
	std::vector<Node*> children_;
	friend class Graph;
public:
	inline void addLink(size_t const to, int const w){
		neighbors_[to] += w;
		degree_ += w;
	}
	inline void addSelfLoop(int const w){
		degree_ += 2*w;
		selfLoops_ += 2*w;
	}
public:
	Node():neighbors_(),selfLoops_(0),degree_(0),parent_(nullptr), children_(){}
	Node(Node const&) = default;
	Node(Node&&) = default;
	Node& operator= (Node&&) = default;
	Node& operator= (Node const&) = default;
	int degree() const { return degree_; }
	int selfLoops() const { return selfLoops_; }
	std::unordered_map<int, int> const& neighbors() const { return neighbors_; }
};

class Graph final{
private:
	size_t totalLinks_;
	std::vector<Node> nodes_;
	Graph(Graph&&) = default;
public:
	inline Graph(size_t totalLinks, std::vector<Node>&& nodes):totalLinks_(totalLinks),nodes_(std::move(nodes)){};
	Graph() = delete;
	~Graph() noexcept = default;
	Graph(Graph const&) = delete;
	Graph& operator= (Graph const&) = delete;
	Graph& operator= (Graph&&) = default;
public:
	Graph nextLevel(int const max, float const precision);
	size_t nodes() const{return nodes_.size();}
	size_t edges() const{return totalLinks_;}
};

}
