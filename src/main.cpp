/**
 * PSI
 */
#include <thread>
#include <memory>
#include <chrono>
#include <signal.h>

#include <folly/Memory.h>
#include <folly/Portability.h>
#include <folly/io/async/EventBaseManager.h>

#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>

#include <unistd.h>

#include "graph/loader.h"

using namespace proxygen;

using folly::EventBase;
using folly::EventBaseManager;
using folly::SocketAddress;

using Protocol = HTTPServer::Protocol;

DEFINE_int32(http_port, 11000, "Port to listen on with HTTP protocol");
DEFINE_int32(spdy_port, 11001, "Port to listen on with SPDY protocol");
DEFINE_int32(thrift_port, 10000, "Port to listen on for thrift");
DEFINE_string(ip, "localhost", "IP/Hostname to bind to");
DEFINE_int32(threads, 0, "Number of threads to listen on. Numbers <= 0 "
             "will use the number of cores on this machine.");

class EchoHandlerFactory : public RequestHandlerFactory {
 public:
  void onServerStart() noexcept override {
  }

  void onServerStop() noexcept override {
  }

  RequestHandler* onRequest(RequestHandler*, HTTPMessage*) noexcept override {
    return nullptr;
  }

 private:
};

int main(int argc, char* argv[]) {
  nicopp::GraphLoader loader = nicopp::GraphLoader();
  loader.loadGraph("2011-01-01 00:00:00.000000", "2011-02-01 00:00:00.000000", 1400);
  LOG(INFO) << "done";
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  std::vector<HTTPServer::IPConfig> IPs = {
    {SocketAddress(FLAGS_ip, FLAGS_http_port, true), Protocol::HTTP},
    {SocketAddress(FLAGS_ip, FLAGS_spdy_port, true), Protocol::SPDY},
  };

  if (FLAGS_threads <= 0) {
    FLAGS_threads = sysconf(_SC_NPROCESSORS_ONLN);
    CHECK(FLAGS_threads > 0);
  }

  HTTPServerOptions options;
  options.threads = static_cast<size_t>(FLAGS_threads);
  options.idleTimeout = std::chrono::milliseconds(60000);
  options.shutdownOn = {SIGINT, SIGTERM};
  options.handlerFactories = RequestHandlerChain()
      .addThen<EchoHandlerFactory>()
      .build();

  HTTPServer server(std::move(options));
  server.bind(IPs);

  // Start HTTPServer mainloop in a separate thread
  std::thread t([&] () {
    server.start();
  });

  t.join();
  return 0;
}
