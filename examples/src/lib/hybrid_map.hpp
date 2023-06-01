#include <unordered_map>
#include "tlx/container/btree_map.hpp"
#include <optional>

#define MAX_REHASH 3
template <
    typename HashMapIter,
    typename BtreeIter
>
class HybridMapIterator {
public:
  using HashMapValueType = typename HashMapIter::value_type;
  using BtreeValueType = typename BtreeIter::value_type;

  using HashMapKeyType = typename std::remove_const<typename HashMapValueType::first_type>::type;
  using BtreeKeyType = typename BtreeValueType::first_type;
  using ValueType = std::pair<HashMapKeyType, typename HashMapValueType::second_type>;

  static_assert(std::is_same_v<ValueType, BtreeValueType>,
                "HashMap and BTree must have the same value type");

  bool useBtree;
  std::optional<HashMapIter> hashmapIt;
  std::optional<BtreeIter> btreeIt;

  HybridMapIterator(bool useBtree, std::optional<HashMapIter> hashmapIter, std::optional<BtreeIter> btreeIter)
      : useBtree(useBtree), hashmapIt(hashmapIter), btreeIt(btreeIter) {}

  ValueType operator*() const {
    return useBtree ? *btreeIt.value() : ValueType(hashmapIt.value()->first, hashmapIt.value()->second);
  }

  HybridMapIterator& operator++() {
    if (useBtree && btreeIt.has_value()) {
      ++(*btreeIt);
    } else if (!useBtree && hashmapIt.has_value()) {
      ++(*hashmapIt);
    }
    return *this;
  }

  HybridMapIterator operator++(int) {
    HybridMapIterator tmp(*this); // make a copy for result
    operator++(); // prefix increment this instance
    return tmp;   // return the copy (the old) value
  }


  bool operator==(const HybridMapIterator& other) const {
    if (useBtree != other.useBtree) {
      return false;
    }

    if (useBtree) {
      return btreeIt == other.btreeIt;
    } else {
      return hashmapIt == other.hashmapIt;
    }
  }
  bool operator!=(const HybridMapIterator& other) const {
    return !(*this == other);
  }


};

template <
    typename Key,
    typename T,
    typename Hash = std::hash<Key>,
    typename Pred = std::equal_to<Key>,
    typename Alloc = std::allocator<std::pair<const Key, T>>,
    typename Compare = std::less<Key>
>
class TrackedUnorderedMap : public std::unordered_map<Key, T, Hash, Pred, Alloc> {
private:
  int rehashCount = 0;

public:
  TrackedUnorderedMap() {}

  std::pair<typename std::unordered_map<Key, T, Hash, Pred, Alloc>::iterator, bool> insert(
      const typename std::unordered_map<Key, T, Hash, Pred, Alloc>::value_type& value) {
    if ((float)(this->size() + 1) / this->bucket_count() > this->max_load_factor() && rehashCount < MAX_REHASH) {
      this->rehash(this->bucket_count() * 2);
      rehashCount++;
    } else if ((float)(this->size() + 1) / this->bucket_count() > this->max_load_factor()) {
      rehashCount ++;
    }
    return std::unordered_map<Key, T, Hash, Pred, Alloc>::insert(value);
  }

  T& operator[](const Key& key) {
    if ((float)(this->size() + 1) / this->bucket_count() > this->max_load_factor() && rehashCount < MAX_REHASH) {
      this->rehash(this->bucket_count() * 2);
      rehashCount++;
    } else if ((float)(this->size() + 1) / this->bucket_count() > this->max_load_factor()) {
      rehashCount ++;
    }
    return std::unordered_map<Key, T, Hash, Pred, Alloc>::operator[](key);
  }

  int getRehashCount() {
    return rehashCount;
  }

};

template <
    typename Key,
    typename T,
    typename Hash = std::hash<Key>,
    typename Pred = std::equal_to<Key>,
    typename Alloc = std::allocator<std::pair<const Key, T>>,
    typename Compare = std::less<Key>
>
class HybridMap {
private:
  TrackedUnorderedMap<Key, T, Hash, Pred, Alloc>* hashmap;
  tlx::btree_map<Key, T, Compare> btreemap;

  bool useBtree = false;

public:

  HybridMap() : hashmap(new TrackedUnorderedMap<Key, T, Hash, Pred, Alloc>()) { }

  ~HybridMap() {
    if (!useBtree) {
      delete hashmap;
    }
  }
  // Copy constructor
  HybridMap(const HybridMap& other) : hashmap(new TrackedUnorderedMap<Key, T, Hash, Pred, Alloc>(*other.hashmap)),
                                      btreemap(other.btreemap),
                                      useBtree(other.useBtree) {}

  // Copy assignment operator
  HybridMap& operator=(const HybridMap& other) {
    if (this != &other) {
      delete hashmap;
      hashmap = new TrackedUnorderedMap<Key, T, Hash, Pred, Alloc>(*other.hashmap);
      btreemap = other.btreemap;
      useBtree = other.useBtree;
    }
    return *this;
  }

  void insert(const Key &key, const T &value) {
    if (useBtree) {
      btreemap.insert(std::make_pair(key, value));
    } else {
      if (hashmap->getRehashCount() > MAX_REHASH) {
        for (const auto &pair: *hashmap) {
          btreemap.insert(pair);
        }
        hashmap->reset(); // destructs the hashmap and releases its memory
        useBtree = true;
        btreemap.insert(std::make_pair(key, value));
      } else {
        hashmap->insert(std::make_pair(key, value));
      }
    }
  }
  void erase(const Key& key) {
    if (useBtree) {
      btreemap.erase(key);
    } else {
      hashmap->erase(key);
    }
  }

  T find(const Key& key) {
    if (useBtree) {
      auto it = btreemap.find(key);
      return it != btreemap.end() ? it->second : T();
    } else {
      auto it = hashmap->find(key);
      return it != hashmap->end() ? it->second : T();
    }
  }

  size_t size() const {
    return useBtree ? btreemap.size() : hashmap->size();
  }

  void clear(){
    if (useBtree) {
      btreemap.clear();
    } else {
      hashmap->clear();
    }
  }

  bool empty() const {
    return size() == 0;
  }


  auto begin() {
    if (useBtree) {
      return HybridMapIterator<decltype(hashmap->begin()), decltype(btreemap.begin())>(useBtree, std::nullopt, btreemap.begin());
    } else {
      return HybridMapIterator<decltype(hashmap->begin()), decltype(btreemap.begin())>(useBtree, hashmap->begin(), std::nullopt);
    }
  }

  auto end() {
    if (useBtree) {
      return HybridMapIterator<decltype(hashmap->end()), decltype(btreemap.end())>(useBtree, std::nullopt, btreemap.end());
    } else {
      return HybridMapIterator<decltype(hashmap->end()), decltype(btreemap.end())>(useBtree, hashmap->end(), std::nullopt);
    }
  }

  // And for const iterators:

  auto begin() const {
    if (useBtree) {
      return HybridMapIterator<decltype(hashmap->begin()), decltype(btreemap.begin())>(useBtree, std::nullopt, btreemap.begin());
    } else {
      return HybridMapIterator<decltype(hashmap->begin()), decltype(btreemap.begin())>(useBtree, hashmap->begin(), std::nullopt);
    }
  }

  auto end() const {
    if (useBtree) {
      return HybridMapIterator<decltype(hashmap->end()), decltype(btreemap.end())>(useBtree, std::nullopt, btreemap.end());
    } else {
      return HybridMapIterator<decltype(hashmap->end()), decltype(btreemap.end())>(useBtree, hashmap->end(), std::nullopt);
    }
  }


  T& operator[](const Key& key) {
    if (useBtree) {
      return btreemap[key];
    } else if (hashmap->getRehashCount() > MAX_REHASH) {
      for (const auto &pair: *hashmap) {
        btreemap.insert(pair);
      }
      delete hashmap; // destructs the hashmap and releases its memory
      useBtree = true;
      return btreemap[key];
    } else {
      return (*hashmap)[key];
    }
  }



};
