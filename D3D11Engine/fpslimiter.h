#pragma once

#include <chrono>
#include <thread>
#include <Windows.h>

struct FpsLimiter {
public:
	FpsLimiter() {
		m_frequency = 0;
		m_counterStart = 0;
		m_speedLimit = 0;
		m_enabled = false;
		m_fps = 0;
		m_timeStart = 0;
	}
	void Start() {
		m_timeStart = GetCounter();
		m_enabled = true;
	}
	void Wait() {
		using namespace std;
		using namespace std::chrono;

		if (!m_enabled) {
			return;
		}

		auto timeNow = GetCounter();
		auto frameDuration = timeNow - m_timeStart;
		if (frameDuration < m_speedLimit) {
			auto waittime = m_speedLimit - frameDuration;
			//if (true) {
			if (waittime < 10) {
				while (GetCounter() - timeNow < waittime) {}
			}
			else {
				std::this_thread::sleep_until(high_resolution_clock::now() + duration<int, milli>(((int)waittime / 10) * 10));
				while (GetCounter() - timeNow < waittime) {}
			}
			timeNow += waittime;
		}

		m_timeStart = timeNow;
	}

	void Reset() {
		m_enabled = false;
	}

	int GetLimit() { return m_fps; }
	void SetLimit(int framerateLimit) {
		constexpr int MIN_FPS = 1;
		if (framerateLimit < MIN_FPS)
			framerateLimit = MIN_FPS;

		m_fps = framerateLimit;
		m_enabled = framerateLimit > 0;
		m_timeStart = 0;
		if (m_enabled) {
			m_speedLimit = 1000.0 / framerateLimit;
			LARGE_INTEGER li;

			QueryPerformanceFrequency(&li);
			m_frequency = double(li.QuadPart) / 1000.0;

			QueryPerformanceCounter(&li);
			m_counterStart = li.QuadPart;

			m_timeStart = GetCounter();
		}
	}

private:
	unsigned int m_fps;
	bool m_enabled;
	double m_speedLimit;
	__int64 m_counterStart;
	double m_timeStart;
	double m_frequency;

	double GetCounter() {
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return double(li.QuadPart - m_counterStart) / m_frequency;
	}
};
