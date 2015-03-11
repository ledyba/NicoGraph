/**
 * PSI
 */
#include <thread>
#include <memory>
#include <chrono>
#include <signal.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "../original/community.h"
#include "../nico/file.h"

int main(int argc, char* argv[]) {
	google::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "Running...";
	Graph g;
	nicopp::DataSet dset(std::move(nicopp::DataSet::Load("compiled/tagf","compiled/vf","compiled/linkf")));
	LOG(INFO) << "DataSet loaded. "<<dset.tags().size()<<" tags / " << dset.videoIds().size() << " videos";
	bool run = true;
	for(;run;){
		Community c(g, 10, 0);
		run = c.one_level();
		g = c.partition2graph_binary();
		LOG(INFO) << g.nb_nodes << " Nodes";
	}
	return 0;
}
