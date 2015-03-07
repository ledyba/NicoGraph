/* coding: utf-8 */
/**
 * nicopp
 *
 * Copyright 2015, PSI
 */
#pragma once
#include <mysql/mysql.h>
#include <string>

namespace nicopp {

class GraphLoader final{
private:
	MYSQL* mysql_;
public:
	GraphLoader();
	~GraphLoader() noexcept;
	void loadGraph(std::string const& from,std::string const& to, int const limit);
};

}

