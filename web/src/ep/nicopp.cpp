/**
 * PSI
 */
#include <thread>
#include <memory>
#include <chrono>
#include <signal.h>
#include <unistd.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
 #include <signal.h>
 #include "../nico/file.h"
 #include "../util/fmt.h"
 #include <cstdio>

typedef websocketpp::server<websocketpp::config::asio> server;

DEFINE_int32(port, 9002, "Port to listen on with HTTP protocol");
std::shared_ptr<server> serv;
std::shared_ptr<nicopp::DataSet> dataSet;
void printOne(std::stringstream& ss, std::shared_ptr<nicopp::DataSet> const& dset, nicopp::TagNode const& node){
	ss << nicopp::sprintf("{\"title\": \"%s\", \"deg\":%d, \"selfLoop\": %d, \"links\":[", node.degree(), node.selfLoops(), dset->tag(node.payload().tagId));
	for(auto const& link : node.neighbors()){
		ss << link.first << "," << link.second << ",";
	}
	ss << "] }\n";
}

void pullHandler(websocketpp::connection_hdl hdl, server::message_ptr msg) {
	int64_t from = 0;
	int64_t to = 0;
	sscanf(msg->get_payload().c_str(), "%ld:%ld", &from, &to);
	LOG(INFO) << msg->get_payload() << " : " << from << " -> " << to;
	using nicopp::TagGraph;
	std::vector<TagGraph> vecs;
	vecs.reserve(6);
	vecs.emplace_back(dataSet->searchTag(from, to, 150000));
	for(int i=0;i<5;++i){
		int const from = vecs.back().nodes().size();
		vecs.emplace_back(vecs.back().nextLevel());
		int const to = vecs.back().nodes().size();
		LOG(INFO) << "Stage: " << i+1 << " " << from << " -> " << to;
		if((from-to) < 10){
			vecs.pop_back();
			break;
		}
	}
	std::stringstream ss;
	for(auto it = vecs.crbegin(); it != vecs.crend();++it) {
		for(auto node = it->nodes().begin(), end = it->nodes().end(); node != end; ++node){
			printOne(ss, dataSet, *node);
		}
		serv->send(hdl, ss.str(), msg->get_opcode());
		ss.clear();
		break;
	}
}

void firstHandler(websocketpp::connection_hdl hdl, server::message_ptr msg) {
	msg->set_compressed(true);
	if(msg->get_payload() == "TELL RANGE"){
		server::connection_ptr con = serv->get_con_from_hdl(hdl);
		con->set_message_handler(pullHandler);
		serv->send(hdl, nicopp::sprintf("%ld:%ld", dataSet->min(), dataSet->max()), msg->get_opcode());
	}
}

void onSigint(int signo){
	if (serv) {
		websocketpp::lib::error_code ec;
		serv->stop_listening(ec);
		serv->stop();
	}
}

int main(int argc, char* argv[]) {
	using websocketpp::lib::placeholders::_1;
	using websocketpp::lib::placeholders::_2;
	using websocketpp::lib::bind;
	google::InitGoogleLogging(argv[0]);
	google::InstallFailureSignalHandler();
	if (signal(SIGINT, onSigint) == SIG_ERR) {
		LOG(FATAL) << "can't catch SIGINT";
	}

	LOG(INFO) << "Loading DataSet...";
	dataSet = std::shared_ptr<nicopp::DataSet>(new nicopp::DataSet(std::move(nicopp::DataSet::Load("compiled/tagf","compiled/vf","compiled/linkf"))));
	LOG(INFO) << "Done." << std::flush;

	serv = std::shared_ptr<server>(new server);
	serv->set_message_handler(firstHandler);
	serv->init_asio();
	serv->listen(FLAGS_port);
	LOG(INFO) << "Server Started: http://localhost:" << FLAGS_port << "/";
	serv->start_accept();
	serv->run();
	LOG(INFO) << "Server stopped";
}
