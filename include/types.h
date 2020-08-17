#ifndef TYPE_H
#define TYPE_H

#ifdef USE_QT_TYPES
#include <QString>
#include <QVector>
template <typename T>
using HmdfVector = QVector<T>;
using HmdfString = QString;
#else
#include <string>
#include <vector>

#include "date.h"
template <typename T>
using HmdfVector = std::vector<T>;
using HmdfString = std::string;
#endif

#endif  // TYPE_H
