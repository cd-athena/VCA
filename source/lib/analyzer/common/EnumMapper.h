/* Copyright (C) 2024 Christian Doppler Laboratory ATHENA
 *
 * Authors: Christian Feldmann <christian.feldmann@bitmovin.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *****************************************************************************/

#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>
#include <stdexcept>

/* This class implement mapping of "enum class" values to and from names (string).
 */
template <typename T> class EnumMapper
{
public:
  struct Entry
  {
    Entry(T value, std::string name) : value(value), name(name) {}
    Entry(T value, std::string name, std::string text) : value(value), name(name), text(text) {}
    T           value;
    std::string name;
    std::string text;
  };

  using EntryVector = std::vector<Entry>;

  EnumMapper() = default;
  EnumMapper(const EntryVector &entryVector) : entryVector(entryVector){};

  std::optional<T> getValue(std::string name, bool isText = false) const
  {
    for (const auto &entry : this->entryVector)
      if ((!isText && entry.name == name) || (isText && entry.text == name))
        return entry.value;
    return {};
  }

  std::string getName(T value) const
  {
    for (const auto &entry : this->entryVector)
      if (entry.value == value)
        return entry.name;
    throw std::logic_error(
        "The given type T was not registered in the mapper. All possible enums must be mapped.");
  }

  std::string getText(T value) const
  {
    for (const auto &entry : this->entryVector)
      if (entry.value == value)
        return entry.text;
    throw std::logic_error(
        "The given type T was not registered in the mapper. All possible enums must be mapped.");
  }

  size_t indexOf(T value) const
  {
    for (size_t i = 0; i < this->entryVector.size(); i++)
      if (this->entryVector.at(i).value == value)
        return i;
    throw std::logic_error(
        "The given type T was not registered in the mapper. All possible enums must be mapped.");
  }

  std::optional<T> at(size_t index) const
  {
    if (index >= this->entryVector.size())
      return {};
    return this->entryVector.at(index).value;
  }

  std::vector<T> getEnums() const
  {
    std::vector<T> m;
    for (const auto &entry : this->entryVector)
      m.push_back(entry.value);
    return m;
  }

  std::vector<std::string> getNames() const
  {
    std::vector<std::string> l;
    for (const auto &entry : this->entryVector)
      l.push_back(entry.name);
    return l;
  }

  std::vector<std::string> getTextEntries() const
  {
    std::vector<std::string> l;
    for (const auto &entry : this->entryVector)
      l.push_back(entry.text);
    return l;
  }

  size_t size() const { return this->entryVector.size(); }

  const EntryVector &entries() const { return this->entryVector; }

private:
  EntryVector entryVector;
};
