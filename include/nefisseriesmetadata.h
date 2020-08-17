#ifndef NEFISSERIESMETADATA_H
#define NEFISSERIESMETADATA_H

#include <iostream>
#include <vector>

#include "types.h"

class NefisSeriesMetadata {
 public:
  NefisSeriesMetadata();

  HmdfString name() const;
  void setName(const HmdfString &name);

  HmdfString quantity() const;
  void setQuantity(const HmdfString &quantity);

  HmdfString units() const;
  void setUnits(const HmdfString &units);

  HmdfString type() const;
  void setType(const HmdfString &type);

  HmdfString description() const;
  void setDescription(const HmdfString &description);

  HmdfString fromSeries() const;
  void setFromSeries(const HmdfString &fromSeries);

  bool is3d() const;
  void setIs3d(bool is3d);

  HmdfVector<size_t> *dim();
  HmdfVector<size_t> cdim() const;
  void setDim(const HmdfVector<size_t> &dim);

  void print(std::ostream &os = std::cout) const;

  friend std::ostream &operator<<(std::ostream &os,
                                  const NefisSeriesMetadata &s);

 private:
  HmdfString m_name;
  HmdfString m_quantity;
  HmdfString m_units;
  HmdfString m_type;
  HmdfString m_description;
  HmdfString m_fromSeries;
  bool m_is3d;
  HmdfVector<size_t> m_dim;
};

#endif  // NEFISSERIESMETADATA_H
