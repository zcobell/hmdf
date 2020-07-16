#ifndef NEFISSERIESMETADATA_H
#define NEFISSERIESMETADATA_H

#include <iostream>
#include <string>
#include <vector>

class NefisSeriesMetadata {
 public:
  NefisSeriesMetadata();

  std::string name() const;
  void setName(const std::string &name);

  std::string quantity() const;
  void setQuantity(const std::string &quantity);

  std::string units() const;
  void setUnits(const std::string &units);

  std::string type() const;
  void setType(const std::string &type);

  std::string description() const;
  void setDescription(const std::string &description);

  bool is3d() const;
  void setIs3d(bool is3d);

  std::vector<size_t> *dim();
  std::vector<size_t> cdim() const;
  void setDim(const std::vector<size_t> &dim);

  void print(std::ostream &os = std::cout) const;

  friend std::ostream &operator<<(std::ostream &os,
                                  const NefisSeriesMetadata &s);

 private:
  std::string m_name;
  std::string m_quantity;
  std::string m_units;
  std::string m_type;
  std::string m_description;
  bool m_is3d;
  std::vector<size_t> m_dim;
};

#endif  // NEFISSERIESMETADATA_H
