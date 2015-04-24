.PHONY: all mac linux bench prof

all: $(shell if uname -a | grep darwin -i > /dev/null; then echo mac; else echo linux; fi)

linux:
	time bjam toolset=gcc -j8 nicopp
	GLOG_logtostderr=1 GLOG_stderrthreshold=0 bin/gcc-4.9.1/debug/nicopp

mac:
	time bjam toolset=clang -j8 nicopp
	GLOG_logtostderr=1 GLOG_stderrthreshold=0 bin/clang-darwin-4.2.1/debug/nicopp

bench:
	time bjam toolset=gcc -j8 bench
	GLOG_logtostderr=1 GLOG_stderrthreshold=0 bin/gcc-4.9.1/debug/bench

prof:
	google-pprof bin/gcc-4.9.1/debug/bench prof.out

clean:
	time bjam toolset=gcc -j8 clean
