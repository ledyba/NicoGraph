/* coding: utf-8 */
/**
 * nicopp
 *
 * Copyright 2015, PSI
 */
#include "file.h"
#include <gflags/gflags.h>
#include <glog/logging.h>

#include <cstdio>
namespace nicopp {

std::vector<char const*> parse(char* const buff) {
	std::vector<char const*> lst;
	size_t pos = 0;
	size_t len = buff[pos];
	pos++;
	for (;len > 0;){
		lst.emplace_back(buff + pos);
		pos += len;
		len = buff[pos];
		buff[pos] = 0;
		pos++;
	}
	return std::move(lst);
}
template <typename T>
std::vector<T> load(std::string const& fname){
	FILE* const f = fopen(fname.c_str(), "rb");
	CHECK_NOTNULL(f);
	const off_t from = ftello(f);
	fseeko(f, 0, SEEK_END);
	const off_t to = ftello(f);
	fseeko(f, 0, SEEK_SET);
	size_t size = to-from;
	std::vector<T> buff;
	buff.resize(size/sizeof(T));
	size_t read = fread(buff.data(), 1, size, f);
	if (read < size){
		LOG(FATAL) << "Size unmatch" << read << " != " << size;
	}
	fclose(f);
	return std::move(buff);
}
DataSet DataSet::Load(std::string const& tagf, std::string const& vf, std::string const& linkf)
{
	return std::move(DataSet(load<char>(tagf), load<char>(vf), load<Video>(linkf)));
}

DataSet::DataSet(std::vector<char>&& tagBuff, std::vector<char>&& videoIdBuff, std::vector<Video>&& videos)
:tagBuff_(std::move(tagBuff))
,videoIdBuff_(std::move(videoIdBuff))
,tags_(std::move(parse(tagBuff_.data())))
,videoIds_(parse(videoIdBuff_.data()))
,videos_(std::move(videos))
{
	LOG(INFO) << "DataSet Loaded." << std::endl
	          << tags_.size() << " tags" << std::endl
	          << videoIds_.size() << " videos" << std::endl
	          << videos_.size() << " entries";
}

}
