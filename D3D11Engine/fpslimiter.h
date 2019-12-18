#include <windows.h>

class FpsLimiter {
public:
	FpsLimiter() {
		m_hTimer = CreateWaitableTimer(nullptr, true, nullptr);
		SetLimit(60);
	}
	~FpsLimiter() {
		if (m_hTimer) CloseHandle(m_hTimer);
	}
private:
	LARGE_INTEGER m_liWaitDurationRelative;
	HANDLE m_hTimer;
	int m_FramerateLimit = 60;
	bool m_started = false;
public:
	void Start() {
		SetWaitableTimer(m_hTimer, &m_liWaitDurationRelative, 0, nullptr, nullptr, false);
		m_started = true;
	}
	void Wait() {
		if (!m_started) return;
		WaitForSingleObject(m_hTimer, 1000);
		m_started = false;
	}
	int GetLimit() { return m_FramerateLimit; }
	void SetLimit(int framerateLimit) {
		constexpr int MIN_FPS = 1;
		if (framerateLimit < MIN_FPS)
			framerateLimit = MIN_FPS;

		auto m_fpsWaiterMs = (long long)((1000.0 / framerateLimit) * 10000);
		m_liWaitDurationRelative.QuadPart = -m_fpsWaiterMs;
		m_FramerateLimit = framerateLimit;
	}
};