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

void onMessage(websocketpp::connection_hdl hdl, server::message_ptr msg) {
	msg->set_compressed(true);
	int64_t from = 0;
	int64_t to = 0;
	sscanf(msg->get_payload().c_str(), "%ld:%ld", &from, &to);
	LOG(INFO) << from << "/" << to;
	using nicopp::TagGraph;
	TagGraph graph = dataSet->searchTag(from, to, 150000);
	serv->send(hdl, nicopp::sprintf("Tag: %d", graph.nodes().size()), msg->get_opcode());
}

void onSigint(int signo){
	if (serv) {
		serv->stop();
		serv->stop_listening();
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
	serv->set_message_handler(onMessage);
	serv->init_asio();
	serv->listen(FLAGS_port);
	LOG(INFO) << "Server Started: http://localhost:" << FLAGS_port << "/";
	serv->start_accept();
	serv->run();
	LOG(INFO) << "Server stopped";
}
