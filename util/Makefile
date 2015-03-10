.PHONY: all fmt deploy bench test

all: fmt
	GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin go install -v -gcflags -N ./...

fmt:
	gofmt -w src/

deploy: fmt
	GOOS=linux GOARCH=amd64 GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin \
		go install -v -gcflags -N ./...
	scp bin/linux_amd64/exportTags ledyba.org:/tmp
	scp bin/linux_amd64/exporter ledyba.org:/tmp

rasp: fmt
	GOARM=5 GOOS=linux GOARCH=arm GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin \
		go install -v -gcflags -N ./...
	scp bin/linux_arm/exporter 192.168.1.128:/tmp

tags:
	scp ledyba.org:/tmp/tags tags
	sort --buffer-size=128M tags --parallel=6 -o tags.sorted
	uniq tags.sorted tags.sorted.uniq

bench:
	GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin go test -bench=. tools/bench

test:
	GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin go test louvain

