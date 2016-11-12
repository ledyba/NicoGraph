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
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <signal.h>
#include "../nico/file.h"
#include "../util/fmt.h"
#include <cstdio>

typedef websocketpp::server<websocketpp::config::asio_tls> server;
typedef websocketpp::lib::shared_ptr<websocketpp::transport::asio::ssl::context> context_ptr;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
class Server;
class Session;

/** For TLS **/
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

std::string get_password() {
    return "";
}

context_ptr on_tls_init(tls_mode mode, websocketpp::connection_hdl hdl) {
    namespace asio = websocketpp::lib::asio;

    std::cout << "on_tls_init called with hdl: " << hdl.lock().get() << std::endl;
    std::cout << "using TLS mode: " << (mode == MOZILLA_MODERN ? "Mozilla Modern" : "Mozilla Intermediate") << std::endl;

    context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);

    try {
      ctx->set_options(asio::ssl::context::default_workarounds |
                       asio::ssl::context::no_sslv2 |
                       asio::ssl::context::no_sslv3 |
                       asio::ssl::context::no_tlsv1 |
                       asio::ssl::context::single_dh_use);
      ctx->set_password_callback(bind(&get_password));
      ctx->use_certificate_chain_file(FLAGS_cert);
      ctx->use_private_key_file(FLAGS_privkey, asio::ssl::context::pem);

      // Example method of generating this file:
      // `openssl dhparam -out dh.pem 2048`
      // Mozilla Intermediate suggests 1024 as the minimum size to use
      // Mozilla Modern suggests 2048 as the minimum size to use.
      ctx->use_tmp_dh_file("dh.pem");

      std::string ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK";

      if (SSL_CTX_set_cipher_list(ctx->native_handle() , ciphers.c_str()) != 1) {
          std::cout << "Error setting cipher list" << std::endl;
      }
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    return ctx;
}


/**  **/
DEFINE_int32(port, 9002, "Port to listen on with HTTP protocol");
DEFINE_string(cert, "server.pem", "Server Certificate File");
DEFINE_string(privkey, "server.pem", "Server Private key File");
std::shared_ptr<nicopp::DataSet> dataSet;
std::shared_ptr<Server> serv;

class Session final
{
  std::vector<nicopp::TagGraph> graphs_;
  int level_;
  int group_;
public:
  Session()
  :graphs_()
  ,level_(-1)
  ,group_(-1){

  };
  ~Session() noexcept = default;
  void zoomIn(int const id);
  void zoomOut();
  void out(server& serv, websocketpp::connection_hdl hdl, server::message_ptr msg) {
    LOG(INFO) << "Out at " << group_ << ", level " << level_;
    std::stringstream ss;
    std::stringstream nodess;
    std::stringstream linkss;
    if(level_ >= 0){
      std::vector<nicopp::TagNode> const& nodes = graphs_.back().nodes();
      for(unsigned int i=0;i<nodes.size();++i){
        printOne(nodess, linkss, dataSet, i, nodes[i]);
      }
    }else{
      auto const& nodes = graphs_.at(graphs_.size()+level_-1).nodes();
      for(auto const child : graphs_.at(graphs_.size()+level_).nodes().at(group_).children()){
        printOne(nodess, linkss, dataSet, child, nodes.at(child));
      }
    }
    ss << "{\"nodes\":[" << nodess.str() << "],\"edges\":[" << linkss.str() << "]}";
    serv.send(hdl, ss.str(), msg->get_opcode());
  }
  void seek(int64_t const from, int64_t const to){
    LOG(INFO) << "Seek from " << from << " to " << to;
    // clustering
    using nicopp::TagGraph;
    std::vector<TagGraph> vecs;
    graphs_.clear();
    graphs_.reserve(6);
    graphs_.emplace_back(dataSet->searchTag(from, to, 150000));
    for(int i=0;i<5;++i){
      int const from = graphs_.back().nodes().size();
      this->graphs_.emplace_back(graphs_.back().nextLevel());
      int const to = this->graphs_.back().nodes().size();
      LOG(INFO) << "Stage: " << i+1 << " " << from << " -> " << to;
      if((from-to) < 10){
        graphs_.pop_back();
        break;
      }
    }
    this->group_ = -1;
    this->level_ = 0;
  }
  void onMessage(server& serv, websocketpp::connection_hdl hdl, server::message_ptr msg){
    std::string const payload(msg->get_payload());
    if(std::strncmp(payload.c_str(), "RANGE", 5) == 0){
      serv.send(hdl, nicopp::sprintf("%ld:%ld", dataSet->min(), dataSet->max()), msg->get_opcode());
    }else if(std::strncmp(payload.c_str(), "SEEK ", 5) == 0){
      int64_t from = 0;
      int64_t to = 0;
      sscanf(payload.c_str(), "SEEK %ld:%ld", &from, &to);
      this->seek(from, to);
      this->out(serv, hdl, msg);
    }else if(std::strncmp(payload.c_str(), "ZOOMIN ", 7) == 0){
      if( (unsigned int)(-(level_-1)) < graphs_.size() ){
        --this->level_;
        int64_t to = 0;
        sscanf(payload.c_str(), "ZOOMIN %ld", &to);
        this->group_ = to;
        this->out(serv, hdl, msg);
      }else{
        serv.send(hdl, "{}", msg->get_opcode());
      }
    }else if(std::strncmp(payload.c_str(), "ZOOMOUT", 7) == 0){
      if(level_ < 0){
        int64_t to = this->graphs_.at(graphs_.size()+level_).nodes().at(this->group_).parent();
        this->group_ = to;
        ++this->level_;
        this->out(serv, hdl, msg);
      }else{
        serv.send(hdl, "{}", msg->get_opcode());
      }
    }else{
      LOG(ERROR) << "Handler not found: " << payload;
    }
  }
private:
  void printOne(std::stringstream& nodess, std::stringstream& linkss, std::shared_ptr<nicopp::DataSet> const& dset, int const id, nicopp::TagNode const& node){
    if(node.neighbors().size() <= 0){
      return;
    }
    {
      if(nodess.tellp() > 0){
        nodess  << ",";
      }
    }
    nodess << nicopp::sprintf("{\"id\":%d,\"label\":\"%s\",\"value\":%d}",id, dset->tag(node.payload().tagId), node.selfLoops());
    for(auto const& link : node.neighbors()){
      if (link.first < id){
        continue;
      }
      if(linkss.tellp() > 0){
        linkss << ",";
      }
      linkss << nicopp::sprintf("{\"from\":%d,\"to\":%d,\"value\":%d}", id, link.first, link.second);
    }
  }
};

struct connection_data {
  int sessionid;
  std::string name;
};

class Server {

  typedef std::map<connection_hdl,std::shared_ptr<Session>,std::owner_less<connection_hdl>> SessionMap;

  server spr_;
  SessionMap sessionMap_;
public:
  Server() {
    spr_.init_asio();

    spr_.set_open_handler(bind(&Server::onOpen,this,::_1));
    spr_.set_close_handler(bind(&Server::onClose,this,::_1));
    spr_.set_message_handler(bind(&Server::onMessage,this,::_1,::_2));
		spr_.set_tls_init_handler(on_tls_init);
  }

  void onOpen(connection_hdl hdl) {
    std::shared_ptr<Session> sess(new Session);
    sessionMap_[hdl] = sess;
  }

  void onClose(connection_hdl hdl) {
    std::shared_ptr<Session> sess ( getDataFromHandle(hdl) );
    sessionMap_.erase(hdl);
  }

  void onMessage(connection_hdl hdl, server::message_ptr msg) {
    std::shared_ptr<Session> sess ( getDataFromHandle(hdl) );
    sess->onMessage(spr_, hdl, msg);
  }

  std::shared_ptr<Session> getDataFromHandle(connection_hdl hdl) {
    auto it = sessionMap_.find(hdl);

    if (it == sessionMap_.end()) {
      // this connection is not in the list. This really shouldn't happen
      // and probably means something else is wrong.
      throw std::invalid_argument("No data avaliable for session");
    }

    return it->second;
  }

  void run(uint16_t port) {
    spr_.listen(port);
    spr_.start_accept();
    spr_.run();
  }
  void stop(){
    websocketpp::lib::error_code ec;
    spr_.stop_listening(ec);
    spr_.stop();
  }
};

void onSigint(int signo){
  if (serv) {
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

  serv = std::shared_ptr<Server>(new Server);
  LOG(INFO) << "Server Started: http://localhost:" << FLAGS_port << "/";
  serv->run(FLAGS_port);
  LOG(INFO) << "Server stopped";
  return 0;
}
