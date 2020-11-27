#include <string>
#include <vector>

class Projection {

 public:
  static std::string projVersion();

  static bool containsEpsg(int epsg);

  static int transform(int epsgInput, int epsgOutput, double x, double y, double &outx, double &outy);

  static int transform(int epsgInput,
                       int epsgOutput,
                       const std::vector<double> &x,
                       const std::vector<double> &y,
                       std::vector<double> &outx,
                       std::vector<double> &outy);

  static int cpp(double lambda, double phi, double x, double y, double &outx, double &outy);
  static int cpp(double lambda,
                 double phi,
                 const std::vector<double> &x,
                 const std::vector<double> &y,
                 std::vector<double> &outx,
                 std::vector<double> &outy);

  static int inverseCpp(double lambda, double phi, double x, double y, double &outx, double &outy);

  static int inverseCpp(double lambda,
                        double phi,
                        const std::vector<double> &x,
                        const std::vector<double> &y,
                        std::vector<double> &outx,
                        std::vector<double> &outy);
};
