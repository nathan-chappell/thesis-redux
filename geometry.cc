// geometry.cc

#include "geometry.h"

#include <algorithm>
#include <cmath>
#include <limits>

using namespace std;

// a little bit of leaway for loss of accuracy in double, e.g. for "Contains"
// relations
static const double accuracy_buffer = .001;

// assumes a <= b
bool in_proper_interval(double a, double b, double x) {
  return a <= x && x <= b;
}

// fixes interval if necessary
bool in_interval(double a, double b, double x) {
  auto mmax = std::minmax(a, b);
  return in_proper_interval(mmax.first, mmax.second, x);
}

/// Point

Point &Point::operator+=(const Point &p) {
  x += p.x;
  y += p.y;
  return *this;
}

Point &Point::operator-=(const Point &p) {
  x -= p.x;
  y -= p.y;
  return *this;
}

Point &Point::operator*=(double scale) {
  x *= scale;
  y *= scale;
  return *this;
}

Point &Point::operator/=(double scale) {
  x /= scale;
  y /= scale;
  return *this;
}

Point Point::NotAPoint() {
  return Point(std::numeric_limits<double>::quiet_NaN(),
               std::numeric_limits<double>::quiet_NaN());
}

Point Point::infinity() {
  return Point(std::numeric_limits<double>::infinity(),
               std::numeric_limits<double>::infinity());
}

bool Point::is_valid(const Point &p) {
  return p.x != std::numeric_limits<double>::infinity() &&
         p.x != std::numeric_limits<double>::quiet_NaN() &&
         p.y != std::numeric_limits<double>::infinity() &&
         p.y != std::numeric_limits<double>::quiet_NaN();
}

std::ostream &operator<<(std::ostream &o, const Point &p) {
  o << "{" << p.x << ", " << p.y << "}";
  return o;
}

std::istream &operator>>(std::istream &i, Point &p) {
  i >> p.x;
  i >> p.y;
  return i;
}

bool operator<(const Point &l, const Point &r) {
  return l.x < r.x || (l.x == r.x && l.y < r.y);
}

Point get_canonical_orthogonal(const Point &p) { return {p.y, -p.x}; }

/// Line

bool Line::Contains(const Point &p) const {
  Point diff = p - start;
  return abs(diff.x / direction.x - diff.y / direction.y) < accuracy_buffer;
}

Point Line::Intersection(const Line &line) const {
  double det = direction * get_canonical_orthogonal(line.direction);
  if (det == 0 && Contains(line.start)) {
    return Point::NotAPoint();
  } else if (det == 0) {
    return Point::infinity();
  } else {
    double t = direction * get_canonical_orthogonal(start - line.start) / det;
    return line.start + t * line.direction;
  }
}

bool Line::Intersects(const Line &line) const {
  return Intersection(line) != Point::infinity();
}

Point Line::Project(const Point &point) const {
  return start +
         ((point - start) * direction) / (direction * direction) * direction;
}

double Line::Distance(const Point &point) const {
  Point diff = point - Project(point);
  return std::hypot(diff.x, diff.y);
}

std::ostream &operator<<(std::ostream &o, const Line &line) {
  o << "{" << line.start << " + t*" << line.direction << "}";
  return o;
}

std::istream &operator>>(std::istream &i, Line &line) {
  i >> line.start;
  i >> line.direction;
  return i;
}

bool in_box(const Point &position, const Extent &extent, const Point &p) {
  return in_interval(position.x - accuracy_buffer, position.x + extent.x + accuracy_buffer,
                     p.x) &&
         in_interval(position.y - accuracy_buffer, position.y + extent.y + accuracy_buffer, p.y);
}

/// LineSegment

bool LineSegment::Contains(const Point &p) const {
  return Line(u, v - u).Contains(p) && in_interval(u.x, v.x, p.x);
}

Point LineSegment::Intersection(const LineSegment &line) const {
  return Line(*this).Intersection(Line(line));
}

Point LineSegment::MidPoint() const { return (u + v)/2.0; }

bool LineSegment::Intersects(const Line &line) const {
  Point intersection = Line(*this).Intersection(line);
  return Point::is_valid(intersection) && in_box(u, v - u, intersection);
}

Point LineSegment::Intersection(const Line &line) {
  if (Intersects(line)) {
    return Line(*this).Intersection(line);
  }
  return Point::NotAPoint();
}

bool LineSegment::Intersects(const LineSegment &lineSegment) const {
  return Intersects(Line(lineSegment));
}

double LineSegment::Distance(const Point &point) const {
  if (Contains(Line(*this).Project(point))) {
    return Line(*this).Distance(point);
  } else {
    auto du = u - point;
    auto dv = v - point;
    return std::min(std::hypot(du.x, du.y), std::hypot(dv.x, dv.y));
  }
}

std::ostream &operator<<(std::ostream &o, const LineSegment &lineSegment) {
  o << "{" << lineSegment.u << ", " << lineSegment.v << "}";
  return o;
}

std::istream &operator>>(std::istream &i, LineSegment &lineSegment) {
  i >> lineSegment.u;
  i >> lineSegment.v;
  return i;
}

/// Rectangle

LineSegment Rectangle::Left() const {
  return LineSegment(position, position + Point(0, extent.y));
}
LineSegment Rectangle::Right() const {
  return LineSegment(position + Point(extent.x, 0), position + Point(extent.x, extent.y));
}
LineSegment Rectangle::Bottom() const {
  return LineSegment(position + Point(0, extent.y), position + Point(extent.x, extent.y));
}
LineSegment Rectangle::Top() const {
  return LineSegment(position, position + Point(extent.x, 0));
}

std::list<Point> Rectangle::Intersection(const Line &line) const {
  std::list<Point> result;
  auto top = Top();
  auto bottom = Bottom();
  auto left = Left();
  auto right = Right();
  if (top.Intersects(line))
    result.push_back(top.Intersection(line));
  if (bottom.Intersects(line))
    result.push_back(bottom.Intersection(line));
  if (left.Intersects(line))
    result.push_back(left.Intersection(line));
  if (right.Intersects(line))
    result.push_back(right.Intersection(line));

  result.sort();
  result.unique();
  return result;
}

std::list<Point> Rectangle::Intersection(const LineSegment &lineSegment) const {
  auto result = Intersection((Line)lineSegment);
  result.remove_if(
      [&lineSegment](const Point &p) { return !lineSegment.Contains(p); });
  return result;
}

bool Rectangle::Contains(const Point &p) const { return in_box(position, extent, p); }

bool Rectangle::Contains(const LineSegment &lineSegment) const {
  return Contains(lineSegment.u) && Contains(lineSegment.v);
}

bool Rectangle::Contains(const Rectangle &rectangle) const {
  return Contains(rectangle.Left()) && Contains(rectangle.Right()) &&
         Contains(rectangle.Bottom()) && Contains(rectangle.Top());
}

bool Rectangle::Intersects(const Line &line) const {
  return Left().Intersects(line) || Right().Intersects(line) ||
         Bottom().Intersects(line) || Top().Intersects(line);
}

bool Rectangle::Intersects(const Rectangle &rectangle) const {
  return Contains(rectangle) || rectangle.Contains(*this) ||
         Top().Intersects(rectangle.Right()) ||
         Top().Intersects(rectangle.Left()) ||
         Bottom().Intersects(rectangle.Right()) ||
         Bottom().Intersects(rectangle.Left()) ||
         Left().Intersects(rectangle.Top()) ||
         Left().Intersects(rectangle.Bottom()) ||
         Right().Intersects(rectangle.Top()) ||
         Right().Intersects(rectangle.Bottom());
}

std::ostream &operator<<(std::ostream &o, const Rectangle &rectangle) {
  o << "{" << rectangle.position << ", " << rectangle.extent << "}";
  return o;
}

std::istream &operator>>(std::istream &i, Rectangle &rectangle) {
  i >> rectangle.position;
  i >> rectangle.extent;
  return i;
}
