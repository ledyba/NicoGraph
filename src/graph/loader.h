/* coding: utf-8 */
/**
 * nicopp
 *
 * Copyright 2015, PSI
 */
#pragma once
#include <mysql/mysql.h>
#include <vector>
#include <ctime>
#include <mutex>
#include <memory>

namespace nicopp {

class GraphLoader final{
private:
	MYSQL_STMT* stmt_;
private:
	MYSQL_TIME from_;
	MYSQL_TIME to_;
	int limit_;
	MYSQL_BIND inBind_[3];
private:
	MYSQL_BIND outBind_[3];
	std::vector<char> videoId_;
	long unsigned int videoIdLength_;
	int viewCount_;
	std::vector<char> tags_;
	long unsigned int tagsLength_;
public:
	GraphLoader(MYSQL_STMT* stmt);
	~GraphLoader() noexcept;
	void loadGraph(MYSQL_TIME const& from, MYSQL_TIME const& to, int const limit);
};

class LoaderPool final {
public:
	class Session final{
		LoaderPool& pool_;
		std::unique_ptr<GraphLoader> loader_;
	public:
		Session() = delete;
		Session(Session const&) = delete;
		Session(Session&&) = delete;
		Session& operator= (Session&&) = delete;
		Session& operator= (Session const&) = delete;
		Session(LoaderPool& pool)
		:pool_(pool)
		,loader_(std::move(pool.get()))
		{
		}
		GraphLoader* operator->() {
			return this->loader_.get();
		}
		GraphLoader const* operator->() const {
			return this->loader_.get();
		}
		~Session() noexcept{
			pool_.back(std::move(loader_));
		}
	};
private:
	MYSQL* mysql_;
	std::mutex mutex_;
	std::vector< std::unique_ptr<GraphLoader> > connections_;
	void back(std::unique_ptr<GraphLoader>&& loader);
	std::unique_ptr<GraphLoader> get();
public:
	LoaderPool();
	~LoaderPool() noexcept = default;
public:

};
}

