#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <random>
#include <mutex>
#include <semaphore>
#include <atomic>
#include <chrono>

// Const
const int simulation_time = 120;
const int threads = 100;

// State enum
enum philoState {
	Sleep,
	LookForSticks,
	Eat,
	PutDownSticks
};

// Global variables
std::vector<std::thread> thread_vec;
std::mutex sticks[threads];
std::mutex co_mutex;
std::atomic_bool thread_check = true;
std::counting_semaphore<threads> sem(threads / 3);

std::random_device rd;
std::mt19937 gen;
std::uniform_int_distribution<> sleep_dist(0, 10);
std::uniform_int_distribution<> eat_dist(0, 5);

// Control variables
std::vector<unsigned long long> stick_bounce;

// State functions
void sleep(philoState* state, int id) {
	int t = 10 + sleep_dist(gen);
	co_mutex.lock();
	std::cout << id << " - Sleep for " << t << std::endl;
	co_mutex.unlock();
	std::this_thread::sleep_for(std::chrono::seconds(t));
	*state = philoState::LookForSticks;
}

void getSticks(philoState* state, int id) {
	while (*state == philoState::LookForSticks) {
		int left = id;
		int right = (id + 1) % threads;
		bool checkL = false;
		bool checkR = false;

		checkL = !sticks[left].try_lock();
		checkR = !sticks[right].try_lock();

		if (!checkR && !checkL) {
			*state = philoState::Eat;
			int t = 15 + eat_dist(gen);
			co_mutex.lock();
			std::cout << id << " - Eat for " << t << std::endl;
			co_mutex.unlock();
			std::this_thread::sleep_for(std::chrono::seconds(t));
		}
		else {
			if (!checkL) {
				sticks[left].unlock();
			}
			if (!checkR) {
				sticks[right].unlock();
			}
			co_mutex.lock();
			std::cout << id << " - Bounce " << ++stick_bounce[id] << "\n";
			co_mutex.unlock();
			// Let first third of bounced philosophers
			sem.acquire();
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
	}
}

void dropSticks(philoState* state, int id) {
	*state = philoState::PutDownSticks;
	int left = id;
	int right = (id + 1) % threads;
	sticks[left].unlock();
	sticks[right].unlock();
	// Let more to the table
	sem.release();
	*state = philoState::Sleep;
}


// The main functions
void philosopherLoop(const int id) {
	srand(time(NULL));
	philoState state = philoState::Sleep;
	while (thread_check)
	{
		sleep(&state, id);
		getSticks(&state, id);
		dropSticks(&state, id);
	}
	co_mutex.lock();
	std::cout << id << " - FINISHED -----------\n";
	co_mutex.unlock();
}

int main() {
	gen = std::mt19937(rd());
	std::cout << "Philosophers: " << threads << std::endl;
	thread_check.store(true);

	// Start threads
	thread_vec.resize(threads);
	stick_bounce.resize(threads);

	int i = 0;
	for (std::thread& thr : thread_vec) {
		thr = std::thread(philosopherLoop, i++);
	}
	if (simulation_time == -1) {
		for (std::thread& thr : thread_vec) {
			thr.join();
		}
	}
	else {
		std::this_thread::sleep_for(std::chrono::seconds(simulation_time));
		thread_check.store(false);
		for (std::thread& thr : thread_vec) {
			thr.join();
		}
	}
	int avg = 0, min = INT32_MAX, max = 0;
	
	for (int val : stick_bounce) {
		avg += val;
		if (min > val) min = val;
		if (max < val) max = val;
	}
	avg /= threads;
	co_mutex.lock();
	std::cout << "Avg: " << avg << ", Min: " << min << ", Max: " << max << std::endl;
	co_mutex.unlock();
	return 0;
}