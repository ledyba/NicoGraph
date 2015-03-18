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

 #include "tag.h"

namespace nicopp {

struct Video final {
	uint32_t videoId;
	uint32_t viewCount;
	int64_t uploadedAt;
	int32_t tags[10];
};

struct VideoLess final {
	inline bool operator()(Video const& a, Video const& b){
		return std::less<int64_t>()(a.uploadedAt, b.uploadedAt);
	}
	inline bool operator()(Video const& a, int64_t const& b){
		return std::less<int64_t>()(a.uploadedAt, b);
	}
	inline bool operator()(int64_t const& a, Video const& b){
		return std::less<int64_t>()(a, b.uploadedAt);
	}
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
	inline char const* const& tag(size_t const i) const{ return tags_[i]; };
	inline char const* const& videoId(size_t const i) const{ return tags_[i]; };
	inline Video const& video(size_t const cnt) const{ return videos_[cnt]; };
	inline size_t tags() const{ return tags_.size(); };
	inline size_t videoIds() const{ return tags_.size(); };
	inline size_t videos() const{ return videos_.size(); };
	TagGraph searchTag(uint64_t const from, uint64_t const to, int const limit = -1);
};

}
