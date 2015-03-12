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

DEFINE_int32(port, 9003, "Port to listen on with HTTP protocol");
typedef websocketpp::server<websocketpp::config::asio> server;

void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
}

int main(int argc, char* argv[]) {
	google::InitGoogleLogging (argv[0]);
	google::InstallFailureSignalHandler();
	server nico_server;
	nico_server.set_message_handler(server::message_handler(on_message));
	nico_server.init_asio();
	nico_server.listen(9002);
	nico_server.start_accept();
	nico_server.run();
}
