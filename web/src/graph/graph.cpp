/* coding: utf-8 */
/**
 * nicopp
 *
 * Copyright 2015, PSI
 */

#include "graph.h"
#include <gflags/gflags.h>
#include <glog/logging.h>

namespace nicopp {

void shuffle(std::vector<int>& vec){
	const int size = vec.size();
	for(unsigned int n = size - 1; n >= 1; --n) {
		unsigned int k = std::rand() % (n + 1);
		if(k != n) {
			std::swap(vec[k], vec[n]);
		}
	}
}

Graph Graph::nextLevel(int const max, float const precision){
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
		shuffle(order);
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

}
