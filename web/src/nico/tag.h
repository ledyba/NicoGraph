#pragma once

#include <vector>
#include <louvain/louvain.h>

namespace nicopp{

struct Tag final{
	int tagId;
	int viewCount;
};
struct TagMergeFn final{
	Tag operator()(std::vector<louvain::Node<Tag> > const& nodes, std::vector<int> const& children){
		int maxView = -1;
		int maxTag = -1;
		for(int idx : children){
			louvain::Node<Tag> const& node = nodes[idx];
			if(node.payload().viewCount >= maxView){
				maxView = node.payload().viewCount;
				maxTag = node.payload().tagId;
			}
		}
		return Tag{maxTag,maxView};
	}
};

typedef louvain::Graph<Tag, TagMergeFn> TagGraph;
typedef louvain::Node<Tag> TagNode;

}

