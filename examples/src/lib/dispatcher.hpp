#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <iterator>
#include <initializer_list>
#include <map>

class Dispatcher {
  public:
    std::string name;
    Dispatcher(std::string _name) : name(_name) { }
  virtual ~Dispatcher() { }
    virtual bool has_next() const = 0;
    virtual void next() = 0;
};

template <typename T>
class EventDispatcher : public Dispatcher {
  private:
    typedef typename std::vector<T>::iterator Iterator;
    typedef std::function<void(T&)> Func;

    Iterator it;
    const Iterator end;
    const Func& func;

  public:
    EventDispatcher(std::vector<T>& v, const Func& f, std::string _name) : Dispatcher(_name), it(v.begin()), end(v.end()), func(f){ }

    EventDispatcher(std::vector<T>& v, Func&& f) = delete;

    inline bool has_next() const { return it != end; }

    inline void next() { func(*it++); }
};

template <typename T, size_t BATCH_SZ>
class BatchDispatcher : public Dispatcher {
  private:
    typedef typename std::vector<T>::iterator Iterator;
    typedef std::function<void(const Iterator&, const Iterator&)> Func;

    Iterator it;
    const Iterator end;
    const Func& func;

  public:
    BatchDispatcher(std::vector<T>& v, const Func& f,std::string _name) : Dispatcher(_name), it(v.begin()), end(v.end()), func(f) { }

    BatchDispatcher(std::vector<T>& v, Func&& f) = delete;

    inline bool has_next() const { return it != end; }

    inline void next() {
        size_t dist = std::distance(it, end);
        size_t batchSz = (BATCH_SZ < dist ? BATCH_SZ : dist);
        func(it, it + batchSz); 
        it += batchSz;
    }
};

class Multiplexer : public Dispatcher {
  private:
    std::vector<std::unique_ptr<Dispatcher>> v;
    size_t idx, active;

  public:
  std::map<std::string, long> updating_times;

  Multiplexer(): Dispatcher("Multiplexer"), idx(0), active(0), updating_times() { }

    Multiplexer(std::initializer_list<Dispatcher*> dispatchers) :Dispatcher("Multiplexer"), idx(0), active(0), updating_times() {
        for (auto d : dispatchers) {
            add_dispatcher(d);
        }
    }

    inline bool has_next() const { return active > 0; }

    inline void next() {
      Stopwatch sw;
      sw.restart();
      v[idx]->next();

      if (v[idx]->has_next()) {
          idx++;
      }
      else {
          active--;
          for (size_t i = idx; i < active; i++) {
              std::swap(v[i], v[i + 1]);
          }
      }

      if (idx >= active) { idx = 0; }
      sw.stop();
      updating_times[v[idx]->name] += sw.elapsedTimeInMilliSeconds();
    }

    void add_dispatcher(Dispatcher* d) {
        if (d->has_next()) {
            v.push_back(std::unique_ptr<Dispatcher>(d));
            active++;
        }
    }

    void clear() {
        v.clear();
        idx = 0;
        active = 0;
    }
};

#endif /* DISPATCHER_HPP */