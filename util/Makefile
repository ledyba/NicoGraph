.PHONY: all fmt deploy bench test tags prof

all: fmt
	GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin go install -v -gcflags -N ./...

fmt:
	gofmt -w src/

deploy: fmt
	GOOS=linux GOARCH=amd64 GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin \
		go install -v -gcflags -N ./...
	scp bin/exporter ledyba.org:/tmp
	scp bin/exportVideo ledyba.org:/tmp
	scp bin/exportTags ledyba.org:/tmp

prof:
	GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin go tool pprof bin/fileLoad prof

rasp: fmt
	GOARM=5 GOOS=linux GOARCH=arm GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin \
		go install -v -gcflags -N ./...
	scp bin/linux_arm/exporter 192.168.1.128:/tmp

tags: all
	#bin/exportTags
	LANG=C sort --buffer-size=8G tags --parallel=8 -o tags.sorted
	LANG=C uniq tags.sorted tags.sorted.uniq
	scp tags.sorted.uniq ledyba.org:/tmp/tags
	#mysqlimport -urails -prails -c name -L --use-threads=16 -d --fields-escaped-by='' -v nico tags

videos:
	mysql -urails -prails --local-infile nico -e "LOAD DATA LOCAL INFILE '/tmp/videos' INTO TABLE videos FIELDS ESCAPED BY '' (video_id, view_count, uploaded_at, @var1) SET tags = UNHEX(@var1)"

bench:
	GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin go test -bench=. tools/bench

test:
	GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin go test louvain

