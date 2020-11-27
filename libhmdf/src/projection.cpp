
#include "projection.h"

#include <cmath>
#include <string>

#include "constants.h"
#include "proj.h"

bool Projection::containsEpsg(int epsg) { return true; }

int Projection::transform(int epsgInput, int epsgOutput, double x, double y,
                          double &outx, double &outy) {
  std::vector<double> xv = {x};
  std::vector<double> yv = {y};
  std::vector<double> outxv;
  std::vector<double> outyv;
  if (Projection::transform(epsgInput, epsgOutput, xv, yv, outxv, outyv))
    return 1;
  outx = outxv[0];
  outy = outyv[0];
  return 0;
}

int Projection::transform(int epsgInput, int epsgOutput,
                          const std::vector<double> &x,
                          const std::vector<double> &y,
                          std::vector<double> &outx,
                          std::vector<double> &outy) {
  if (x.size() != y.size()) return 1;
  if (x.empty()) return 1;

  std::string p1 = "EPSG:" + std::to_string(epsgInput);
  std::string p2 = "EPSG:" + std::to_string(epsgOutput);
  PJ *pj1 =
      proj_create_crs_to_crs(PJ_DEFAULT_CTX, p1.c_str(), p2.c_str(), NULL);
  if (pj1 == nullptr) return 1;
  PJ *pj2 = proj_normalize_for_visualization(PJ_DEFAULT_CTX, pj1);
  if (pj2 == nullptr) return 1;
  proj_destroy(pj1);

  outx.clear();
  outy.clear();
  outx.reserve(x.size());
  outy.reserve(y.size());

  for (size_t i = 0; i < x.size(); ++i) {
    PJ_COORD cin;
    if (proj_angular_input(pj2, PJ_INV)) {
      cin.lp.lam = Hmdf::Constants::toRadians(x[i]);
      cin.lp.phi = Hmdf::Constants::toRadians(y[i]);
    } else {
      cin.xy.x = x[i];
      cin.xy.y = y[i];
    }

    PJ_COORD cout = proj_trans(pj2, PJ_FWD, cin);

    if (proj_angular_output(pj2, PJ_FWD)) {
      outx.push_back(proj_todeg(cout.lp.lam));
      outy.push_back(proj_todeg(cout.lp.phi));
    } else {
      outx.push_back(cout.xy.x);
      outy.push_back(cout.xy.y);
    }
  }
  proj_destroy(pj2);
  return 0;
}

std::string Projection::projVersion() {
  return std::to_string(static_cast<unsigned long long>(PROJ_VERSION_MAJOR)) +
         "." +
         std::to_string(static_cast<unsigned long long>(PROJ_VERSION_MINOR)) +
         "." +
         std::to_string(static_cast<unsigned long long>(PROJ_VERSION_PATCH));
}

int Projection::cpp(double lambda0, double phi0, double xin, double yin,
                    double &xout, double &yout) {
  std::vector<double> vxin = {xin};
  std::vector<double> vyin = {yin};
  std::vector<double> vxout, vyout;
  int ierr = Projection::cpp(lambda0, phi0, vxin, vyin, vxout, vyout);
  if (ierr == 0) {
    xout = vxout[0];
    yout = vyout[0];
  }
  return ierr;
}

int Projection::cpp(double lambda0, double phi0, const std::vector<double> &xin,
                    const std::vector<double> &yin, std::vector<double> &xout,
                    std::vector<double> &yout) {
  if (xin.empty()) return 1;
  if (xin.size() != yin.size()) return 1;

  double slam0 = Hmdf::Constants::toRadians(lambda0);
  double sfea0 = Hmdf::Constants::toRadians(phi0);
  double r = Hmdf::Constants::radiusEarth(phi0);
  xout.clear();
  yout.clear();
  xout.reserve(xin.size());
  yout.reserve(yout.size());
  for (size_t i = 0; i < xin.size(); ++i) {
    xout.push_back(r * (Hmdf::Constants::toRadians(xin[i]) - slam0) *
                   std::cos(sfea0));
    yout.push_back(r * Hmdf::Constants::toRadians(yin[i]));
  }
  return 0;
}

int Projection::inverseCpp(double lambda0, double phi0, const double lambda,
                           const double phi, double &x, double &y) {
  std::vector<double> vlambda = {lambda};
  std::vector<double> vphi = {phi};
  std::vector<double> vx, vy;
  int ierr = Projection::inverseCpp(lambda0, phi0, vlambda, vphi, vx, vy);
  if (ierr == 0) {
    x = vx[0];
    y = vy[0];
  }
  return ierr;
}

int Projection::inverseCpp(double lambda0, double phi0,
                           const std::vector<double> &lambda,
                           const std::vector<double> &phi,
                           std::vector<double> &x, std::vector<double> &y) {
  if (lambda.empty()) return 1;
  if (lambda.size() != phi.size()) return 1;

  x.clear();
  y.clear();
  x.reserve(lambda.size());
  y.reserve(lambda.size());

  double slam0 = Hmdf::Constants::toRadians(lambda0);
  double sfea0 = Hmdf::Constants::toRadians(phi0);
  double r = Hmdf::Constants::radiusEarth(phi0);
  for (size_t i = 0; i < lambda.size(); ++i) {
    x.push_back(
        Hmdf::Constants::toDegrees(slam0 + lambda[i] / (r * std::cos(sfea0))));
    y.push_back(Hmdf::Constants::toDegrees(phi[i] / r));
  }
  return 0;
}
