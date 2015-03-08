package bench

import (
	"math/rand"
	"testing"
)

func prepareRandomKeys(n int, max int) []int {
	r := make([]int, n)
	for i := range r {
		r[i] = rand.Intn(max)
	}
	return r
}

const MAX = 50

func BenchmarkHash(t *testing.B) {
	keys := prepareRandomKeys(t.N, MAX)
	m := make(map[int]int)
	t.ResetTimer()
	for _, v := range keys {
		m[v]++
	}
}

func BenchmarkArray(t *testing.B) {
	keys := prepareRandomKeys(t.N, MAX)
	m := make([]KV, 0, MAX)
	t.ResetTimer()
	for _, v := range keys {
		found := false
		for i := range m {
			if m[i].key == v {
				m[i].value++
				found = true
				break
			}
		}
		if !found {
			m = append(m, KV{v, 1})
		}
	}
}
