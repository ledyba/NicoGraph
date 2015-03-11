package main

import (
	"encoding/binary"
	"github.com/ledyba/go-louvain/louvain"
	"log"
	opener "nico/db"
)

func main() {
	db := opener.Connect("localhost")
	defer db.Close()
	err := db.Ping()
	if err != nil {
		panic(err)
	}
	log.Printf("Connected.")
	rows, err := db.Query("select video_id,view_count,tags from videos where uploaded_at between \"2007-03-05 22:10:10\" and \"2010-03-05 22:10:10\" order by uploaded_at asc limit 150000")
	tag2index := make(map[uint32]int)
	if err != nil {
		panic(err)
	}
	defer rows.Close()
	var videoId string
	var viewCount int
	var tags []byte
	nodes := make([]louvain.Node, 0, 200000)
	totalLinks := 0
	for rows.Next() {
		rows.Scan(&videoId, &viewCount, &tags)
		if len(tags) <= 4 {
			continue
		}
		tlen := len(tags) / 4
		for i := 0; i < tlen; i++ {
			ftag := binary.LittleEndian.Uint32(tags[i*4:])
			fidx, ok := tag2index[ftag]
			if !ok {
				tag2index[ftag] = len(nodes)
				nodes = append(nodes, louvain.Node{
					Data:  ftag,
					Links: make(map[int]int),
				})
				fidx = len(nodes) - 1
			}
			for j := i + 1; j < tlen; j++ {
				ttag := binary.LittleEndian.Uint32(tags[j*4:])
				tidx, ok := tag2index[ttag]
				if !ok {
					tag2index[ttag] = len(nodes)
					nodes = append(nodes, louvain.Node{
						Data:  ttag,
						Links: make(map[int]int),
					})
					tidx = len(nodes) - 1
				}
				fnode := &nodes[fidx]
				fnode.Links[tidx]++
				fnode.Degree++
				tnode := &nodes[tidx]
				tnode.Links[fidx]++
				tnode.Degree++
			}
		}
		totalLinks += tlen * (tlen - 1)
	}
	graph := louvain.MakeNewGraphFromNodes(nodes, totalLinks, func([]*louvain.Node) interface{} { return true })
	log.Printf("done: %v tags, edges: %v", len(graph.Nodes), totalLinks)
	for i := 0; i < 20; i++ {
		graph = graph.NextLevel(10, float32(-1000))
		log.Printf("done: %v tags, edges: %v", len(graph.Nodes), totalLinks)
	}
	log.Printf("Enum all links:")
	for i := range graph.Nodes {
		node := &graph.Nodes[i]
		log.Printf("%v: Self Loop: %v/%v", i, node.SelfLoop, node.Degree)
		for link, weight := range node.Links {
			log.Printf("%v->%v: %v", i, link, weight)
		}
	}
}