package main

import (
	"fmt"
	"math/rand"
	"os"
	"sync"
	"time"
)

const prime = 4222234741
const data = `A purely peer-to-peer version of electronic cash would allow online
payments to be sent directly from one party to another without going through a
financial institution. Digital signatures provide part of the solution, but the main
benefits are lost if a trusted third party is still required to prevent double-spending.
We propose a solution to the double-spending problem using a peer-to-peer network.
The network timestamps transactions by hashing them into an ongoing chain of
hash-based proof-of-work, forming a record that cannot be changed without redoing
the proof-of-work. The longest chain not only serves as proof of the sequence of
events witnessed, but proof that it came from the largest pool of CPU power. As
long as a majority of CPU power is controlled by nodes that are not cooperating to
attack the network, they'll generate the longest chain and outpace attackers. The
network itself requires minimal structure. Messages are broadcast on a best effort
basis, and nodes can leave and rejoin the network at will, accepting the longest
proof-of-work chain as proof of what happened while they were gone. \n`

func main() {
	test_size := 1000

	rate := 25
	lnum_limit := test_size - rate*10
	snum_limit := rate * 10
	lnum := 0
	snum := 0

	result := make([]int, snum_limit)
	for i := 0; i < snum_limit; i++ {
		result[i] = i
	}

	// r := rand.Rand{}
	rand.Seed(0)

	wg := sync.WaitGroup{}
	start := time.Now()
	for i := 0; i < test_size; i++ {
		wg.Add(1)
		if lnum >= lnum_limit {
			go short_task(&wg, &result[snum])
			snum++
			continue
		}
		if snum >= snum_limit {
			go long_task(&wg, lnum)
			lnum++
			continue
		}

		x := rand.Int() % 100
		if x < rate || lnum >= lnum_limit {
			go short_task(&wg, &result[snum])
			snum++
		} else {
			go long_task(&wg, lnum)
			lnum++
		}
	}

	// fmt.Printf("lnum %v, snum %v \n", lnum, snum)
	wg.Wait()
	t := time.Now()
	elapsed := t.Sub(start)
	ms := elapsed.Nanoseconds() / 1000000.0
	fmt.Printf("Go execution finished, time is %v ms : %v ns \n", ms, elapsed.Nanoseconds())
}

func short_task(wg *sync.WaitGroup, num *int) {
	defer wg.Done()
	sum := 0
	// compute sum
	for x := 1; x < *num; x++ {
		sum += x
	}
	*num = (sum * (sum - 2)) % prime
}

func long_task(wg *sync.WaitGroup, arg int) {
	defer wg.Done()
	f, err := os.Create(fmt.Sprintf("goutput/tmp-%d", arg))
	if err != nil {
		fmt.Println("Failed to create")
		return
	}

	wsize, err := f.WriteString(data)
	if err != nil {
		fmt.Println("Failed to write string")
		return
	}
	if wsize < len(data) {
		fmt.Printf("Failed to write all data expected %v, written %v \n", len(data), wsize)
	}
}
