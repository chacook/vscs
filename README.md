# VSCS - Very simple cache server

A 100 lines of code in memory key-value distributed cache.

## To run
1. Run `python3 vscs.py`.
1. Clients use a TCP connection to the server's port and send requests using the protocol below:
```
get request: g<KEY>!
set request: s<KEY>!<DATA>
delete request: d<KEY>!
clear cache request: c!

get response: <DATA> or /x00 if null
set response: !
delete response: !
clear response: !
```
Note: keys cannot contain the delimiter `!`.

## Performance
A basic performance test running 3 simultaneous instances of `client.py`:
```
Completed 3000000 requests in 84.06687784194946 seconds.
Average: 35685.87387817794 per second.
```