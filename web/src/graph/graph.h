/* coding: utf-8 */
/**
 * nicopp
 *
 * Copyright 2015, PSI
 */
#pragma once

#include <vector>
#include <unordered_map>
#include <random>

namespace nicopp {

namespace {
class Shuffler final {
	std::mt19937 mt_;
public:
	Shuffler()
	:mt_(std::random_device()()){

	}
	~Shuffler() noexcept = default;
	void operator()(std::vector<int>& vec){
		const int size = vec.size();
		for(unsigned int n = size - 1; n >= 1; --n) {
			unsigned int k = (mt_()) % (n + 1);
			if(k != n) {
				std::swap(vec[k], vec[n]);
			}
		}
	}
	Shuffler(Shuffler&&) noexcept = default;
	Shuffler& operator=(Shuffler&&) noexcept = default;
	Shuffler(Shuffler const&) = delete;
	Shuffler& operator=(Shuffler const&) = delete;
};
}

class Node final {
private:
	std::vector<std::pair<int, int> > neighbors_;
	int selfLoops_;
	int degree_;
	Node* parent_;
	std::vector<Node*> children_;
	friend class Graph;
public:
	Node() noexcept: neighbors_(),selfLoops_(0),degree_(0),parent_(nullptr), children_(){}
	Node(Node const&) = default;
	Node(Node&&) noexcept = default;
	Node& operator= (Node&&) noexcept = default;
	Node& operator= (Node const&) = default;
	int degree() const { return degree_; }
	void degree(int const deg) { degree_ = deg; }
	int selfLoops() const { return selfLoops_; }
	void selfLoops(int const loop) { selfLoops_ = loop; }
	std::vector<std::pair<int, int> > const& neighbors() const { return neighbors_; }
	std::vector<std::pair<int, int> >& neighbors() { return neighbors_; }
};

class Graph final{
private:
	size_t totalLinks_;
	std::vector<Node> nodes_;
	Shuffler shuffler_;
	Graph(Graph&& m) = default;
public:
	inline Graph(size_t totalLinks, std::vector<Node>&& nodes)
	:totalLinks_(totalLinks)
	,nodes_(std::move(nodes))
	,shuffler_()
	{};
	Graph() noexcept = delete;
	~Graph() noexcept = default;
	Graph(Graph const&) = delete;
	Graph& operator= (Graph const&) = delete;
	Graph& operator= (Graph&&) = default;
public:
	Graph nextLevel(int const max, float const precision) {
		size_t const nNodes = nodes_.size();
		std::vector<int> tmpComm(nNodes);
		{
			const float gTotal = this->totalLinks_;
			std::vector<int> commTotal(nNodes);
			std::vector<int> order(nNodes);
			for(size_t i = 0;i < nNodes;i++){
				tmpComm[i] = i;
				order[i] = i;
				commTotal[i] = this->nodes_[i].degree_;
			}
			
			std::vector<int> neighLinks(nNodes, 0);

			std::vector<int> neighComm;
			neighComm.reserve(nNodes);
			int changed = nNodes;
			int cnt = 0;
			int const changeLimit = nNodes/100;
			shuffler_(order);
			while(changed > changeLimit){
				if(max > 0 && cnt >= max){
					LOG(WARNING) << "Exceed Limit Pass";
					break;
				}
				cnt++;
				changed = 0;
				for(int pos : order){
					Node const& node = nodes_[pos];
					int const nodeTmpComm = tmpComm[pos];
					int const nodeDegree = node.degree_;
					/* Calculating Neighbor Communities */
					for(int comm : neighComm) {
						neighLinks[comm] = 0;
					}
					neighComm.clear();
					for (std::pair<int,int> const& link : node.neighbors_) {
						int const to = tmpComm[link.first];
						int const weight = link.second;
						if (neighLinks[to] <= 0) {
							neighComm.emplace_back(to);
							neighLinks[to] = weight;
						} else {
							neighLinks[to] += weight;
						}
					}
					/* Calculating the BEST community */
					int bestComm = nodeTmpComm;
					float bestGain = precision;
					for(int comm : neighComm) {
						float gain;
						if (comm == nodeTmpComm) {
							gain = float(neighLinks[comm]) - float((commTotal[comm]-nodeDegree))*nodeDegree/gTotal;
						} else {
							gain = float(neighLinks[comm]) - float(commTotal[comm])*nodeDegree/gTotal;
						}
						if (gain > bestGain) {
							bestGain = gain;
							bestComm = comm;
						}
					}
					/* Insert to the best community */
					if (nodeTmpComm != bestComm) {
						changed++;
						tmpComm[pos] = bestComm;
						/* Remove from the original community */
						commTotal[nodeTmpComm] -= nodeDegree;
						/* insert */
						commTotal[bestComm] += nodeDegree;
					}
				}
			}
		}
		//Calc Next nodes:
		std::vector<int> oldCommIdx;
		std::vector<int> c2i(nNodes, 0);
		std::vector<Node> communities;
		communities.reserve(nodes_.size()/10);
		oldCommIdx.reserve(nodes_.size()/10);
		for(unsigned int i=0;i<nNodes;i++){
			Node& node = nodes_[i];
			int const nodeTmpComm = tmpComm[i];
			int const c = c2i[nodeTmpComm];
			if(c <= 0){
				c2i[nodeTmpComm] = communities.size() + 1;
				oldCommIdx.emplace_back(nodeTmpComm);
				communities.emplace_back();
				communities.back().children_.emplace_back(&node);
			}else{
				communities[c-1].children_.emplace_back(&node);
			}
		}
		std::vector<std::unordered_map<int,int> > links(communities.size());
		// Merging edges
		for (unsigned int i=0,maxc=communities.size();i<maxc;i++) {
			Node& comm = communities[i];
			int const oldComm = oldCommIdx[i];
			for (Node* const child : comm.children_) {
				child->parent_ = &comm;
				comm.selfLoops_ += child->selfLoops_;
				comm.degree_ += child->selfLoops_;
				for (std::pair<int,int> const& link : child->neighbors_){
					int const linkToIdx = link.first;
					int const weight = link.second;
					int const cLinkToCommNow = tmpComm[linkToIdx];
					comm.degree_ += weight;
					if(cLinkToCommNow == oldComm){
						comm.selfLoops_ += weight;
					} else {
						links[i][c2i[cLinkToCommNow]-1] += weight;
					}
				}
			}
			std::unordered_map<int,int> const& link = links[i];
			comm.neighbors_.reserve(link.size());
			comm.neighbors_.insert(comm.neighbors_.begin(), link.begin(),link.end());
		}
		return std::move(Graph(totalLinks_, std::move(communities)));
	}
	size_t nodes() const{return nodes_.size();}
	size_t edges() const{return totalLinks_;}
};

}
