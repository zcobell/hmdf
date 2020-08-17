#include "nefisseriesmetadata.h"

#include <iostream>

NefisSeriesMetadata::NefisSeriesMetadata()
    : m_name("none"),
      m_quantity("none"),
      m_units("none"),
      m_type("none"),
      m_description("none"),
      m_fromSeries("none"),
      m_is3d(false),
      m_dim(0) {}

HmdfString NefisSeriesMetadata::name() const { return m_name; }

void NefisSeriesMetadata::setName(const HmdfString &name) { m_name = name; }

HmdfString NefisSeriesMetadata::quantity() const { return m_quantity; }

void NefisSeriesMetadata::setQuantity(const HmdfString &quantity) {
  m_quantity = quantity;
}

HmdfString NefisSeriesMetadata::units() const { return m_units; }

void NefisSeriesMetadata::setUnits(const HmdfString &units) { m_units = units; }

HmdfString NefisSeriesMetadata::type() const { return m_type; }

void NefisSeriesMetadata::setType(const HmdfString &type) { m_type = type; }

HmdfString NefisSeriesMetadata::description() const { return m_description; }

void NefisSeriesMetadata::setDescription(const HmdfString &description) {
  m_description = description;
}

bool NefisSeriesMetadata::is3d() const { return m_is3d; }

void NefisSeriesMetadata::setIs3d(bool is3d) { m_is3d = is3d; }

HmdfVector<size_t> *NefisSeriesMetadata::dim() { return &m_dim; }

HmdfVector<size_t> NefisSeriesMetadata::cdim() const { return m_dim; }

void NefisSeriesMetadata::setDim(const HmdfVector<size_t> &dim) { m_dim = dim; }

void NefisSeriesMetadata::print(std::ostream &os) const {
  os << "       Name: " << this->name() << "\n";
  os << "   Quantity: " << this->quantity() << "\n";
  os << "      Units: " << this->units() << "\n";
  os << "       Type: " << this->type() << "\n";
  os << "Description: " << this->description() << "\n";
  os << "         3D: " << (this->is3d() == true ? "True" : "False") << "\n";
  os << "  Dimension: " << this->cdim().size() << "\n";
  os << "        Dim: ";
  for (size_t i = 0; i < this->cdim().size(); ++i) {
    os << this->cdim()[i] << " ";
  }
  os << "\n\n";
}

HmdfString NefisSeriesMetadata::fromSeries() const { return m_fromSeries; }

void NefisSeriesMetadata::setFromSeries(const HmdfString &fromSeries) {
  m_fromSeries = fromSeries;
}

std::ostream &operator<<(std::ostream &os, const NefisSeriesMetadata &s) {
  s.print(os);
  return os;
}
