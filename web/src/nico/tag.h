#include "../graph/graph.h"

namespace nicopp{

class Tag final{

};
struct TagMergeFn final{
	Tag operator()(std::vector<nicopp::Node<Tag>*> const& children){
		return children.front()->payload();
	}
};

}

