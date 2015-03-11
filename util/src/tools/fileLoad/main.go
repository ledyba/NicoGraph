package main

import (
	"github.com/ledyba/go-louvain/louvain"
	"log"
	"os"
	"runtime/pprof"
	file "nico/file/deserialize"
)


func onece(db *file.DB) {
	nodes := make([]louvain.Node, 0, 200000)
	totalLinks := 0
	tag2index := make([]int, len(db.Tags))
	for v := 0; v < 150000; v++ {
		video := &db.Videos[v]
		tagLength := 10
		for i := 0; i < 10; i++ {
			ftag := video.Tags[i]
			if ftag < 0 {
				tagLength = i
				break
			}
			fidx := tag2index[ftag]
			if fidx == 0 {
				tag2index[ftag] = len(nodes) + 1
				nodes = append(nodes, louvain.Node{
					Data:  ftag,
					Links: make(map[int]int),
				})
				fidx = len(nodes) - 1
			} else {
				fidx--
			}
			for j := i + 1; j < 10; j++ {
				ttag := video.Tags[j]
				if ttag < 0 {
					break
				}
				tidx := tag2index[ttag]
				if tidx == 0 {
					tag2index[ttag] = len(nodes) + 1
					nodes = append(nodes, louvain.Node{
						Data:  ttag,
						Links: make(map[int]int),
					})
					tidx = len(nodes) - 1
				} else {
					tidx--
				}
				fnode := &nodes[fidx]
				fnode.Links[tidx]++
				fnode.Degree++
				tnode := &nodes[tidx]
				tnode.Links[fidx]++
				tnode.Degree++
			}
		}
		totalLinks += tagLength * (tagLength - 1)
	}
	graph := louvain.MakeNewGraphFromNodes(nodes, totalLinks, func([]*louvain.Node) interface{} { return true })
	log.Printf("done: %v tags, edges: %v", len(graph.Nodes), totalLinks)
	for i := 0; i < 5; i++ {
		graph = graph.NextLevel(4, 0)
		if i == 0{
			//graph.Print()
		}
		log.Printf("done: %v tags, edges: %v", len(graph.Nodes), totalLinks)
	}
}

func main() {
	log.SetFlags(log.Lmicroseconds)
	db := file.LoadDB("compiled/tagf", "compiled/vf", "compiled/linkf")
	log.Printf("All Data Loaded: %v videos", len(db.Videos))
	{
		f, err := os.Create("prof")
		if err != nil {
			log.Fatal(err)
		}
		pprof.StartCPUProfile(f)
		defer pprof.StopCPUProfile()
	}
	for i := 0; i < 1; i++ {
		onece(db)
	}
}
