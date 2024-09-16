#include <coroutine>
#include <iostream>
struct Generator {
	class ExhaustedException :std::exception {};

	struct promise_type {
		int value = 0;
		bool is_ready = false;
		// ��ʼִ��ʱ������ִ�е���һ�������
		std::suspend_never initial_suspend() { return {}; };

		// ���ǹ����� Generator ������
		std::suspend_always final_suspend() noexcept { return {}; }

		// Ϊ�˼򵥣�������Ϊ�������������в����׳��쳣�����ﲻ���κδ���
		void unhandled_exception() { }

		// ��ֵ��ͬʱҪ����ֵ���� value ����
		std::suspend_always await_transform(int value) {
			this->value = value;
			is_ready = true;
			return {};
		}

		// ����Э�̵ķ���ֵ����
		Generator get_return_object() {
			return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
		}

		std::suspend_always yield_value(int value) {
			this->value = value;
			is_ready = true;
			return {};
		}

		// û�з���ֵ
		void return_void() { }
	};
	std::coroutine_handle<promise_type> handle; // ��¼��frame_ptr
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
		if(handle.done()) return false; // Ҫע����ǣ����������Э�����������Ѿ������ˣ��������ǻ�ȡ��һֱ��false����Ҳ������Ҫ��Э��ִ����֮������ԭ��
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
		// co_await ����Ĺ�ע���ڹ����Լ����ȴ������ϣ���ʹ�� co_yield ���ǹ����Լ���ֵ��ȥ
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
	// co_yield 1,2,3,4,5 ֻ�ᴦ��1
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