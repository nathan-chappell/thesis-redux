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

C get_matrix_transform(const Cairo::Matrix &m, double x, double y) {
  m.transform_point(x, y);
  return C(x, y);
}

ostream &operator<<(ostream &o, const Cairo::Matrix &m) {
  C origin = get_matrix_transform(m, 0, 0);
  C c1 = get_matrix_transform(m, 1, 0);
  C c2 = get_matrix_transform(m, 0, 1);
  cout << c1.real() << "\t" << c2.real() << endl;
  cout << c1.imag() << "\t" << c2.imag() << endl;
  cout << origin.real() << "\t" << origin.imag() << endl;
  return o;
}

DrawingArea_ZoomDrag::DrawingArea_ZoomDrag()
    : m{Cairo::identity_matrix()}, drag_(false), last_pos_(0, 0),
      what_to_drag_(nullptr), zoomed_draw{[](CContext) { return false; }},
      set_drag{[](GdkEventButton *) { return make_shared<C>(); }} {
  add_events(Gdk::SCROLL_MASK | Gdk::BUTTON_PRESS_MASK |
             Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
}

DrawingArea_ZoomDrag::DrawingArea_ZoomDrag(
    std::function<bool(CContext)> zoomed_draw,
    std::function<shared_ptr<C>(GdkEventButton *)> set_drag)
    : m{Cairo::identity_matrix()}, drag_(false), last_pos_(0, 0),
      what_to_drag_(nullptr), zoomed_draw{zoomed_draw}, set_drag{set_drag} {
  add_events(Gdk::SCROLL_MASK | Gdk::BUTTON_PRESS_MASK |
             Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
}

bool DrawingArea_ZoomDrag::on_button_press_event(GdkEventButton *e) {
  //cout << "drawing area button press " << e->x << ", " << e->y << ","
       //<< e->button << endl;
  if (e->button == 1) { // left click
    drag_ = true;
    what_to_drag_ = set_drag(e);
    last_pos_ = C(e->x, e->y);
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
  C new_pos(e->x, e->y);
  C translate = (new_pos - last_pos_);
  if (what_to_drag_) {
    translate_item(what_to_drag_.get(), translate);
  } else {
    translate_matrix(translate);
  }
  last_pos_ = new_pos;
  queue_draw();
  return false;
}

bool DrawingArea_ZoomDrag::on_draw(CContext c) {
  c->set_matrix(m * c->get_matrix());
  return zoomed_draw(c);
}

bool DrawingArea_ZoomDrag::on_scroll_event(GdkEventScroll *e) {
  double this_zoom_factor;
  if (e->direction == GDK_SCROLL_UP) {
    this_zoom_factor = zoom_factor;
  } else if (e->direction == GDK_SCROLL_DOWN) {
    this_zoom_factor = 1 / zoom_factor;
  } else {
    return false;
  }

  double old_scale = get_scale_from_matrix(m);
  double new_scale = old_scale * this_zoom_factor;
  C click(e->x, e->y);

  C new_tx =
      click - new_scale / old_scale * (click - get_matrix_transform(m, 0, 0));
  m = Cairo::Matrix(new_scale, 0, 0, new_scale, new_tx.real(), new_tx.imag());

  queue_draw();
  return false;
}

C DrawingArea_ZoomDrag::user_to_image(const C &c) const {
  double x = c.real();
  double y = c.imag();
  Cairo::Matrix inv = m;
  inv.invert();
  inv.transform_point(x, y);
  return {x, y};
}

void DrawingArea_ZoomDrag::translate_item(C *item, C translate) {
  translate /= get_scale_from_matrix(m);
  *item += translate;
}

void DrawingArea_ZoomDrag::translate_matrix(C c) {
  c /= get_scale_from_matrix(m);
  m.translate(c.real(), c.imag());
}
