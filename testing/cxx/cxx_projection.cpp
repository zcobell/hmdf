#include <iomanip>
#include <iostream>

#include "projection.h"

int main() {
  bool found_valid_cs = Hmdf::Projection::containsEpsg(26915);
  if (!found_valid_cs) return 1;

  bool found_invalid_cs = Hmdf::Projection::containsEpsg(123456789);
  if (found_invalid_cs) return 1;

  const double x0 = -90.0;
  const double y0 = 29.0;
  const double x_expected = 792271.071071;
  const double y_expected = 3211697.373252;
  double xout = 0.0;
  double yout = 0.0;

  int ierr = Hmdf::Projection::transform(4326, 26915, x0, y0, xout, yout);

  std::cout << std::fixed;
  std::cout << std::setprecision(6);

  std::cout << "Projecting " << x0 << ", " << y0
            << " from 4326 to 26915:  and got: " << xout << ", " << yout
            << ". Expected: " << x_expected << ", " << y_expected << std::endl;

  if (std::abs(xout - x_expected) > 0.0001 ||
      std::abs(yout - y_expected) > 0.0001) {
    return 1;
  }

  std::string cs_name = Hmdf::Projection::epsgDescription(26915);
  std::cout << "EPSG 26915 is: " << cs_name << std::endl;

  return 0;
}
