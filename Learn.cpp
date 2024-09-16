#include<iostream>
#include<coroutine>
struct promise {
	struct promise_type {
		promise get_return_object() {
			return { std::coroutine_handle<promise_type>::from_promise(*this) };
		}
		std::suspend_never initial_suspend() noexcept { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		void return_void() {}
		void unhandled_exception() {}
	};
	std::coroutine_handle<promise_type> _h;
};
// co_await need this type (three function)
struct Input { 
	bool await_ready() {
		return false; // false -> not ready -> pause
	}
	void await_suspend(std::coroutine_handle<> h) {}
	void await_resume() {}
};
// co_yeild need yiled_value
void Learn() {
	auto lambda = []()->promise {
		Input t;
		std::cout << "exec start\n";
		co_await t;
		std::cout << "exec done\n";
	};
	promise result = lambda();
	std::cout << "main" << std::endl;
	result._h.resume();
}