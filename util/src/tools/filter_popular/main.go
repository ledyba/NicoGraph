package main

import (
	"fmt"
	nico "nico/dist"
)

func main() {
	metas := make([]nico.MetaInfo, 0)
	for i := 1; i <= 1925; i++ {
		metas = nico.LoadDataSet(fmt.Sprintf("/Users/psi/data/meta/%04d.dat.gz", i), metas, 0)
	}
	fmt.Printf("Total: %d\n", len(metas))
	nico.PrintDataSet("popular.dat.gz", metas)
}
