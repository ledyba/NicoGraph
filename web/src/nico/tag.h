#pragma once

#include <louvain/louvain.h>

namespace nicopp{

struct Tag final{
	int tagId;
	int viewCount;
};
struct TagMergeFn final{
	Tag operator()(std::vector<louvain::Node<Tag>*> const& children){
		int maxView = -1;
		int maxTag = -1;
		for(louvain::Node<Tag> const*const node : children){
			if(node->payload().viewCount >= maxView){
				maxView = node->payload().viewCount;
				maxTag = node->payload().tagId;
			}
		}
		return Tag{maxTag,maxView};
	}
};

typedef louvain::Graph<Tag, TagMergeFn> TagGraph;

}

