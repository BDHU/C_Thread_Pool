package main

import (
	"fmt"
	"sync"
	"time"
)

const prime = 4222234741

func main() {
	test_size := 1000
	result := make([]int, test_size)
        for i:=0; i<test_size; i++ {
		result[i] = i
	}

	wg := sync.WaitGroup{}
	start := time.Now()
	for i := 0; i < test_size; i++ {
		wg.Add(1)

		go func(num *int) {
			defer wg.Done()
			sum := 0
			// compute sum 
			for  x:=1; x<*num; x++ {
				sum += x;
			}
			*num = (sum * (sum-2)) % prime;
		}(&result[i])
	}
	wg.Wait()
	t := time.Now()
	elapsed := t.Sub(start)
	ms := elapsed.Nanoseconds() / 1000000.0
	fmt.Printf("Go execution finished, time is %v ms : %v ns \n", ms, elapsed.Nanoseconds())
}
