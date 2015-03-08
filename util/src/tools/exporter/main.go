package main

import (
	"fmt"
	"log"
	opener "nico/db"
	"nico/dist"
	"strings"
)

func tags2string(tags []nico.TagItem) string {
	strs := make([]string, len(tags))
	for i := range tags {
		strs[i] = tags[i].Tag
	}
	return strings.Join(strs, ",")
}

func main() {
	db := opener.Connect()
	defer db.Close()
	log.Printf("Connected.")
	for i := 0; i <= 1925; i++ {
		metas := make([]nico.MetaInfo, 0)
		metas = nico.LoadDataSet(fmt.Sprintf("/Users/psi/data/meta/%04d.dat.gz", i), metas, 0)
		for _, meta := range metas {
			_, err := db.Exec("INSERT INTO `nico`.`videos` (`video_id`, `uploaded_at`, `view_count`, `tags`) VALUES (?, ?, ?, ?)", meta.VideoId, meta.UploadTime, meta.ViewCounter, tags2string(meta.Tags))
			log.Printf("%v: %v", meta.VideoId, meta.Title)
			if err != nil {
				panic(err)
			}
		}
	}

}
