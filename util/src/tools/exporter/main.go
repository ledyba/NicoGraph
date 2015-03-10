package main

import (
	"database/sql"
	"encoding/binary"
	"fmt"
	"log"
	opener "nico/db"
	"nico/dist"
	"runtime"
	"sync"

	"strings"
)

func tags2bin(tag2int map[string]uint32, tags []nico.TagItem) []byte {
	var buff = make([]byte, 4*len(tags))
	for i := range tags {
		tag := tags[i].Tag
		id := tag2int[tag]
		binary.LittleEndian.PutUint32(buff[i*4:], (id))
	}
	return buff
}

func fillAllTagIndex(db *sql.DB, tags []string) map[string]uint32 {
	type KV struct {
		key   string
		value uint32
	}
	dic := make(map[string]uint32)
	left := make([]string, 0)
	{
		selStmt, err := db.Prepare("select `id` from `tags` where `name` LIKE ?")
		if err != nil {
			panic(err)
		}
		defer selStmt.Close()
		var id uint32
		for _, tag := range tags {
			r := strings.Replace(tag, `\`, `\\`, -1)
			r = strings.Replace(r, `%`, `\%`, -1)
			r = strings.Replace(r, `_`, `\_`, -1)
			err = selStmt.QueryRow(r).Scan(&id)
			if err == sql.ErrNoRows {
				left = append(left, tag)
			} else if err == nil {
				dic[tag] = id
			} else {
				panic(err)
			}
		}
	}
	{
		batch := (len(left) + PARA) / PARA
		ch := make(chan KV)
		for i := 0; i < PARA; i++ {
			go func(from int) {
				to := from + batch
				if to > len(left) {
					to = len(left)
				}
				if from > len(left) {
					from = len(left)
				}
				if from >= to {
					return
				}
				insStmt, err := db.Prepare("insert into tags (id,name) values (NULL,?)")
				if err != nil {
					panic(err)
				}
				defer insStmt.Close()
				for j := from; j < to; j++ {
					res, err := insStmt.Exec(left[j])
					if err != nil {
						panic(err)
					}
					id64, err := res.LastInsertId()
					if err != nil {
						panic(err)
					}
					ch <- KV{left[j], uint32(id64)}
				}
			}(i * batch)
		}
		for i := 0; i < len(left); i++ {
			kv := <-ch
			dic[kv.key] = kv.value
		}
	}
	return dic
}
func enumAllTags(metas []nico.MetaInfo) []string {
	tagbs := make(map[string]bool)
	tagcs := make(map[string]bool)
	for _, meta := range metas {
		for _, tag := range meta.Tags {
			l := strings.ToLower(tag.Tag)
			if !tagcs[l] {
				tagcs[l] = true
				tagbs[tag.Tag] = true
			}
		}
	}
	tags := make([]string, 0, len(tagbs))
	for k, _ := range tagbs {
		tags = append(tags, k)
	}
	return tags
}

const PARA = 10

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())
	db := opener.Connect("localhost")
	defer db.Close()
	log.Printf("Connected.")
	for i := 0; i <= 1925; i++ {
		metas := make([]nico.MetaInfo, 0)
		metas = nico.LoadDataSet(fmt.Sprintf("/opt/dolce/cache/data/meta/%04d.dat.gz", i), metas, 0)
		tags := fillAllTagIndex(db, enumAllTags(metas))
		log.Printf("Dict loaded")
		continue
		batch := (len(metas) + PARA) / PARA
		wg := sync.WaitGroup{}
		wg.Add(PARA)
		for i := 0; i < PARA; i++ {
			go func(idx int) {
				defer wg.Done()
				stmt, err := db.Prepare("INSERT INTO `nico`.`videos` (`video_id`, `uploaded_at`, `view_count`, `tags`) VALUES (?, ?, ?, ?)")
				if err != nil {
					panic(err)
				}
				defer stmt.Close()
				from := batch * idx
				to := from + batch
				if to > len(metas) {
					to = len(metas)
				}
				if from > len(metas) {
					from = len(metas)
				}
				if from >= to {
					return
				}
				for _, meta := range metas[from:to] {
					tagb := tags2bin(tags, meta.Tags)
					if len(tagb)%4 != 0 && len(tagb) > 40 {
						panic(fmt.Sprintf("Tagb Length: %v", len(tagb)))
					}
					if len(meta.VideoId) <= 0 {
						panic(fmt.Sprintf("meta.VidoId Empty: %v", meta))
					}
					_, err := stmt.Exec(meta.VideoId, meta.UploadTime, meta.ViewCounter, tagb)
					if err != nil {
						panic(err)
					}
				}
			}(i)
		}
		wg.Wait()
	}

}
