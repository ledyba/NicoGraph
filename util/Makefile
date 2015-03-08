.PHONY: all fmt deploy bench test

all: fmt
	GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin go install -v -gcflags -N ./...

fmt:
	gofmt -w src/

deploy: fmt
	GOOS=linux GOARCH=amd64 GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin \
		go install -v -gcflags -N ./...
	scp bin/linux_amd64/exporter ledyba.org:/tmp
	scp bin/linux_amd64/load_test ledyba.org:/tmp

bench:
	GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin go test -bench=. tools/bench

test:
	GOPATH=$(shell pwd):${GOPATH} GOBIN=$(shell pwd)/bin go test louvain

