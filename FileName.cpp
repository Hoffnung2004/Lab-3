template <class T>
// УПРАВЛЕНИЕ СЧЁТЧИКОМ ССЫЛОК НА ОБЪЕКТ.
class smart_shared_ptr {
public:
	// Конструктор по умолчанию
	smart_shared_ptr() : ptr(nullptr), ref_count(nullptr) {}

	// Конструктор с передачей обычного указателя
	smart_shared_ptr(T* raw_ptr) : ptr(raw_ptr), ref_count(new int(1)) {}

	// Copy-конструктор
	smart_shared_ptr(const smart_shared_ptr& other) : ptr(other.ptr), ref_count(other.ref_count) {
		if (ref_count) {
			++(*ref_count);
		}
	}

	// Copy-оператор присваивания
	smart_shared_ptr& operator=(const smart_shared_ptr& other) {
		if (this != &other) {
			// Уменьшаем счетчик ссылок текущего объекта
			release();

			// Копируем указатель и счетчик ссылок
			ptr = other.ptr;
			ref_count = other.ref_count;

			if (ref_count) {
				++(*ref_count);
			}
		}
		return *this;
	}

	// Move-конструктор
	smart_shared_ptr(smart_shared_ptr&& other) noexcept : ptr(nullptr), ref_count(nullptr) {
		swap(other);
	}

	// Move-оператор присваивания
	smart_shared_ptr& operator=(smart_shared_ptr&& other) noexcept {
		if (this != &other) {
			// Освобождаем текущие ресурсы
			release();
			swap(other);
		}
		return *this;
	}

	// Деструктор
	~smart_shared_ptr() {
		release();
	}

	// Оператор разыменования
	T& operator*() const {
		return *ptr;
	}

	// Оператор доступа к членам объекта
	T* operator->() const {
		return ptr;
	}

	// Метод для получения указателя
	T* get() const {
		return ptr;
	}

	// Метод для сброса указателя
	void reset() {
		release();
	}

	// Преобразование к bool (проверка на нулевой указатель)
	explicit operator bool() const {
		return ptr != nullptr;
	}

private:
	T* ptr;
	int* ref_count;

	void release() {
		if (ref_count) {
			if (--(*ref_count) == 0) {
				delete ptr;
				delete ref_count;
			}
		}
		ptr = nullptr;
		ref_count = nullptr;
	}

	void swap(smart_shared_ptr& other) {
		std::swap(ptr, other.ptr);
		std::swap(ref_count, other.ref_count);
	}
};

template <class T>
// УСТРАНЕНИЕ ЦИКЛА ЗАВИСИМОСТИ
class smart_weak_ptr {
public:
	// Конструктор по умолчанию
	smart_weak_ptr() : ptr(nullptr), ref_count(nullptr) {}

	// Copy-конструктор
	smart_weak_ptr(const smart_shared_ptr<T>& shared_ptr) : ptr(shared_ptr.get()), ref_count(shared_ptr.ref_count) {}

	// Copy-оператор присваивания
	smart_weak_ptr& operator=(const smart_shared_ptr<T>& shared_ptr) {
		ptr = shared_ptr.get();
		ref_count = shared_ptr.ref_count;
		return *this;
	}

	// Move-конструктор
	smart_weak_ptr(smart_shared_ptr<T>&& shared_ptr) noexcept : ptr(shared_ptr.get()), ref_count(shared_ptr.ref_count) {
		shared_ptr.reset();
	}

	// Move-оператор присваивания
	smart_weak_ptr& operator=(smart_shared_ptr<T>&& shared_ptr) noexcept {
		ptr = shared_ptr.get();
		ref_count = shared_ptr.ref_count;
		shared_ptr.reset();
		return *this;
	}

	// Метод для проверки, разрушен ли объект
	bool expired() const {
		return ref_count ? *ref_count == 0 : true;
	}

	// Метод для получения shared_ptr из weak_ptr
	smart_shared_ptr<T> lock() const {
		return smart_shared_ptr<T>(*this);
	}

	// Метод для сброса указателя
	void reset() {
		ptr = nullptr;
		ref_count = nullptr;
	}

private:
	T* ptr;
	int* ref_count;
};
class A;
class B;

class A {
public:
	A(smart_shared_ptr<B> b) : b(b) {}
private:
	smart_shared_ptr<B> b;
};

class B {
public:
	B(smart_shared_ptr<A> a) : a(a) {}
private:
	smart_shared_ptr<A> a;
};
int main()
{
	smart_shared_ptr<A> a_ptr(new A(smart_shared_ptr<B>(new B(smart_shared_ptr<A>(nullptr)))));
	smart_shared_ptr<B> b_ptr(new B(a_ptr));
}
