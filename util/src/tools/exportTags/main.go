package main

import (
	"fmt"
	"nico/dist"
)

func main() {
	for i := 0; i <= 1925; i++ {
		metas := make([]nico.MetaInfo, 0)
		metas = nico.LoadDataSet(fmt.Sprintf("/opt/dolce/cache/data/meta/%04d.dat.gz", i), metas, 0)
		for _, meta := range metas {
			for _, tag := range meta.Tags {
				fmt.Printf("%v\n", tag.Tag)
			}
		}
	}
}
