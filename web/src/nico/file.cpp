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
#include <algorithm>
#include <glog/logging.h>

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

TagGraph DataSet::searchTag(uint64_t const from, uint64_t const to, int const limit)
{
	using louvain::Node;
	std::vector<Node<Tag>> nodes;
	size_t totalLink = 0;
	{
		std::vector<size_t> tag2index(this->tags_.size(),0);
		std::vector<std::unordered_map<int,int> > links;
		std::vector<int> degrees;
		std::vector<Tag> tags;
		auto beg = std::lower_bound(this->videos_.begin(), this->videos_.end(), from, VideoLess());
		auto end = std::lower_bound(this->videos_.begin(), this->videos_.end(), to, VideoLess());
		int cnt = 0;
		for (auto it = beg; it < end;++it){
			if(limit >= 0 && cnt >= limit){
				break;
			}
			++cnt;
			Video const& video = *it;
			size_t tagCount = 10;
			for(unsigned int i=0;i<10;++i){
				int const ftag = video.tags[i];
				if(ftag < 0){
					tagCount = i;
					break;
				}
				size_t fidx = tag2index[ftag];
				if(fidx <= 0){
					fidx = links.size();
					links.emplace_back();
					degrees.emplace_back();
					tags.emplace_back(Tag{ftag, 0});
					tag2index[ftag] = fidx+1;
				}else{
					fidx--;
				}
				for(unsigned int j=i+1;j<10;++j){
					int const ttag = video.tags[j];
					if(ttag < 0){
						break;
					}
					size_t tidx = tag2index[ttag];
					if(tidx <= 0){
						tidx = links.size();
						links.emplace_back();
						degrees.emplace_back();
						tags.emplace_back(Tag{ttag, 0});
						tag2index[ttag] = tidx+1;
					}else{
						tidx--;
					}
					++links[fidx][tidx];
					++links[tidx][fidx];
					++degrees[fidx];
					++degrees[tidx];
				}
				tags[fidx].viewCount += video.viewCount;
			}
			totalLink += tagCount * (tagCount - 1);
		}
		nodes.resize(links.size());
		for (unsigned int i = 0;i<nodes.size();++i){
			Node<Tag>& node = nodes[i];
			std::unordered_map<int,int>& link = links[i];
			node.degree(degrees[i]);
			node.payload(tags[i]);
			node.neighbors().insert(node.neighbors().end(), link.begin(), link.end());
		}
	}
	return std::move(TagGraph(totalLink, std::move(nodes)));
}

}
