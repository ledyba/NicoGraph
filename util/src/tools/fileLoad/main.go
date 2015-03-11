package main

import (
	"bufio"
	"encoding/binary"
	"github.com/ledyba/go-louvain/louvain"
	"io"
	"log"
	"os"
	"runtime/pprof"
)

type Video struct {
	VideoId    uint32
	ViewCount  uint32
	UploadedAt uint64
	Tags       [10]int32
}
type DB struct {
	Tags     []string
	VideoIds []string
	Videos   []Video
}

func NewDB() *DB {
	db := &DB{}
	db.Tags = make([]string, 0)
	db.Videos = make([]Video, 0)
	return db
}
func readString(fname string) []string {
	log.Printf("Loading: %v", fname)
	var err error
	rf, err := os.Open(fname)
	defer rf.Close()
	if err != nil {
		panic(nil)
	}
	f := bufio.NewReader(rf)
	var c byte
	var n int
	strs := make([]string, 0)
	for {
		c, err = f.ReadByte()
		if err != nil {
			break
		}
		bstr := make([]byte, int(c))
		pos := 0
		for pos < int(c) {
			n, err = f.Read(bstr[pos:])
			pos += n
			if err != nil {
				break
			}
		}
		if err != nil {
			break
		}
		strs = append(strs, string(bstr))
	}
	if err != io.EOF {
		panic(err)
	}
	return strs
}
func (db *DB) Deserialize(tagf, vf, linkf string) {
	var err error
	db.Tags = readString(tagf)
	db.VideoIds = readString(vf)
	log.Printf("Loading: %v", linkf)
	rf, err := os.Open(linkf)
	defer rf.Close()
	if err != nil {
		panic(nil)
	}
	f := bufio.NewReader(rf)
	for {
		video := Video{}
		err = binary.Read(f, binary.LittleEndian, &video.VideoId)
		if err != nil {
			break
		}
		binary.Read(f, binary.LittleEndian, &video.ViewCount)
		binary.Read(f, binary.LittleEndian, &video.UploadedAt)
		for i := 0; i < 10; i++ {
			binary.Read(f, binary.LittleEndian, &video.Tags[i])
		}
		db.Videos = append(db.Videos, video)
	}
	if err != io.EOF {
		panic(err)
	}
}

func onece(db *DB) {
	nodes := make([]louvain.Node, 0, 200000)
	totalLinks := 0
	tag2index := make([]int, len(db.Tags))
	for v := 0; v < 150000; v++ {
		video := &db.Videos[v]
		tagLength := 0
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
			for j := i + 1; j < 10 && video.Tags[j] >= 0; j++ {
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

func main() {
	db := NewDB()
	db.Deserialize("compiled/tagf", "compiled/vf", "compiled/linkf")
	log.Printf("All Data Loaded: %v videos", len(db.Videos))
	{
		f, err := os.Create("prof")
		if err != nil {
			log.Fatal(err)
		}
		pprof.StartCPUProfile(f)
		defer pprof.StopCPUProfile()
	}
	for i := 0; i < 10; i++ {
		onece(db)
	}
}
