#ifndef HMDF_NEFISSERIESMETADATA_H
#define HMDF_NEFISSERIESMETADATA_H

#include <iostream>
#include <string>
#include <vector>

namespace Hmdf {

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

  std::string fromSeries() const;
  void setFromSeries(const std::string &fromSeries);

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
  std::string m_fromSeries;
  bool m_is3d;
  std::vector<size_t> m_dim;
};

}  // namespace Hmdf

#endif  // NEFISSERIESMETADATA_H
