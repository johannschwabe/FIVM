#include <unordered_map>
#define MAX_SIZE 1500000
#define MAX_REGULAR 30000
#define GROWTH_RATE 2

template<
    typename Key,
    typename T,
    typename Hash = std::hash<Key>
>
class CustomHashmap : public std::unordered_map<Key, T, Hash> {
public:
  CustomHashmap() {}
  std::pair<typename std::unordered_map<Key, T, Hash>::iterator, bool> insert(
      const typename std::unordered_map<Key, T, Hash>::value_type &value) {
    if ((float) (this->size() + 1) / this->bucket_count() > this->max_load_factor() && this->size() < MAX_REGULAR) {
      this->rehash(this->bucket_count() * GROWTH_RATE);
    } else if ((float) (this->size() + 1) / this->bucket_count() > this->max_load_factor() && this->size() >= MAX_REGULAR) {
      this->rehash(MAX_SIZE);
    }
    return std::unordered_map<Key, T, Hash>::insert(value);
  }
  T &operator[](const Key &key) {
    if ((float) (this->size() + 1) / this->bucket_count() > this->max_load_factor() && this->size() < MAX_REGULAR) {
      this->rehash(this->bucket_count() * GROWTH_RATE);
    } else if ((float) (this->size() + 1) / this->bucket_count() > this->max_load_factor() && this->size() >= MAX_REGULAR) {
      this->rehash(MAX_SIZE);
    }
    return std::unordered_map<Key, T, Hash>::operator[](key);
  }
};
