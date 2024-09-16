#include <coroutine>
#include <iostream>
struct Generator {
	class ExhaustedException :std::exception {};

	struct promise_type {
		int value = 0;
		bool is_ready = false;
		// 开始执行时不挂起，执行到第一个挂起点
		std::suspend_never initial_suspend() { return {}; };

		// 总是挂起，让 Generator 来销毁
		std::suspend_always final_suspend() noexcept { return {}; }

		// 为了简单，我们认为序列生成器当中不会抛出异常，这里不做任何处理
		void unhandled_exception() { }

		// 传值的同时要挂起，值存入 value 当中
		std::suspend_always await_transform(int value) {
			this->value = value;
			is_ready = true;
			return {};
		}

		// 构造协程的返回值类型
		Generator get_return_object() {
			return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
		}

		std::suspend_always yield_value(int value) {
			this->value = value;
			is_ready = true;
			return {};
		}

		// 没有返回值
		void return_void() { }
	};
	std::coroutine_handle<promise_type> handle; // 记录了frame_ptr
	int next() const {
		if (has_next()) {
			handle.promise().is_ready = false;
			return handle.promise().value;
		}
		throw ExhaustedException();
	}
	bool has_next() const {
		if(!handle||handle.done()) return false; 
		if(!handle.promise().is_ready) handle.resume();
		if(handle.done()) return false; // 要注意的是，在这里可能协程生命周期已经结束了，导致我们获取的一直是false，这也是我们要在协程执行完之后挂起的原因
		return true;
	}
	~Generator() {
		if(handle) handle.destroy();
	}
	explicit Generator(std::coroutine_handle<promise_type> handle) noexcept:handle(handle) {}
	Generator(Generator&) = delete;
	Generator& operator=(Generator&) = delete;
	Generator(Generator&& generator) noexcept
		: handle(std::exchange(generator.handle, {})) {}
};
Generator sequence() {
	int i = 0;
	while (i < 5) {
		co_yield i++;
		// co_await 更多的关注点在挂起自己，等待别人上，而使用 co_yield 则是挂起自己传值出去
	}
}
Generator fibonacci() {
	co_yield 0; // fib(0)
	co_yield 1; // fib(1)

	int a = 0;
	int b = 1;
	while (true) {
		co_yield a + b; // fib(N), N > 1
		b = a + b;
		a = b - a;
	}
}
Generator fold() {
	co_yield 1;
	co_yield 2;
	// co_yield 1,2,3,4,5 只会处理1
}
template<typename ...TArgs>
Generator static from(TArgs ...args) {
	(co_yield args, ...);
}
void GeneratorCaller() {
	Generator gen = from(1,2,3,4);
	for (int i = 0; i < 10; ++i) {
		if (gen.has_next()) {
			std::cout << gen.next() << std::endl;
		}
		else {
			break;
		}
	}
}