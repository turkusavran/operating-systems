# Virtual Memory Manager Project
Türkü Bengisu Savran, mail: tsavran17@ku.edu.tr
Mehmet Efe Yüzügüler, mail: myuzuguler16@ku.edu.tr

## Run

--compile

```bash
gcc virtmem.c -o a.out

```

--run

```bash
./a.out ./BACKING_STORE.bin addresses.txt

```


### Part 1

- First, we read the corresponding project on the book and then read the chapter 9. Later on we assumed search and add to tlb works as blackbox and start implementing the calculation part of the page offset and the logical page number from the logical_address.

- TLB hit case was already implemented. Therefore, secondly, we implemented the page fault case. We increment page faults counter, update physical page as free page and increment free page. Moreover, we read 1024-byte sized page from BACKING_STORE and then store that in to an available page frame in the physical memory.

- Finally we implemented the helper functions search tlb and add to tlb. In the search tlb we iterate over tlb and return the physical address if it is present and -1 otherwise. In the add to tlb we update the logical and the physical address of the tlbindexed entry with given logical and physical address values. Then we incremented tlbindex value to prevent overlap. 

- To conclude our part 1 works perfectly. We obtained 421 page faults from the given test case.

### Part 2

- Unfortunately we didn't have enough time to work on this part.

