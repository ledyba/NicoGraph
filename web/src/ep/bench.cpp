/**
 * PSI
 */
#include <thread>
#include <memory>
#include <chrono>
#include <signal.h>

#include <gflags/gflags.h>
#include <glog/logging.h>
#define DEBUG
#ifdef DEBUG
//#include <google/profiler.h>
#include <gperftools/profiler.h>
#endif

#include "../original/community.h"
#include "../nico/file.h"
#include "../graph/graph.h"

class Tag final{

};
struct TagMergeFn final{
	Tag operator()(std::vector<nicopp::Node<Tag>*> const& children){
		return children.front()->payload();
	}
};

int main(int argc, char* argv[]) {
	google::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "Running...";
	nicopp::DataSet dset(std::move(nicopp::DataSet::Load("compiled/tagf","compiled/vf","compiled/linkf")));
#ifdef DEBUG
	ProfilerStart ("prof.out");
#endif
	int const from = 0;
	int const to = 150000;
	std::vector<nicopp::Node<Tag>> nodes;
	size_t totalLink = 0;
	{
		std::vector<size_t> tag2index(dset.tags(),0);
		std::vector<std::unordered_map<int,int> > links;
		std::vector<int> degrees;
		for (int v=from;v<to;v++){
			nicopp::Video const& video = dset.video(v);
			size_t tagCount = 10;
			for(unsigned int i=0;i<10;++i){
				int const ftag = video.tags[i];
				if(ftag < 0){
					tagCount = i;
					break;
				}
				size_t fidx = tag2index[ftag];
				if(fidx <= 0){
					fidx = links.size();
					links.emplace_back();
					degrees.emplace_back();
					tag2index[ftag] = fidx+1;
				}else{
					fidx--;
				}
				for(unsigned int j=i+1;j<10;++j){
					int const ttag = video.tags[j];
					if(ttag < 0){
						break;
					}
					size_t tidx = tag2index[ttag];
					if(tidx <= 0){
						tidx = links.size();
						links.emplace_back();
						degrees.emplace_back();
						tag2index[ttag] = tidx+1;
					}else{
						tidx--;
					}
					++links[fidx][tidx];
					++links[tidx][fidx];
					++degrees[fidx];
					++degrees[tidx];
				}
			}
			totalLink += tagCount * (tagCount - 1);
		}
		nodes.resize(links.size());
		for (unsigned int i = 0;i<nodes.size();++i){
			nicopp::Node<Tag>& node = nodes[i];
			std::unordered_map<int,int>& link = links[i];
			node.degree(degrees[i]);
			node.neighbors().insert(node.neighbors().end(), link.begin(), link.end());
		}
	}

	std::clock_t t = std::clock();
	for(unsigned int i=0;i<nodes.size();i++){
		nicopp::Node<Tag>& node = nodes[i];
		node.degree();
	}
	/*
	bool run = true;
	Graph g;
	g.total_weight = totalLink;
	int degreeTotal = 0;
	for(unsigned int i=0;i<nodes.size();i++){
		nicopp::Node& node = nodes[i];
		degreeTotal += node.neighbors().size();
		g.degrees.emplace_back(degreeTotal);
		for(auto& kv : node.neighbors()){
			g.links.emplace_back(kv.first);
			g.weights.emplace_back(kv.second);
		}
	}
	g.nb_links = g.links.size();
	g.nb_nodes = nodes.size();
	for(;run;){
		int from = g.degrees.size();
		Community c(g, 10, 0);
		LOG(INFO) << g.degrees.size() << " / " << g.links.size() << " / " << g.weights.size() << " <> " << c.size;
		run = c.one_level();
		g = c.partition2graph_binary();
		LOG(INFO) << from << " -> " << g.degrees.size() << " Nodes";
	}
	LOG(INFO) << "Lap: " << (std::clock() - t);
	*/

	t = std::clock();
	nicopp::Graph<Tag,TagMergeFn> graph (totalLink, std::move(nodes));
	for(int i=0;i<5;i++){
		LOG(INFO) << graph.nodes() << " nodes / " << graph.edges() << " edges";
		graph = std::move(graph.nextLevel(4, .0));
	}
	LOG(INFO) << "Lap: " << (std::clock() - t);
#ifdef DEBUG
	ProfilerStop ();
#endif
	return 0;
}
