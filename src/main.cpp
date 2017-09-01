/*
*	shared_mutex (C) 2017 E. Oriani, ema <AT> fastwebnet <DOT> it
*
*	This file is part of shared_mutex.
*
*	shared_mutex is free software: you can redistribute it and/or modify
*	it under the terms of the GNU Lesser General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	shared_mutex is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with nettop.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <list>
#include <mutex>
#include <sys/time.h>
#include <sys/resource.h>
#include "shared_mutex.h"

#define	N_BUCKETS	4

namespace microbench {
	struct clocks {
		double	real,
			user,
			sys;

		clocks() : real(.0), user(.0), sys(.0) {
		}
	};

	class proc_timer {
		clocks&		c_;
		struct rusage	s_;
		struct timeval	st_;
	public:
		proc_timer(clocks& c) : c_(c) {
			if(gettimeofday(&st_, NULL)) throw std::runtime_error("gettimeofday error");
			if(getrusage(RUSAGE_SELF, &s_)) throw std::runtime_error("getrusage error");
		}

		~proc_timer() {
			struct rusage	e;
			struct timeval	et;
			if(gettimeofday(&et, NULL)) throw std::runtime_error("gettimeofday error");
			if(getrusage(RUSAGE_SELF, &e)) throw std::runtime_error("getrusage error");
			
			auto	f_tv2d = [](const timeval& t, const timeval& f) -> double { return 1.0*(t.tv_sec-f.tv_sec)+1e-6*(t.tv_usec-f.tv_usec); };
			const double	real = f_tv2d(et, st_),
					user = f_tv2d(e.ru_utime, s_.ru_utime),
					sys = f_tv2d(e.ru_stime, s_.ru_stime);
			c_.real += real;
			c_.user += user;
			c_.sys += sys;
		}
	};

	void RW_test(const std::string& label, std::function<void(void)> w_func, std::function<void(void)> r_func, const size_t n_threads, const size_t n_iters, const size_t w_freq, const size_t n_writers) {
		clocks		c;
		{
			proc_timer	_pt(c);
			// test
			std::list<std::thread>	th_list;
			for(size_t i = 0; i < n_threads; ++i) {
				th_list.push_back(
					std::thread( [&](const bool writer) -> void {
						for(size_t j = 0; j < n_iters; ++j) {
							if(writer && ((j%w_freq) == 0)){
								w_func();
							} else {
								r_func();
							}
						}
					},
					i < n_writers
					)
				);
			}
			// wait for threads
			for(auto& i : th_list)
				i.join();
		}
		// some stats...
		std::printf("%6.2f,%6.2f,%6.2f,%24s\t(%lu,%lu,%lu,%lu)\n", c.real, c.user, c.sys, label.c_str(), n_threads, n_iters, w_freq, n_writers);
	}
}

int main(int argc, char *argv[]) {
	try {
		ema::shared_mutex<N_BUCKETS>	sm;
		std::mutex			mtx;
		//
		size_t			a = 0,
					b = 0;

		// increase frequency of writing
		size_t			w_freq_arr[] = {1024, 512, 256, 128, 16, 4};
		std::printf("%6s,%6s,%6s,%24s\n", "real", "user", "sys", "mutex_type");
		for(size_t i = 0; i < sizeof(w_freq_arr)/sizeof(size_t); ++i) {
			microbench::RW_test(
				"ema::shared_mutex",
				[&]() { ema::x_lock<N_BUCKETS> l_(sm); ++a; ++b; },
				[&]() { ema::s_lock<N_BUCKETS> l_(sm); if(a != b) throw std::runtime_error("Fail in test!"); },
				4,
				32*1024*1024,
				w_freq_arr[i],
				4
			);
			microbench::RW_test(
				"std::mutex",
				[&]() { std::lock_guard<std::mutex> l_(mtx); ++a; ++b; },
				[&]() { std::lock_guard<std::mutex> l_(mtx); if(a != b) throw std::runtime_error("Fail in test!"); },
				4,
				32*1024*1024,
				w_freq_arr[i],
				4
			);
		}
	} catch(const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	} catch(...) {
		std::cerr << "Unknown exception" << std::endl;
	}
}

