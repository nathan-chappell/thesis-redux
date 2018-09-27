// geometry.h
#pragma once

#include <iostream>
#include <list>

struct Point;
using Direction = Point;
using Extent = Point;

/// Point
//
struct Point {
  double x, y;

  Point() : x{0}, y{0} {}
  Point(double x, double y) : x{x}, y{y} {}
  Point(const Point &) = default;
  Point(Point &&) = default;
  Point &operator=(const Point &) = default;
  Point &operator=(Point &&) = default;

  Point &operator+=(const Point &p);
  Point &operator-=(const Point &p);
  Point &operator*=(double scale);
  Point &operator/=(double scale);

  static Point NotAPoint();
  static Point infinity();
  static bool is_valid(const Point &p);
};

inline bool operator==(const Point &p, const Point &q) {
  return p.x == q.x && p.y == q.y;
}
inline bool operator!=(const Point &p, const Point &q) { return !(p == q); }
inline Point operator+(Point p, const Point &q) { return p += q; }
inline Point operator-(Point p, const Point &q) { return p -= q; }
inline Point operator*(double d, Point p) { return p *= d; }
inline Point operator/(Point p, double d) { return p /= d; }
inline double operator*(const Point &p, const Point &q) {
  return p.x * q.x + p.y * q.y;
}

std::ostream &operator<<(std::ostream &o, const Point &p);
bool operator<(const Point &l, const Point &r);

/// Line
struct Line {
  Point start;
  Direction direction;

  Line() = default;
  Line(const Point &start, const Direction &direction)
      : start(start), direction(direction) {}
  Line(const Line &) = default;
  Line(Line &&) = default;
  Line &operator=(const Line &) = default;
  Line &operator=(Line &&) = default;

  Point Intersection(const Line &line) const;
  Point Project(const Point &point) const;
  bool Contains(const Point &p) const;
  bool Intersects(const Line &line) const;
  double Distance(const Point &point) const;
};

std::ostream &operator<<(std::ostream &o, const Line &line);

bool in_box(const Point &position, const Extent &extent, const Point &p);

struct LineSegment {
  Point u, v;

  LineSegment() = default;
  LineSegment(const Point &u, const Point &v) : u(u), v(v) {}
  LineSegment(const LineSegment &) = default;
  LineSegment(LineSegment &&) = default;
  LineSegment &operator=(const LineSegment &) = default;
  LineSegment &operator=(LineSegment &&) = default;
  explicit operator Line() const { return Line(u, v - u); }

  Point Intersection(const Line &line);
  Point Intersection(const LineSegment &line) const;
  Point MidPoint() const;
  bool Contains(const Point &p) const;
  bool Intersects(const Line &line) const;
  bool Intersects(const LineSegment &lineSegment) const;
  double Distance(const Point &point) const;
};

std::ostream &operator<<(std::ostream &o, const LineSegment &lineSegment);

struct Rectangle {
  Point position; // upper left
  Extent extent;

  Rectangle() = default;
  Rectangle(const Point &position, const Extent &extent) : position(position), extent(extent) {}
  Rectangle(const Rectangle &) = default;
  Rectangle(Rectangle &&) = default;
  Rectangle &operator=(const Rectangle &) = default;
  Rectangle &operator=(Rectangle &&) = default;

  LineSegment Left() const;
  LineSegment Right() const;
  LineSegment Bottom() const;
  LineSegment Top() const;

  std::list<Point> Intersection(const Line &line) const;
  std::list<Point> Intersection(const LineSegment &lineSegment) const;

  bool Contains(const Point &p) const;
  bool Contains(const LineSegment &lineSegment) const;
  bool Contains(const Rectangle &rectangle) const;
  bool Intersects(const Line &line) const;
  bool Intersects(const LineSegment &lineSegment) const;
  bool Intersects(const Rectangle &rectangle) const;
};

std::ostream &operator<<(std::ostream &o, const Rectangle &rectangle);

