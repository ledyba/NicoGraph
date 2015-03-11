package main

import (
	"github.com/ledyba/go-louvain/louvain"
	"log"
	file "nico/file/deserialize"
	"os"
	"runtime/pprof"
	"time"
)

func toGraph(db *file.DB) *louvain.Graph {
	nodes := make([]louvain.Node, 0, 200000)
	totalLinks := 0
	tag2index := make([]int, len(db.Tags))
	links := make([]map[int]int, 0, 2000)
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
					Data: ftag,
				})
				links = append(links, make(map[int]int))
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
						Data: ttag,
					})
					links = append(links, make(map[int]int))
					tidx = len(nodes) - 1
				} else {
					tidx--
				}
				nodes[fidx].Degree++
				nodes[fidx].Degree++
				links[fidx][tidx]++
				links[tidx][fidx]++
			}
		}
		totalLinks += tagLength * (tagLength - 1)
	}
	for i := range links {
		node := &nodes[i]
		node.Links = make([]louvain.Link, 0, len(links[i]))
		for to, weight := range links[i] {
			node.Links = append(node.Links, louvain.Link{to, weight})
		}
	}
	return louvain.MakeNewGraphFromNodes(nodes, totalLinks, func([]*louvain.Node) interface{} { return true })
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
	graph := toGraph(db)
	now := time.Now()
	for i := 0; i < 10; i++ {
		g := graph
		for j := 0; j < 5; j++ {
			g = g.NextLevel(4, 0)
			if i == 0 {
				//graph.Print()
			}
			log.Printf("done: %v tags, edges: %v", len(g.Nodes), g.Total)
		}
	}
	log.Printf("Epalsed: %v", float64(time.Now().Sub(now).Nanoseconds())/1000.0/1000.0)
}
