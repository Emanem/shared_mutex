# shared_mutex
A C++11 shared mutex with high emphasis on performance.

Please note this is a template library (released under LGPLv3) hence the main itself is simply a test to show the performance against a `std::mutex`; the current version compiles on Linux, still it should be easy to adapt to Windows or other architectures.

## Building
Simply invoke `make` (`make release` for optimized build) and the main should compile.

## Running
The main is a simple test for performance. What is does is sharing two `size_t` variables and then increment them from one thread evey *w_freq* loops and just reading both of them from other threads.
This is to reproduce the test case of *many R/O - less R/W*, where this kind of `ema::shared_mutex` excels.

Output is going to be a sort-of-table showing the performance of the benchmark. As example
```
  real,  user,   sys,              mutex_type
  2.48,  5.10,  3.52,       ema::shared_mutex	(4,33554432,1024,4)
 12.18, 18.05, 27.29,              std::mutex	(4,33554432,1024,4)
  2.38,  5.16,  3.29,       ema::shared_mutex	(4,33554432,512,4)
 11.89, 17.33, 26.68,              std::mutex	(4,33554432,512,4)
  2.96,  5.87,  4.50,       ema::shared_mutex	(4,33554432,256,4)
 11.87, 17.54, 26.44,              std::mutex	(4,33554432,256,4)
  3.03,  6.31,  4.76,       ema::shared_mutex	(4,33554432,128,4)
 12.08, 17.77, 27.19,              std::mutex	(4,33554432,128,4)
  5.02, 10.69,  8.58,       ema::shared_mutex	(4,33554432,16,4)
 12.51, 18.30, 28.25,              std::mutex	(4,33554432,16,4)
  8.59, 18.08, 13.08,       ema::shared_mutex	(4,33554432,4,4)
 12.71, 18.98, 28.04,              std::mutex	(4,33554432,4,4)
```
Where for every row, you have total *real*, *user* and *system* time (in seconds), the type of mutex and then between brackets how many threads, how many iterations, frequency of writing (i.e. every *x* iterations) and how many threads are allowed to write.

As example, on my [i7-3770k](http://ark.intel.com/products/65523/Intel-Core-i7-3770K-Processor-8M-Cache-up-to-3_90-GHz) (not overclocked) in order for 4 threads to run, execute 32M loops, each thread modify the variables every 1024 iterations, took:
- 2.48 s *real* (a.k.a. wall) time
- 5.1  s *user* time
- 3.52 s *system* time

The same when using a `std::mutex` has been:
- 12.18 s *real* (a.k.a. wall) time
- 18.05 s *user* time
- 27.29 s *system* time

Quite the **difference**, isn't it? :-)

*Updates*

Moved my rig to a [5950x](https://www.amd.com/en/products/cpu/amd-ryzen-9-5950x) and run the same test (4 threads):
```
  real,  user,   sys,              mutex_type
  0.29,  0.93,  0.16,       ema::shared_mutex	(4,33554432,1024,4)
  4.06,  5.51,  9.51,              std::mutex	(4,33554432,1024,4)
  0.38,  1.12,  0.30,       ema::shared_mutex	(4,33554432,512,4)
  4.23,  5.66, 10.02,              std::mutex	(4,33554432,512,4)
  0.51,  1.45,  0.49,       ema::shared_mutex	(4,33554432,256,4)
  4.28,  5.57, 10.28,              std::mutex	(4,33554432,256,4)
  0.84,  2.25,  0.80,       ema::shared_mutex	(4,33554432,128,4)
  4.60,  5.81, 11.11,              std::mutex	(4,33554432,128,4)
  3.03,  7.62,  3.52,       ema::shared_mutex	(4,33554432,16,4)
  6.95,  7.93, 17.04,              std::mutex	(4,33554432,16,4)
 10.75, 27.50, 13.37,       ema::shared_mutex	(4,33554432,4,4)
  7.32,  8.24, 17.58,              std::mutex	(4,33554432,4,4)
```
And then with 16 threads (number of physical cores on this CPU):
```
  real,  user,   sys,              mutex_type
  1.29,  8.23,  8.01,       ema::shared_mutex	(16,33554432,1024,16)
 15.72, 22.81,215.80,              std::mutex	(16,33554432,1024,16)
  1.97, 12.93, 12.56,       ema::shared_mutex	(16,33554432,512,16)
 16.38, 23.04,228.58,              std::mutex	(16,33554432,512,16)
  2.88, 19.64, 18.18,       ema::shared_mutex	(16,33554432,256,16)
 17.45, 24.63,242.72,              std::mutex	(16,33554432,256,16)
  4.39, 32.54, 26.16,       ema::shared_mutex	(16,33554432,128,16)
 19.40, 26.51,269.83,              std::mutex	(16,33554432,128,16)
 20.21,209.27, 88.88,       ema::shared_mutex	(16,33554432,16,16)
 32.93, 43.14,453.16,              std::mutex	(16,33554432,16,16)
 73.96,819.51,312.90,       ema::shared_mutex	(16,33554432,4,16)
 33.63, 43.01,462.74,              std::mutex	(16,33554432,4,16)
```
Clearly shows where this type of mutex excels (low write/high read frequency).

## How do I include in my project(s)?
Feel free to copy the file `shared_mutex.h` and include it wherever needed; see [license](#license).

## How do I use it in my code?
After including the libray, one can use the utility classes `ema::x_lock` and `ema::s_lock` for a nifty RAII e*X*clusive and *S*hared lock respectively. As example:
``` c++
ema::shared_mutex<4>	sm;
if(write_access) {
	ema::x_lock<4>	lock(sm);
	// do your R/W stuff here
}
//
if(read_access) {
	ema::s_lock<4>	lock(sm);
	// do your R/O stuff her
}
```

### What is this class a template?
I could make this class a stadard class (i.e. no template) but being a template allows specifying how many buckets (i.e. the argument to the template) at compile them, thus allowing for compilte time loop unrolling in case the compiler feels to be particularly frisky.

In all honesty, this template can be easily adapted to be a standard class and the number of buckets to be passed at *run-time*. It's a trivial exercise left to the reader :-)

## F.A.Q.s

### Why should I use this instead of `std::mutex`?
Run the main (settings optimized for a 4 real cores CPUs) and see the difference between a `std::mutex` and `ema::shared_mutex`. In case you have some writes but many, many __fast__ parallel reads, then `ema::shared_mutex` would provide a huge difference.

### Is it better than `boost::shared_mutex`?
Not sure, haven't tested it yet - but in general depends very much on the implementation.
Emphasys of `ema::shared_mutex` implementation is that when a thread needs to access the data in R/O mode, unless there's a writer, then the access will be __virtually__ cost free: there is not going to be __any__ cache contention between threads/CPU cores.

### Is it better than `std::shared_mutex`?
See previous Q and A.

### When shouldn't I use `ema::shared_mutex`?
If your R/W access blocks for *some time*, then it's much better to go back to `std::mutex`: in this case the R/O (or other R/W) will just spin and burn CPU resources.
The same applies if you have many writers which again do block for some time: the more the writers (and CPU cycles they block), the better `std::mutex` is.

### Is it fully tested? Can I use it in my projects?
I have been testing it for a while and seems stable; if you want to use it in your projects, please do perform extensive testing. Again I'm not responsible if it breaks your projects...

### Why did you write this?
I was wondering how complex would it be to write something like this from scratch, so I started and possibly produced a *decent prototype*

## License
This software is licensed under the LGPLv3, so you can include the header in your source code and just say thanks - no need to release your sources (unless you modify the template that is).
