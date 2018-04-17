package main

import (
	"fmt"
	"sync"
	"time"
)

func main() {
	wg := sync.WaitGroup{}
	start := time.Now()
	for i := 0; i < 10; i++ {
		wg.Add(2)
		go func() {
			defer wg.Done()
			for j := 0; j < 34242440; j++ {

			}
		}()

		go func(num int) {
			defer wg.Done()
			a := 0
			b := 1
			for index := 0; index < num; index++ {
				temp := a
				a = a + b
				b = temp
			}
		}(i)
	}
	wg.Wait()
	t := time.Now()
	elapsed := t.Sub(start)
	ms := elapsed.Nanoseconds() / 1000000
	fmt.Printf("Go execution finished, time is %v mss \n", ms)
}
