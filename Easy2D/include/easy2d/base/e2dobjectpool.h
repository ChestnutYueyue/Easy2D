#pragma once
#include <easy2d/core/e2dcommon.h>
#include <easy2d/config/e2dmacros.h>
#include <memory>
#include <mutex>
#include <stack>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>


namespace easy2d {

class Object;

template <typename T> class ObjectPool {
public:
  ObjectPool();

  virtual ~ObjectPool();

  template <typename... Args> T *acquire(Args &&...args);

  void release(T *obj);

  void preallocate(size_t count);

  size_t getAvailableCount() const;

  size_t getInUseCount() const;

  void clear();

private:
  mutable std::mutex _mutex;
  std::stack<T *> _available;
  std::unordered_set<T *> _inUse;
};

class ObjectPoolManager {
public:
  static ObjectPoolManager &getInstance();

  template <typename T> ObjectPool<T> *getPool();

  template <typename T> void preallocate(size_t count);

  void clearAll();

  size_t getTotalAvailableCount() const;

  size_t getTotalInUseCount() const;

private:
  ObjectPoolManager();

  ~ObjectPoolManager();

  ObjectPoolManager(const ObjectPoolManager &) = delete;

  ObjectPoolManager &operator=(const ObjectPoolManager &) = delete;

private:
  mutable std::mutex _poolsMutex;
  std::unordered_map<std::type_index, std::shared_ptr<void>> _pools;
};

template <typename T> ObjectPool<T>::ObjectPool() {}

template <typename T> ObjectPool<T>::~ObjectPool() { clear(); }

template <typename T>
template <typename... Args>
T *ObjectPool<T>::acquire(Args &&...args) {
  std::lock_guard<std::mutex> lock(_mutex);

  if (_available.empty()) {
    T *obj = new T(std::forward<Args>(args)...);
    _inUse.insert(obj);
    return obj;
  }

  T *obj = _available.top();
  _available.pop();
  _inUse.insert(obj);

  new (obj) T(std::forward<Args>(args)...);

  return obj;
}

template <typename T> void ObjectPool<T>::release(T *obj) {
  if (!obj) {
    return;
  }

  std::lock_guard<std::mutex> lock(_mutex);

  auto it = _inUse.find(obj);
  if (it != _inUse.end()) {
    _inUse.erase(it);

    if constexpr (std::is_member_function_pointer<decltype(&T::reset)>::value) {
      obj->reset();
    }

    _available.push(obj);
  }
}

template <typename T> void ObjectPool<T>::preallocate(size_t count) {
  std::lock_guard<std::mutex> lock(_mutex);

  for (size_t i = 0; i < count; ++i) {
    T *obj = new T();
    _available.push(obj);
  }
}

template <typename T> size_t ObjectPool<T>::getAvailableCount() const {
  std::lock_guard<std::mutex> lock(_mutex);
  return _available.size();
}

template <typename T> size_t ObjectPool<T>::getInUseCount() const {
  std::lock_guard<std::mutex> lock(_mutex);
  return _inUse.size();
}

template <typename T> void ObjectPool<T>::clear() {
  std::lock_guard<std::mutex> lock(_mutex);

  while (!_available.empty()) {
    T *obj = _available.top();
    _available.pop();
    delete obj;
  }

  for (T *obj : _inUse) {
    delete obj;
  }
  _inUse.clear();
}

template <typename T> ObjectPool<T> *ObjectPoolManager::getPool() {
  std::lock_guard<std::mutex> lock(_poolsMutex);

  std::type_index typeIndex(typeid(T));
  auto it = _pools.find(typeIndex);

  if (it == _pools.end()) {
    auto pool = std::make_shared<ObjectPool<T>>();
    _pools[typeIndex] = pool;
    return static_cast<ObjectPool<T> *>(pool.get());
  }

  return static_cast<ObjectPool<T> *>(it->second.get());
}

template <typename T> void ObjectPoolManager::preallocate(size_t count) {
  ObjectPool<T> *pool = getPool<T>();
  if (pool) {
    pool->preallocate(count);
  }
}

} // namespace easy2d
