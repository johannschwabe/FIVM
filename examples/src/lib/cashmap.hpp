#include <unordered_map>
#define GROWTH_RATE 2

template<
    typename Key,
    typename T,
    typename Hash = std::hash<Key>
>
class CustomHashmap : public std::unordered_map<Key, T, Hash> {
public:
  CustomHashmap() {}

  virtual ~CustomHashmap() {
    if(this->bucket_count() >= MAXSIZE) {
      std::cout << "CustomHashmap size: " << this->size() << ", Bucket count" << this->bucket_count() << std::endl;
    }
  }

  std::pair<typename std::unordered_map<Key, T, Hash>::iterator, bool> insert(
      const typename std::unordered_map<Key, T, Hash>::value_type &value) {
    if ((float) (this->size() + 1) / this->bucket_count() > this->max_load_factor() && this->size() < MAXREGULAR) {
      this->rehash(this->bucket_count() * GROWTH_RATE);
    } else if ((float) (this->size() + 1) / this->bucket_count() > this->max_load_factor() && this->size() >= MAXREGULAR) {
      this->rehash(std::max(static_cast<unsigned long>(MAXSIZE), this->bucket_count() * GROWTH_RATE));
    }
    return std::unordered_map<Key, T, Hash>::insert(value);
  }
  T &operator[](const Key &key) {
    if ((float) (this->size() + 1) / this->bucket_count() > this->max_load_factor() && this->size() < MAXREGULAR) {
      this->rehash(this->bucket_count() * GROWTH_RATE);
    } else if ((float) (this->size() + 1) / this->bucket_count() > this->max_load_factor() && this->size() >= MAXREGULAR) {
      this->rehash(std::max(static_cast<unsigned long>(MAXSIZE), this->bucket_count() * GROWTH_RATE));
    }
    return std::unordered_map<Key, T, Hash>::operator[](key);
  }
};
