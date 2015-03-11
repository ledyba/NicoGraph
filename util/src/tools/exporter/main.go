package main

import (
	"bufio"
	"fmt"
	"log"
	"nico/dist"
	file "nico/file"
	"os"
	"runtime"
)

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())
	db := file.NewDB()
	total := 0
	for i := 0; i <= 1925; i++ {
		metas := make([]nico.MetaInfo, 0)
		metas = nico.LoadDataSet(fmt.Sprintf("/opt/dolce/cache/data/meta/%04d.dat.gz", i), metas, 0)
		c := 0
		for i := range metas {
			meta := &metas[i]
			if len(meta.Tags) >= 2 && meta.ViewCounter >= 2525 {
				db.AddVideo(meta)
				total++
				c++
			}
		}
		log.Printf("%d videos / total: %v videos", c, total)
	}
	tagf, err := os.Create("tagf")
	if err != nil {
		panic(err)
	}

	defer tagf.Close()
	vf, err := os.Create("vf")
	if err != nil {
		panic(err)
	}
	defer vf.Close()
	linkf, err := os.Create("linkf")
	if err != nil {
		panic(err)
	}
	defer vf.Close()
	btagf := bufio.NewWriter(tagf)
	bvf := bufio.NewWriter(vf)
	blinkf := bufio.NewWriter(linkf)
	db.Serialize(btagf, bvf, blinkf)
	btagf.Flush()
	bvf.Flush()
	blinkf.Flush()

}
