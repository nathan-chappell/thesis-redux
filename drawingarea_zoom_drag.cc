// drawingarea_zoom_drag.cc

#include "drawingarea_zoom_drag.h"

#include <iostream>

using namespace std;

const double zoom_factor = 1.1;

// does not work on matrices with shearing properties
double get_scale_from_matrix(const Cairo::Matrix &m) {
  double zero = 0;
  double one = 1;
  m.transform_distance(zero, one);
  return one;
}

Point get_matrix_transform(const Cairo::Matrix &m, double x, double y) {
  m.transform_point(x, y);
  return Point(x, y);
}

ostream &operator<<(ostream &o, const Cairo::Matrix &m) {
  Point origin = get_matrix_transform(m, 0, 0);
  Point p1 = get_matrix_transform(m, 1, 0);
  Point p2 = get_matrix_transform(m, 0, 1);
  cout << p1.x << "\t" << p2.x << endl;
  cout << p1.y << "\t" << p2.y << endl;
  cout << origin.x << "\t" << origin.y << endl;
  return o;
}

DrawingArea_ZoomDrag::DrawingArea_ZoomDrag()
    : m{Cairo::identity_matrix()}, drag_(false), change_since_last_draw_(true),
      last_pos_(0, 0),
      what_to_drag_(nullptr), zoomed_draw{[](CContext) { return false; }},
      set_drag{[](GdkEventButton *) { return make_shared<Rectangle>(); }} {
  add_events(Gdk::SCROLL_MASK | Gdk::BUTTON_PRESS_MASK |
             Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
}

DrawingArea_ZoomDrag::DrawingArea_ZoomDrag(
    std::function<bool(CContext)> zoomed_draw,
    std::function<DragTarget(GdkEventButton *)> set_drag)
    : m{Cairo::identity_matrix()}, drag_(false), change_since_last_draw_(true),
      last_pos_(0, 0),
      what_to_drag_(nullptr), zoomed_draw{zoomed_draw}, set_drag{set_drag} {
  add_events(Gdk::SCROLL_MASK | Gdk::BUTTON_PRESS_MASK |
             Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
}

bool DrawingArea_ZoomDrag::on_button_press_event(GdkEventButton *e) {
  // cout << "drawing area button press " << e->x << ", " << e->y << ","
  //<< e->button << endl;
  if (e->button == 1) { // left click
    drag_ = true;
    what_to_drag_ = set_drag(e);
    last_pos_ = Point(e->x, e->y);
  } else if (e->button == 2) { // middle click
    m = Cairo::identity_matrix();
  }
  return false;
}

bool DrawingArea_ZoomDrag::on_button_release_event(GdkEventButton *e) {
  if (e->button == 1) { // left click
    drag_ = false;
    what_to_drag_.reset();
  }
  return false;
}

bool DrawingArea_ZoomDrag::on_motion_notify_event(GdkEventMotion *e) {
  if (!drag_) {
    return false;
  }
  Point new_pos(e->x, e->y);
  Point translate = (new_pos - last_pos_);
  if (what_to_drag_) {
    translate_item(what_to_drag_->position, translate);
  } else {
    translate_matrix(translate);
    set_changed();
  }
  last_pos_ = new_pos;
  queue_draw();
  return false;
}

bool DrawingArea_ZoomDrag::on_draw(CContext c) {
  c->set_matrix(m * c->get_matrix());
  bool result = zoomed_draw(c);
  unset_changed();
  return result;
}

bool DrawingArea_ZoomDrag::on_scroll_event(GdkEventScroll *e) {
  double this_zoom_factor;
  if (e->direction == GDK_SCROLL_UP) {
    set_changed();
    this_zoom_factor = zoom_factor;
  } else if (e->direction == GDK_SCROLL_DOWN) {
    set_changed();
    this_zoom_factor = 1 / zoom_factor;
  } else {
    return false;
  }

  double old_scale = get_scale_from_matrix(m);
  double new_scale = old_scale * this_zoom_factor;
  Point click(e->x, e->y);

  Point new_tx =
      click - new_scale / old_scale * (click - get_matrix_transform(m, 0, 0));
  m = Cairo::Matrix(new_scale, 0, 0, new_scale, new_tx.x, new_tx.y);

  queue_draw();
  return false;
}

void DrawingArea_ZoomDrag::user_to_image(Point &p) const {
  Cairo::Matrix inv = m;
  inv.invert();
  inv.transform_point(p.x, p.y);
}

void DrawingArea_ZoomDrag::user_to_image_scale(Extent &e) const {
  double scale = 1/get_scale_from_matrix(m);
  e *= scale;
}

void DrawingArea_ZoomDrag::user_to_image(Rectangle &rectangle) const {
  user_to_image(rectangle.position);
  user_to_image_scale(rectangle.extent);
}

bool DrawingArea_ZoomDrag::changed() const { return change_since_last_draw_; }

void DrawingArea_ZoomDrag::translate_item(Point& item, Point translate) {
  translate /= get_scale_from_matrix(m);
  item += translate;
}

void DrawingArea_ZoomDrag::translate_matrix(Point c) {
  c /= get_scale_from_matrix(m);
  m.translate(c.x, c.y);
}
