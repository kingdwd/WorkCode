#pragma once

#include <unordered_map>
#include <vector>
//#include <shared_mutex>
#include "boost/thread/locks.hpp"
#include "boost/thread/shared_mutex.hpp"

#include "modules/common/util/map_util.h"
#include "spider/common/log.h"

namespace dharma {
namespace planning {

template <typename I, typename T> class IndexedList {
public:
  /**
   * @brief copy object into the container. If the id is already exist,
   * overwrite the object in the container.
   * @param id the id of the object
   * @param object the const reference of the objected to be copied to the
   * container.
   * @return The pointer to the object in the container.
   */
  T *Add(const I id, const T &object) {
    auto obs = Find(id);
    if (obs) {
      AWARN << "object " << id << " is already in container";
      *obs = object;
      return obs;
    } else {
      object_dict_.insert({id, object});
      auto *ptr = &object_dict_.at(id);
      object_list_.push_back(ptr);
      return ptr;
    }
  }

  /**
   * @brief Find object by id in the container
   * @param id the id of the object
   * @return the raw pointer to the object if found.
   * @return nullptr if the object is not found.
   */
  T *Find(const I id) {
    return dharma::common::util::FindOrNull(object_dict_, id);
  }

  /**
   * @brief Find object by id in the container
   * @param id the id of the object
   * @return the raw pointer to the object if found.
   * @return nullptr if the object is not found.
   */
  const T *Find(const I id) const {
    return dharma::common::util::FindOrNull(object_dict_, id);
  }

  /**
   * @brief List all the items in the container.
   * @return the list of const raw pointers of the objects in the container.
   */
  const std::vector<const T *> &Items() const { return object_list_; }

  /**
   * @brief List all the items in the container.
   * @return the unordered_map of ids and objects in the container.
   */
  const std::unordered_map<I, T> &Dict() const { return object_dict_; }

  /**
   * @brief Copy the container with objects.
   */
  IndexedList &operator=(const IndexedList &other) {
    this->object_list_.clear();
    this->object_dict_.clear();
    for (const auto &item : other.Dict()) {
      Add(item.first, item.second);
    }
    return *this;
  }

private:
  std::vector<const T *> object_list_;
  std::unordered_map<I, T> object_dict_;
};

template <typename I, typename T>
class ThreadSafeIndexedList : public IndexedList<I, T> {
public:
  T *Add(const I id, const T &object) {
    //    std::unique_lock<std::shared_mutex> writer_lock(mutex_);
    boost::unique_lock<boost::shared_mutex> writer_lock(mutex_);
    return IndexedList<I, T>::Add(id, object);
  }

  T *Find(const I id) {
    //    std::shared_lock<std::shared_mutex> reader_lock(mutex_);
    boost::shared_lock<boost::shared_mutex> reader_lock(mutex_);
    return IndexedList<I, T>::Find(id);
  }

  std::vector<const T *> Items() const {
    //    std::shared_lock<std::shared_mutex> reader_lock(mutex_);
    boost::shared_lock<boost::shared_mutex> reader_lock(mutex_);
    return IndexedList<I, T>::Items();
  }

private:
  //  mutable std::shared_mutex mutex_;
  mutable boost::shared_mutex mutex_;
};

} // namespace planning
} // namespace dharma
