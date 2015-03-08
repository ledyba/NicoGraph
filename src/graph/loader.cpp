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

GraphLoader::GraphLoader(MYSQL_STMT* stmt__)
:stmt_(stmt__)
,limit_(0)
,videoId_(30,0)
,videoIdLength_(0)
,viewCount_(0)
,tags_(1024,0)
,tagsLength_(0)
{
	std::string const stmt = "SELECT video_id,view_count,tags FROM videos WHERE uploaded_at BETWEEN ? AND ? limit ?";
	if(mysql_stmt_prepare(this->stmt_, stmt.data(), stmt.size())){
		throw std::runtime_error(sprintf("Failed to prepare statement: %s",mysql_stmt_error(this->stmt_)));
	}
	memset(this->inBind_, 0, sizeof(this->inBind_));
	this->inBind_[0].buffer = &this->from_;
	this->inBind_[0].buffer_length = sizeof(this->from_);
	this->inBind_[0].buffer_type = MYSQL_TYPE_DATETIME;
	this->inBind_[1].buffer = &this->to_;
	this->inBind_[1].buffer_length = sizeof(this->to_);
	this->inBind_[1].buffer_type = MYSQL_TYPE_DATETIME;
	this->inBind_[2].buffer = &this->limit_;
	this->inBind_[2].buffer_type = MYSQL_TYPE_LONG;
	if(mysql_stmt_bind_param(this->stmt_, this->inBind_)){
		throw std::runtime_error(sprintf("Failed to bind param: %s",mysql_stmt_error(this->stmt_)));
	}
	memset(this->outBind_, 0, sizeof(this->outBind_));
	this->outBind_[0].buffer = this->videoId_.data();
	this->outBind_[0].buffer_length = this->videoId_.size();
	this->outBind_[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	this->outBind_[0].length = &this->videoIdLength_;

	this->outBind_[1].buffer = &this->viewCount_;
	this->outBind_[1].buffer_length = sizeof(this->viewCount_);
	this->outBind_[1].buffer_type = MYSQL_TYPE_LONG;

	this->outBind_[2].buffer = this->tags_.data();
	this->outBind_[2].buffer_length = this->tags_.size();
	this->outBind_[2].buffer_type = MYSQL_TYPE_VAR_STRING;
	this->outBind_[2].length = &this->tagsLength_;
	if(mysql_stmt_bind_result(this->stmt_, this->outBind_)){
		throw std::runtime_error(sprintf("Failed to bind result: %s",mysql_stmt_error(this->stmt_)));
	}
}
GraphLoader::~GraphLoader() noexcept {
	mysql_stmt_close(this->stmt_);
}


void GraphLoader::loadGraph(MYSQL_TIME const& from, MYSQL_TIME const& to, int const limit)
{
	this->limit_=limit;
	this->from_ = from;
	this->to_ = to;
	int r = mysql_stmt_execute(this->stmt_);
	if (r != 0){
		throw std::runtime_error(sprintf("Error (code %d): %s", r, mysql_stmt_error(this->stmt_)));
	}
	int c = 0;
	while((r = mysql_stmt_fetch(this->stmt_)) == 0){
		c++;
		//LOG(INFO) << this->videoId_.data() << ":" << this->viewCount_ << "<タグ>" << this->tags_.data();
	}
	if(r == MYSQL_NO_DATA) {
	}else if(r == MYSQL_DATA_TRUNCATED) {
		for(unsigned int i=0;i<sizeof(this->outBind_)/sizeof(this->outBind_[0]);i++){
			if(this->outBind_[i].error_value){
				throw std::runtime_error(sprintf("Truncated: code: %d, index: %d", this->outBind_[i].error, i));
			}
		}
	}else{
		throw std::runtime_error(sprintf("Error (code %d): %s", r, mysql_stmt_error(this->stmt_)));
	}
	LOG(INFO) << c << " videos";
}

LoaderPool::LoaderPool()
{
	this->mysql_ = mysql_init(nullptr);
	mysql_options(this->mysql_, MYSQL_SET_CHARSET_NAME, "utf8");
	//mysql_options(this->mysql_, MYSQL_OPT_COMPRESS, 0);
	LOG(INFO) << "Loader Pool initialized: " << this->mysql_ << std::flush;
	if(!mysql_real_connect(mysql_, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DB, MYSQL_PORT, nullptr, 0)){
		throw std::runtime_error(sprintf("Failed to init MYSQL: %s",mysql_error(this->mysql_)));
	}
}
void LoaderPool::back(std::unique_ptr<GraphLoader>&& loader){
	std::lock_guard<std::mutex> lk(this->mutex_);
	this->connections_.emplace_back(std::move(loader));
}
std::unique_ptr<GraphLoader> LoaderPool::get(){
	{
		std::lock_guard<std::mutex> lk(this->mutex_);
		if(!this->connections_.empty()){
			std::unique_ptr<GraphLoader> r(std::move(this->connections_.back()));
			this->connections_.pop_back();
			return std::move(r);
		}
	}
	MYSQL_STMT* stmt = mysql_stmt_init(this->mysql_);
	if(!stmt){
		throw std::runtime_error(sprintf("Failed to init statement: %s",mysql_error(this->mysql_)));
	}
	return std::move(std::unique_ptr<GraphLoader>(new GraphLoader(stmt)));
}

}
