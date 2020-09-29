// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.

///

#include <list>
#include <vector>
#include <utility>

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
        // constructor
    ExpandableHashMap(double maximumLoadFactor = 0.5);
        // destructor; deletes all of the items in the hashmap
    ~ExpandableHashMap();
        
        // resets the hashmap back to 8 buckets, deletes all items
    void reset();
        // return the number of associations in the hashmap
    int size() const;
    
        // The associate method associates one item (key) with another (value).
        // If no association currently exists with that key, this method inserts
        // a new association into the hashmap with that key-value pair.
        // If there is already an association with that key in the hashmap, then
        // the tiem associated with that key is replaced by the second parameter
        // (the value), i.e. it is updated
        // Thus, the hashmap must contain no duplicate keys.
    void associate(const KeyType& key, const ValueType& value);
    
        // If no association exists with the given key, return nullptr; otherwise,
        // return a pointer to the value associated with that key.
        // This pointer can be used to examine that value, and if the hashmap is
        // allowed to be modified, to modify that value directly within the map
        // (which the second overload enables).
        // Using some C++ magic, it is implemented in terms of the first overload,
        // which you must implement
      // for a map that can't be modified, return a pointer to const ValueType
    const ValueType* find(const KeyType& key) const;

      // for a modifiable map, return a pointer to modifiable ValueType
    ValueType* find(const KeyType& key)
    {
        return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
    }

      // C++11 syntax for preventing copying and assignment
    ExpandableHashMap(const ExpandableHashMap&) = delete;
    ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
    std::vector<std::list<std::pair<KeyType, ValueType>>> m_hashmap;
    
    size_t m_size;
    double m_maxLoadFactor;
    
    unsigned int getBucketNumber(const KeyType& key) const {
        unsigned int hasher(const KeyType& k);
        unsigned int h = hasher(key);
        return h % m_hashmap.size();
    }
};

template <typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor) : m_hashmap(8), m_size(0), m_maxLoadFactor(maximumLoadFactor)
{}

template <typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{}  // Using STL containers, so no need to implement my own destructor

template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
    m_hashmap.clear();
    m_size = 0;
    m_hashmap.resize();
}

template <typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
    return m_size;
}

template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
    // If we find the key already exists in our hashmap, we update its value with the new value
    ValueType* ptr = find(key);
    if (ptr != nullptr) {
        *ptr = value;
    }
    
    // Else, we did not find the key, so create a new association
    unsigned int bucketNum = getBucketNumber(key);
    m_hashmap[bucketNum].emplace_back(key, value);
    m_size++;
    
    // Rehash if we exceed maxLoadFactor
    if (static_cast<double>(m_size) / m_hashmap.size() > m_maxLoadFactor) {
        // Create new vector with increased capacity, and swap this with the current vector
        std::vector<std::list<std::pair<KeyType, ValueType>>> temp(m_hashmap.size() * 2);
        m_hashmap.swap(temp);
        
        // Re-emplace all the values in the new vector, making sure to account for new hashes
        for (std::list<std::pair<KeyType, ValueType>>& bucket : temp) {
            for (auto& [k, v] : bucket) {
                m_hashmap[getBucketNumber(k)].emplace_back(k, v);
            }
        }
    }
}

template <typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
    unsigned int bucketNum = getBucketNumber(key);
    
    for (auto& [k, v] : m_hashmap[bucketNum]) {
        if (k == key) {
            return &v;
        }
    }
    
    // else we return the nullptr
    return nullptr;
}
