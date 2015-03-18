#include "../graph/graph.h"

namespace nicopp{

struct Tag final{
	int tagId;
	int viewCount;
};
struct TagMergeFn final{
	Tag operator()(std::vector<nicopp::Node<Tag>*> const& children){
		int maxView = 0;
		int maxTag;
		for(nicopp::Node<Tag> const*const node : children){
			if(node->payload().viewCount >= maxView){
				maxView = node->payload().viewCount;
				maxTag = node->payload().tagId;
			}
		}
		return Tag{maxTag,maxView};
	}
};

}

