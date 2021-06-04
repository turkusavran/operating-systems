# Seconds of Fame Project
Türkü Bengisu Savran  
mail: tsavran17@ku.edu.tr
Mehmet Efe Yüzügüler  
mail: myuzuguler16@ku.edu.tr

## Run

--compile

```bash
g++ main.cpp -lpthread -fpermissive -o ./main
```

--run

```bash
./main -p 0.3 -n 6 -q 7 -t 15 
```
with the desired parameters

### Part 1
- First, we created -n commantator threads. Then a moderator thread. Commentator threads call the commentatorExec function, moderator calls the moderatorExec function. 
- Moderator asks questions in a for loop of length -q. Commentator also in a for loop of the same length. When moderator asks a question, a broadcast is released to the commantators. Then, they decide whether they will answer the question or not. If they will, they add their thread ID's to the answer queue.
- Moderator pops the id of the threads one by one from the queue, then sends a signal 'you can speak now'. Commantator that receives the signal, speaks for a random time (maximum -t) long. After they are finished, next thread is popped from  the queue.
- After all commentators spoke, moderator moves to the next question until the questions are finished.
- After all the work is done, we join the commentator threads, and moderator thread. Finally, we terminate the program.
### Part 2

### Part 3

- We created a timelog function to print the exact time that an event occurs. It prints the milliseconds. 
