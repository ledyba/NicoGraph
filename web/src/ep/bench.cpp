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
#include <louvain/louvain.h>
#include "../nico/tag.h"

int main(int argc, char* argv[]) {
	google::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "Running...";
	nicopp::DataSet dset(std::move(nicopp::DataSet::Load("compiled/tagf","compiled/vf","compiled/linkf")));
#ifdef DEBUG
	ProfilerStart ("prof.out");
#endif
	/*{
		long t = std::clock();
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
	}*/
	{
		long t = std::clock();
		nicopp::TagGraph graph = dset.searchTag(0, 99999999999, 150000);
		for(int i=0;i<5;i++){
			LOG(INFO) << graph.nodes().size() << " nodes / " << graph.edges() << " edges";
			graph = std::move(graph.nextLevel(4, .0));
		}
		LOG(INFO) << "Lap: " << (std::clock() - t);
		for (auto const& node : graph.nodes()){
			LOG(INFO) << "  Node: " << dset.tag(node.payload().tagId) << " / "<< node.selfLoops() << "/"<<node.degree();
		}
	}
#ifdef DEBUG
	ProfilerStop ();
#endif
	return 0;
}
