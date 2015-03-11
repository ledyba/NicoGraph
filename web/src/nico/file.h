/* coding: utf-8 */
/**
 * nicopp
 *
 * Copyright 2015, PSI
 */
#pragma once
#include <string>
#include <vector>
#include <memory>

namespace nicopp {

struct Video final {
	uint32_t videoId;
	uint32_t viewCount;
	int64_t uploadedAt;
	int32_t tags[10];
};

class DataSet final{
private:
	std::vector<char> tagBuff_;
	std::vector<char> videoIdBuff_;
	std::vector<char const*> const tags_;
	std::vector<char const*> const videoIds_;
	std::vector<Video> const videos_;
public:
	static DataSet Load(std::string const& tagf, std::string const& vf, std::string const& linkf);
	DataSet(std::vector<char>&& tagBuff, std::vector<char>&& videoIdBuff, std::vector<Video>&& videos);
	DataSet() = delete;
	DataSet(DataSet&&) = default;
	DataSet& operator = (DataSet&&) = default;
	DataSet(DataSet const&) = delete;
	DataSet& operator = (DataSet const&) = delete;
	~DataSet() noexcept = default;
public:
	std::vector<const char*> const& tags() const{ return tags_; };
	std::vector<const char*> const& videoIds() const{ return tags_; };
	std::vector<Video> const& videos() const{ return videos_; };
};

}
