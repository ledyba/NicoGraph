/* coding: utf-8 */
/**
 * nicopp
 *
 * Copyright 2015, PSI
 */

#include "loader.h"

#include <string>
#include <exception>
#include <stdexcept>
#include <mysql/mysql.h>
#include <glog/logging.h>

#include "../util/fmt.h"

#define MYSQL_HOST "192.168.1.130"
#define MYSQL_USER "rails"
#define MYSQL_PASSWORD "rails"
#define MYSQL_DB "nico"

namespace nicopp {

GraphLoader::GraphLoader():mysql_(nullptr){
	this->mysql_ = mysql_init(this->mysql_);
	LOG(INFO) << "MySQL init." << std::flush;
	if(!mysql_real_connect(mysql_, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DB, MYSQL_PORT, nullptr, 0)){
		throw std::runtime_error(mysql_error(this->mysql_));
	}
}
GraphLoader::~GraphLoader() noexcept {
	mysql_close(this->mysql_);
}

void GraphLoader::loadGraph(std::string const& from,std::string const& to, int const limit) {
	std::string const q (sprintf("SELECT video_id,view_count,tags FROM videos WHERE uploaded_at BETWEEN '%s' AND '%s' limit %d;",from,to,limit));
	int const r = mysql_query(this->mysql_, q.c_str());
	if (r != 0){
		throw std::runtime_error(sprintf("Error:%d: %s", r, mysql_error(this->mysql_)));
	}
	MYSQL_RES *res = mysql_use_result(this->mysql_);
	if (!res){
		throw std::runtime_error(sprintf("Error: %s", mysql_error(this->mysql_)));
	}
	MYSQL_ROW row;
	int c = 0;
	while((row = mysql_fetch_row(res))){
		c++;
		//LOG(INFO) << row[0];
	}
	mysql_free_result(res);
}

}
