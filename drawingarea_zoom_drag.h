// drawingarea_zoom_drag.h

#include "geometry.h"

#include <gtkmm-3.0/gtkmm/drawingarea.h>

#include <complex>
#include <functional>
#include <memory>
//#include <iostream>

using CContext = const Cairo::RefPtr<Cairo::Context> &;
using DragTarget = std::shared_ptr<Rectangle>;

/*
 * DrawingArea_ZoomDrag is a drawing area which handles some zooming and
 * dragging in
 * response to motion events
 */
class DrawingArea_ZoomDrag : public Gtk::DrawingArea {
  Cairo::Matrix m;
  bool drag_;
  bool change_since_last_draw_;
  Point last_pos_;
  DragTarget what_to_drag_;

public:
  /*
   * This call back will perform drawing actions after applying the correct
   * zooming tranformations.
   */
  std::function<bool(CContext)> zoomed_draw;
  /*
   * This call back sets the field what_to_drag.
   */
  std::function<DragTarget(GdkEventButton *)> set_drag;

  /*
   * Transform a coordinate from the user space to the image space (i.e. take a
   * click and tell where it is in reference to what you are drawing
   */
  void user_to_image(Point&) const;
  void user_to_image_scale(Extent&) const;
  void user_to_image(Rectangle&) const;
  /*
   * Used to inform the user if the matrix has changed since the last draw
   */
  bool changed() const;

  DrawingArea_ZoomDrag();
  DrawingArea_ZoomDrag(
      std::function<bool(CContext)> zoomed_draw,
      std::function<DragTarget(GdkEventButton *)> set_drag);

protected:
  bool on_button_press_event(GdkEventButton *) override;
  bool on_button_release_event(GdkEventButton *) override;
  bool on_motion_notify_event(GdkEventMotion *) override;
  bool on_draw(CContext) override;
  bool on_scroll_event(GdkEventScroll *e) override;

private:
  void translate_matrix(Point);
  //translates item by translate scaled by the matrices current scale factor
  void translate_item(Point& item, Point translate);
  void set_changed() { change_since_last_draw_ = true; }
  void unset_changed() { change_since_last_draw_ = false; }
};

std::ostream &operator<<(std::ostream &o, const Cairo::Matrix &m);
