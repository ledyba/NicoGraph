.PHONY: all fmt deploy bench test tags prof get exp

all: fmt
	GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin go install -v -gcflags -N ./...

fmt:
	gofmt -w src/

deploy: fmt
	GOOS=linux GOARCH=amd64 GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin \
		go install -v -gcflags -N ./...
	scp bin/exporter 7io.org:/tmp
	scp bin/exportTags 7io.org:/tmp

prof:
	GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin go tool pprof bin/fileLoad prof

tags: all
	bin/exportTags > tags
	LANG=C sort --buffer-size=8G tags --parallel=8 -o tags.sorted
	LANG=C uniq -c tags.sorted | sed 's/^ *\([0-9][0-9]*\) /\1\t/' > tags.sorted.uniq
	LANG=C sort -nr tags.sorted.uniq -o tags.sorted.uniq.sorted
	LANG=C cat tags.sorted.uniq.sorted | awk -v limit=2 '$1 > limit{print $2}' > tags.final
	rm tags.sorted.uniq tags.sorted tags

exp:
	bin/exporter

videos:
	mysql -urails -prails --local-infile nico -e "LOAD DATA LOCAL INFILE '/tmp/videos' INTO TABLE videos FIELDS ESCAPED BY '' (video_id, view_count, uploaded_at, @var1) SET tags = UNHEX(@var1)"

bench:
	GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin go test -bench=. tools/bench

test:
	GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin go test louvain

get:
	go get github.com/go-sql-driver/mysql
	go get github.com/ledyba/go-louvain/louvain
